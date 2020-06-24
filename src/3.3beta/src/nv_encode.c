/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video encode routines
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
#include "nv.h"

#define PACKETLEN		1024
#define AGE_THRESHOLD		8
#define XMIT_THRESHOLD		64

static grabproc_t *grab=NULL;

static int width, height, block_width, block_height;
static int bandwidth, framerate, xmit_color;
static uint8 *curr_y_data=NULL;
static int8 *curr_uv_data=NULL;
static uint8 *xmit=NULL, *age=NULL;

static int start_x=0, start_y=0, age_x=0, age_y=0;
static int frame_ok=0, scan_cycle=0;
static uint8 nv_data[2048];
static uint32 rtp_frame_time;
static struct timeval frame_time, xmit_time;

static struct {
    uint16 width, height;
} nv_hdr;

static void NV_ImageDiff(vidimage_t *image)
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

	scan_cycle = (scan_cycle+1) & 7;
    }
}

static uint8 *NV_PutBlock(uint8 *data, uint8 **runhdrp, int x, int y, int loss)
{
    int i, offset, zcount, rem;
    uint8 b, *yp, *runhdr=*runhdrp;
    uint32 blkw, *blkwp, *blkwLim, block[32];
    int8 *uvp, *blkp;
    static uint8 run_x = -1, run_y = -1;

    if ((runhdr == NULL) || (run_x != x) || (run_y != y)) {
	*runhdrp = runhdr = data;
	runhdr[0] = 1;
	runhdr[1] = run_x = x;
	runhdr[2] = run_y = y;
	data += 3;
    } else {
	runhdr[0]++;
    }

    offset = y*8*width+x*8;
    yp = &curr_y_data[offset];
    uvp = &curr_uv_data[offset];

    NV_FwdTransform(yp, xmit_color? uvp : NULL, width, block);
    blkp = (int8 *)block;

    if (loss > 0) {
	blkp = (int8 *)block;
	if ((blkp[2]>=-loss) && (blkp[2]<=loss)) blkp[2] = 0;
	if ((blkp[3]>=-loss) && (blkp[3]<=loss)) blkp[3] = 0;
	if ((blkp[4]>=-loss) && (blkp[4]<=loss)) blkp[4] = 0;
	if ((blkp[5]>=-loss) && (blkp[5]<=loss)) blkp[5] = 0;
	if ((blkp[6]>=-loss) && (blkp[6]<=loss)) blkp[6] = 0;
	if ((blkp[7]>=-loss) && (blkp[7]<=loss)) blkp[7] = 0;
	blkp = ((int8 *)block)+10;
	for (i=10; i<64; i+=6, blkp+=6) {
	    if ((blkp[0]>=-loss) && (blkp[0]<=loss)) blkp[0] = 0;
	    if ((blkp[1]>=-loss) && (blkp[1]<=loss)) blkp[1] = 0;
	    if ((blkp[2]>=-loss) && (blkp[2]<=loss)) blkp[2] = 0;
	    if ((blkp[3]>=-loss) && (blkp[3]<=loss)) blkp[3] = 0;
	    if ((blkp[4]>=-loss) && (blkp[4]<=loss)) blkp[4] = 0;
	    if ((blkp[5]>=-loss) && (blkp[5]<=loss)) blkp[5] = 0;
	}

	if (xmit_color) {
	    blkp = ((int8 *)block)+64;
	    if ((blkp[2]>=-2*loss) && (blkp[2]<=2*loss)) blkp[2] = 0;
	    if ((blkp[3]>=-2*loss) && (blkp[3]<=2*loss)) blkp[3] = 0;
	    if ((blkp[4]>=-2*loss) && (blkp[4]<=2*loss)) blkp[4] = 0;
	    if ((blkp[5]>=-2*loss) && (blkp[5]<=2*loss)) blkp[5] = 0;
	    if ((blkp[6]>=-2*loss) && (blkp[6]<=2*loss)) blkp[6] = 0;
	    if ((blkp[7]>=-2*loss) && (blkp[7]<=2*loss)) blkp[7] = 0;
	    blkp = ((int8 *)block)+74;
	    for (i=10; i<64; i+=6, blkp+=6) {
		if ((blkp[0]>=-2*loss) && (blkp[0]<=2*loss)) blkp[0] = 0;
		if ((blkp[1]>=-2*loss) && (blkp[1]<=2*loss)) blkp[1] = 0;
		if ((blkp[2]>=-2*loss) && (blkp[2]<=2*loss)) blkp[2] = 0;
		if ((blkp[3]>=-2*loss) && (blkp[3]<=2*loss)) blkp[3] = 0;
		if ((blkp[4]>=-2*loss) && (blkp[4]<=2*loss)) blkp[4] = 0;
		if ((blkp[5]>=-2*loss) && (blkp[5]<=2*loss)) blkp[5] = 0;
	    }
	}
    }

    /* XXX: The ntohl() calls here need to be redone as something more
	    efficient for little-endian machines! */
    blkwp = block;
    blkwLim = xmit_color? block+32 : block+16;
    i = 2;
    zcount = 0;
    blkw = ntohl(*blkwp++);
    data[1] = blkw >> 24;
    blkw <<= 8;
    rem = 3;
    do {
	while ((b = blkw >> 24) != 0) {
	    data[i++] = b;
	    blkw <<= 8;
	    rem--;
	    if (i == 4) break;
	    if (rem == 0) {
		if (blkwp == blkwLim) break;
		blkw = ntohl(*blkwp++);
		rem = 4;
	    }
	}

	while (1) {
	    if (zcount+rem >= 63) {
		break;
	    } else if (blkw == 0) {
		zcount += rem;
		rem = 0;
		if (blkwp == blkwLim) break;
		blkw = ntohl(*blkwp++);
		rem = 4;
	    } else if ((blkw >> 24) == 0) {
		zcount++;
		blkw <<= 8;
		rem--;
	    } else break;
	}

	*data = ((i-1) << 6) + zcount;
	data += i;
	i = 1;
	zcount = 0;
    } while ((rem != 0) || (blkwp != blkwLim));

    yp += 8;
    uvp += 8;

    if (++run_x == block_width) run_x = -1;

    return data;
}

static int NV_SendMotion(int *syncp)
{
    int x, y, offset;
    uint8 *data=nv_data, *runhdr=NULL, *xmitp, *agep;

    offset = start_y*block_width+start_x;
    xmitp = &xmit[offset];
    agep = &age[offset];
    for (y=start_y; y < block_height; y++) {
	for (x=start_x; x < block_width; x++) {
	    if (*xmitp) {
		*xmitp = 0;
		*agep = 1;
		data = NV_PutBlock(data, &runhdr, x, y, 2);
	    } else if ((*agep > 0) && (++*agep == AGE_THRESHOLD)) {
		*agep = 0;
		data = NV_PutBlock(data, &runhdr, x, y, 0);
	    }

	    if (data-nv_data > PACKETLEN) {
		start_x = x;
		start_y = y;
		*syncp = 0;
		return data-nv_data;
	    }

	    xmitp++;
	    agep++;
	}
	start_x = 0;
    }

    start_x = start_y = 0;
    *syncp = 1;
    return data-nv_data;
}

static int NV_SendBackground(int *syncp)
{
    int x, y, offset;
    uint8 *data=nv_data, *runhdr=NULL, *xmitp, *agep;

    offset = age_y*block_width+age_x;
    xmitp = &xmit[offset];
    agep = &age[offset];
    for (y=age_y; y < block_height; y++) {
	for (x=age_x; x < block_width; x++) {
	    if (*agep == 0) {
		*xmitp = 0;
		data = NV_PutBlock(data, &runhdr, x, y, 0);

		if (data-nv_data >= PACKETLEN) {
		    age_x = x;
		    age_y = y;
		    *syncp = 0;
		    return data-nv_data;
		}
	    }

	    xmitp++;
	    agep++;
	}
	age_x = 0;
    }

    age_x = age_y = 0;
    *syncp = 0;
    return data-nv_data;
}

static int NV_Encode(vidimage_t *image, xmitproc_t *callback)
{
    int bytes, sync, tdiff, timeout;
    struct timeval cur_time;
    static int aging_bytes=0, aging_count=0;

    nv_hdr.width &= htons(NV_WIDTHMASK);
    if (xmit_color) nv_hdr.width |= htons(NV_COLORFLAG);

    if (!frame_ok) {
	rtp_frame_time = RTPTime();
	if ((*grab)(curr_y_data, curr_uv_data) == 0)
	    return -1;
	frame_ok = 1;

	NV_ImageDiff(image);
    }

    if (aging_bytes < PACKETLEN) {
	bytes = NV_SendMotion(&sync);
	aging_count = 0;
    } else if (++aging_count == 4) {
	bytes = NV_SendMotion(&sync);
	aging_count = aging_bytes = 0;
    } else {
	bytes = NV_SendBackground(&sync);
	aging_bytes -= bytes;
    }

    (*callback)(sync, RTPCONT_NV, rtp_frame_time, (uint8 *) &nv_hdr,
		sizeof(nv_hdr), nv_data, bytes);
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

/*ARSGUSED*/
static void NV_Encode_Reconfig(int color, int w, int h)
{
    xmit_color = color;
    width = w;
    height = h;

    nv_hdr.width = htons(width);
    nv_hdr.height = htons(height);
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
int NV_Encode_Probe(grabber_t *grabber)
{
    return 1;
}

/*ARSGUSED*/
encodeproc_t *NV_Encode_Start(grabber_t *grabber, int max_bandwidth,
			      int max_framerate, int config)
{
    if ((grab = grabber->start(max_framerate, config,
			       NV_Encode_Reconfig)) == 0) return 0;

    bandwidth = max_bandwidth;
    framerate = max_framerate;

    gettimeofday(&xmit_time, NULL);
    frame_time = xmit_time;
    return NV_Encode;
}

void NV_Encode_Stop(grabber_t *grabber)
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
