/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	CU-SeeMe encode routines
*/

/*
 * Copyright (c) Xerox Corporation 1992. All rights reserved.
 *  
 * License is granted to copy, to use, and to make and to use derivative
 * works for research and evaluation purposes, provided that Xerox is
 * acknowledged in all documentation pertaining to any such copy or derivative
 * work. Xerox grants no other licenses expressed or implied. The Xerox trade
 * name should not be used in any advertising without its written permission.
 *  
 * XEROX CORPORATION MAKES NO REPRESENTATIONS CONCERNING EITHER THE
 * MERCHANTABILITY OF THIS SOFTWARE OR THE SUITABILITY OF THIS SOFTWARE
 * FOR ANY PARTICULAR PURPOSE.  The software is provided "as is" without
 * express or implied warranty of any kind.
 *  
 * These notices must be retained in any copies of any part of this software.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "sized_types.h"
#include "rtp.h"
#include "vid_image.h"
#include "vid_code.h"
#include "cuseeme.h"

#define PACKETLEN		1024
#define AGE_THRESHOLD		16
#define XMIT_THRESHOLD		160

static uint8 dither4[16] =
    { 12,  4, 14,  6,
       3, 11,  1,  9,
      15,  7, 13,  5,
       0,  8,  2, 10 };

static uint8 pack40[0x1112], pack5410[0x3334], map8to4[256*16];

static grabproc_t *grab=NULL;

static int width, height, block_width, block_height;
static int bandwidth, framerate;
static uint8 *curr_y_data=NULL;
static int8 *curr_uv_data=NULL;
static uint8 *xmit=NULL, *age=NULL;

static int start_x=0, start_y=0, age_x=0, age_y=0;
static int frame_ok=0, scan_cycle=0;
static uint8 cuseeme_data[2048];
static uint32 rtp_frame_time;
static struct timeval frame_time, xmit_time;

static uint16 cuseeme_type;

static int CUSeeMe_ImageDiff(vidimage_t *image)
{
    int x, y, offset, shift, mask=0xff00ff;
    int xmax=block_width-1, ymax=block_height-1;
    int32 pri;
    uint8 *xmitp=xmit;
    uint32 *oldp, *newp;
    static int scan_cycle=0;

    if ((image == NULL)||(image->width != width)||(image->height != height)) {
	memset(xmit, 1, block_width*block_height);
    } else {
	offset = (scan_cycle & 3)*width;
	shift = (scan_cycle & 4)*2;
	oldp = (uint32 *) &image->y_data[offset];
	newp = (uint32 *) &curr_y_data[offset];

	for (y=0; y<=ymax; y++) {
	    for (x=0; x<=xmax; x++) {
                pri = ((newp[0]>>shift) & mask) +
                      ((newp[1]>>shift) & mask) +
                      ((newp[width]>>shift) & mask) +
                      ((newp[width+1]>>shift) & mask) -
                      ((oldp[0]>>shift) & mask) -
                      ((oldp[1]>>shift) & mask) -
                      ((oldp[width]>>shift) & mask) -
                      ((oldp[width+1]>>shift) & mask);
                pri = ((pri << 16) + pri) >> 16;
                if (pri < 0) pri = -pri;

		if (pri >= XMIT_THRESHOLD) xmitp[0] = 1;

		if (pri >= XMIT_THRESHOLD+4) {
		    if (x != 0) xmitp[-1] = 1;
		    if (x != xmax) xmitp[1] = 1;
		    if (y != 0) xmitp[-block_width] = 1;
		    if (y != ymax) xmitp[block_width] = 1;
		}

		if (pri >= XMIT_THRESHOLD+8) {
		    if ((x != 0) && (y != 0)) xmitp[-block_width-1] = 1;
		    if ((x != xmax) && (y != 0)) xmitp[-block_width+1] = 1;
		    if ((x != 0) && (y != ymax)) xmitp[block_width-1] = 1;
		    if ((x != xmax) && (y != ymax)) xmitp[block_width+1] = 1;
		}

		oldp += 2;
		newp += 2;
		xmitp++;
	    }

	    oldp += 7*width/4;
	    newp += 7*width/4;
	}

	scan_cycle = (scan_cycle+1) % 8;
    }
}

static int CUSeeMe_CompressRow(uint8 **datap, uint32 row, uint32 prevRow)
{
    int code=0;
    uint8 *data = *datap;
    uint32 diff;

    diff = row - prevRow;
    if ((diff -= 0x22222222) == 0) code = 1;
    else if ((diff += 0x11111111) == 0) code = 4;
    else if ((diff += 0x11111111) == 0) code = 7;
    else if ((diff += 0x11111111) == 0) code = 10;
    else if ((diff += 0x11111111) == 0) code = 13;

    if (code) return code;

    if (((diff += 0x11111111) & 0xeeeeeeee) == 0) code = 14;
    else if (((diff -= 0x11111111) & 0xeeeeeeee) == 0) code = 11;
    else if (((diff -= 0x11111111) & 0xeeeeeeee) == 0) code = 8;
    else if (((diff -= 0x11111111) & 0xeeeeeeee) == 0) code = 5;
    else if (((diff -= 0x11111111) & 0xeeeeeeee) == 0) code = 2;

    if (code) {
	data[0] = (pack40[diff >> 16] << 4) + pack40[diff & 0xffff];
	*datap = data+1;
	return code;
    }

    if (((diff += 0x11111111) & 0xcccccccc) == 0) code = 3;
    else if (((diff += 0x11111111) & 0xcccccccc) == 0) code = 6;
    else if (((diff += 0x11111111) & 0xcccccccc) == 0) code = 9;
    else if (((diff += 0x11111111) & 0xcccccccc) == 0) code = 12;
    else if (((diff += 0x11111111) & 0xcccccccc) == 0) code = 15;

    if (code) {
	data[0] = pack5410[diff >> 16];
	data[1] = pack5410[diff & 0xffff];
	*datap = data+2;
	return code;
    }

    
    data[0] = row >> 24;
    data[1] = row >> 16;
    data[2] = row >> 8;
    data[3] = row;
    *datap = data+4;
    return code;
}

static uint8 *CUSeeMe_CompressSquare(uint8 *data, int x, int y)
{
    int i, j, square, code;
    uint8 *yp, *codep, *map=map8to4;
    uint32 row, prevRow;
    int32 tmp;

    square = y*block_width+x;
    data[0] = square >> 8;
    data[1] = square & 0xff;

    codep = data+2;
    data += 6;

    row = 0x88888888;
    yp = &curr_y_data[y*8*width+x*8];
    for (i=0; i<8; i += 2) {
	prevRow = row;
	j = (i%4) * 4;
	row = map[yp[0]*16+j] + map[yp[1]*16+j+1];
	row = (row << 8) + map[yp[2]*16+j+2] + map[yp[3]*16+j+3];
	row = (row << 8) + map[yp[4]*16+j]   + map[yp[5]*16+j+1];
	row = (row << 8) + map[yp[6]*16+j+2] + map[yp[7]*16+j+3];
	yp += width;

	code = CUSeeMe_CompressRow(&data, row, prevRow);

	prevRow = row;
	j += 4;
	row = map[yp[0]*16+j] + map[yp[1]*16+j+1];
	row = (row << 8) + map[yp[2]*16+j+2] + map[yp[3]*16+j+3];
	row = (row << 8) + map[yp[4]*16+j]   + map[yp[5]*16+j+1];
	row = (row << 8) + map[yp[6]*16+j+2] + map[yp[7]*16+j+3];
	yp += width;

	*codep++ = (code << 4) + CUSeeMe_CompressRow(&data, row, prevRow);
    }

    return data;
}

static int CUSeeMe_SendMotion(int *syncp)
{
    int x, y, offset;
    uint8 *data=cuseeme_data, *xmitp, *agep;

    offset = start_y*block_width+start_x;
    xmitp = &xmit[offset];
    agep = &age[offset];
    for (y=start_y; y < block_height; y++) {
	for (x=start_x; x < block_width; x++) {
	    if (*xmitp) {
		*xmitp = 0;
		*agep = 0;
		data = CUSeeMe_CompressSquare(data, x, y);

		if (data-cuseeme_data > PACKETLEN) {
		    start_x = x;
		    start_y = y;
		    *syncp = 0;
		    return data-cuseeme_data;
		}
	    } else {
		(*agep)++;
	    }

	    xmitp++;
	    agep++;
	}
	start_x = 0;
    }

    start_x = start_y = 0;
    *syncp = 1;
    return data-cuseeme_data;
}

static int CUSeeMe_SendBackground(int *syncp)
{
    int x, y, offset;
    uint8 *data=cuseeme_data, *xmitp, *agep;

    offset = age_y*block_width+age_x;
    xmitp = &xmit[offset];
    agep = &age[offset];
    for (y=age_y; y < block_height; y++) {
	for (x=age_x; x < block_width; x++) {
	    if (++*agep >= AGE_THRESHOLD) {
		*xmitp = *agep = 0;
		data = CUSeeMe_CompressSquare(data, x, y);

		if (data-cuseeme_data >= PACKETLEN) {
		    age_x = x;
		    age_y = y;
		    *syncp = 0;
		    return data-cuseeme_data;
		}
	    }

	    xmitp++;
	    agep++;
	}
	age_x = 0;
    }

    age_x = age_y = 0;
    *syncp = 0;
    return data-cuseeme_data;
}

static int CUSeeMe_Encode(vidimage_t *image, xmitproc_t *callback)
{
    int bytes, sync, tdiff, timeout;
    struct timeval cur_time;
    static int aging_bytes=0, aging_count=0;

    if (cuseeme_type == 0) return -1;

    if (!frame_ok) {
	rtp_frame_time = RTPTime();
	if ((*grab)(curr_y_data, curr_uv_data) == 0) return -1;
	frame_ok = 1;

	CUSeeMe_ImageDiff(image);
    }

    if (aging_bytes < PACKETLEN) {
	bytes = CUSeeMe_SendMotion(&sync);
	aging_count = 0;
    } else if (++aging_count == 4) {
	bytes = CUSeeMe_SendMotion(&sync);
	aging_count = aging_bytes = 0;
    } else {
	bytes = CUSeeMe_SendBackground(&sync);
	aging_bytes -= bytes;
    }

    (*callback)(sync, RTPCONT_CUSEEME, rtp_frame_time, (uint8 *) &cuseeme_type,
		sizeof(cuseeme_type), cuseeme_data, bytes);
    frame_ok = !sync;

    gettimeofday(&cur_time, NULL);
    tdiff = (cur_time.tv_sec-xmit_time.tv_sec)*1000 +
	    (cur_time.tv_usec-xmit_time.tv_usec)/1000;
    if ((timeout = bytes*8/bandwidth-tdiff) < 0) timeout = 0;

    if (sync && (framerate > 0)) {
	int frame_timeout;
 
	tdiff = (cur_time.tv_sec-frame_time.tv_sec)*1000 +
	        (cur_time.tv_usec-frame_time.tv_usec)/1000;
	if ((frame_timeout = 1000/framerate-tdiff) > timeout)
	    timeout = frame_timeout;
    }

    xmit_time = cur_time;
    xmit_time.tv_usec += timeout*1000;
    if (sync) frame_time = xmit_time;
    aging_bytes += bandwidth*tdiff/32;

    return timeout;
}

/*ARGSUSED*/
static void CUSeeMe_Encode_Reconfig(int color, int w, int h)
{
    width = w;
    height = h;

    if ((w == CUSEEME_FULLWIDTH) && (h == CUSEEME_FULLHEIGHT)) {
	cuseeme_type = htons(CUSEEME_FULLSIZE);
    } else if ((w == CUSEEME_HALFWIDTH) && (h == CUSEEME_HALFHEIGHT)) {
	cuseeme_type = htons(CUSEEME_HALFSIZE);
    } else {
	cuseeme_type = 0;
	return;
    }

    block_width = width/8;
    block_height = height/8;

    if (curr_y_data != NULL) free(curr_y_data);
    curr_y_data = (uint8 *) malloc(width*height);

    if (curr_uv_data != NULL) free(curr_uv_data);
    curr_uv_data = (int8 *) malloc(width*height);

    if (xmit != NULL) free(xmit);
    xmit = (uint8 *) malloc(block_height*block_width);
    memset(xmit, 0, block_height*block_width);

    if (age != NULL) free(age);
    age = (uint8 *) malloc(block_height*block_width);
    memset(age, 0, block_height*block_width);

    scan_cycle = start_x = start_y = age_x = age_y = frame_ok = 0;
}

/*ARGSUSED*/
int CUSeeMe_Encode_Probe(grabber_t *grabber)
{
    int i, j, pix;
    uint8 *dith=dither4, *map=map8to4;

    for (i=0; i<=0x1111; i++)
	pack40[i] = ((i & 0x1000) >> 9) + ((i & 0x100) >> 6) +
		    ((i & 0x10) >> 3) + (i & 0x1);

    for (i=0; i<=0x3333; i++)
	pack5410[i] = ((i & 0x3000) >> 6) + ((i & 0x300) >> 4) +
		      ((i & 0x30) >> 2) + (i & 0x3);

    for (i=0; i<256; i++) {
	for (j=0; j<16; j+=2) {
	    pix = i+dith[j];
	    if (pix > 0xff) pix = 0xff;
	    *map++ = 0xf0 - (pix & 0xf0);

	    pix = i+dith[j+1];
	    if (pix > 0xff) pix = 0xff;
	    *map++ = 0xf - (pix >> 4);
	}
    }

    return 1;
}

encodeproc_t *CUSeeMe_Encode_Start(grabber_t *grabber, int max_bandwidth,
				   int max_framerate, int config)
{
    if ((grab = grabber->start(max_framerate, config,
			       CUSeeMe_Encode_Reconfig)) == 0) return 0;

    bandwidth = max_bandwidth;
    framerate = max_framerate;

    gettimeofday(&xmit_time, NULL);
    frame_time = xmit_time;
    return CUSeeMe_Encode;
}

void CUSeeMe_Encode_Stop(grabber_t *grabber)
{
    grabber->stop();

    if (curr_y_data != NULL) free(curr_y_data);
    curr_y_data = NULL;

    if (curr_uv_data != NULL) free(curr_uv_data);
    curr_uv_data = NULL;

    if (xmit != NULL) free(xmit);
    xmit = NULL;

    if (age != NULL) free(age);
    age = NULL;
}
