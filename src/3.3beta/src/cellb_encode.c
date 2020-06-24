/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	Sun CellB encode routines
	Based heavily on code by Michael Speer <speer@eng.sun.com>
*/

/*
 * Copyright (c) Sun Microsystems, Inc.  1992, 1993. All rights reserved.
 *
 * License is granted to copy, to use, and to make and to use derivative
 * works for research and evaluation purposes, provided that Sun Microsystems is
 * acknowledged in all documentation pertaining to any such copy or derivative
 * work. Sun Microsystems grants no other licenses expressed or implied. The
 * Sun Microsystems  trade name should not be used in any advertising without
 * its written permission.
 *
 * SUN MICROSYSTEMS MERCHANTABILITY OF THIS SOFTWARE OR THE SUITABILITY OF
 * THIS SOFTWARE FOR ANY PARTICULAR PURPOSE.  The software is provided "as is"
 * without express or implied warranty of any kind.
 *
 * These notices must be retained in any copies of any part of this software.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include "sized_types.h"
#include "rtp.h"
#include "vid_image.h"
#include "vid_code.h"
#include "cellb.h"
#ifdef SUNVIDEO
#include "sunvideo_grab.h"
#endif

#define LITTLEENDIAN (ntohl(0x12345678) != 0x12345678)

#define PACKETLEN	1024
#define MAXSKIP		32
#define SKIPCODE	0x80
#define UVCLOSE(X,Y)	(((X) * (X)) + ((Y) * (Y)) <= 64)

#define RTABLE_STATES 23

static uint8 cellb_rtable[RTABLE_STATES] = {
    146, 75, 3, 95, 189, 165, 106, 229,
    239, 14, 208, 90, 8, 222, 122, 236,
    200, 171, 225, 131, 94, 12, 74
};

extern uint8  cellb_yyremap[], cellb_uvremap[];
extern uint16 cellb_yytable[], cellb_uvtable[];

static int frame_ok, start_x, start_y, width, height, bandwidth, framerate;
static grabproc_t *grab=NULL;
static uint32 rtp_frame_time;
static struct timeval frame_time, xmit_time;

static uint8 *curr_y_data;
static int8 *curr_uv_data;
static uint8 *cellb_data;
static uint8 *cellb_count;
static uint32 *cellb_history;

static uint8 cellb_dtbl[34816], *cellb_divtable[17];
static uint8 cellb_uvlookup[8192], cellb_error[17][512];

static struct {
    uint16	x, y;
    uint16	w, h;
} xilhdr;

/*
    .... randbyte() does not repeat after 100,000,000 calls ....
*/
static int randbyte(void)
{
    int i, rval;
    static int nextindex=0;
    int *nptr = &nextindex;
    int next = *nptr;

    if ((i = next-1) < 0) i = RTABLE_STATES-1;

    cellb_rtable[next] = rval = cellb_rtable[next] + cellb_rtable[i];
    if ((next += 1) > (RTABLE_STATES-1)) {
	*nptr = 0;
    } else {
	*nptr = next;
    }

    return rval;
}

#define SetMaskBit(YVAL, BIT)   \
    if ((YVAL) < ymean) {       \
	ylo += (YVAL);          \
	tmp += 1;               \
    } else {                    \
	yhi += (YVAL);          \
	mask += (1 << (BIT));   \
    }

static uint32 CellB_Encode_DoCell(uint8 *yp, int8 *uvp, int stride)
{
    int ymean, umean, vmean, uvindx;
    uint32 mask, yhi, ylo, tmp, yval;
    uint32 q0, q1, q2, q3;
    int32 t0, t1, t2, t3;

    t0 = ((int32 *)uvp)[0];
    t1 = ((int32 *)uvp)[stride];
    t2 = ((int32 *)uvp)[2 * stride];
    t3 = ((int32 *)uvp)[3 * stride];
    umean  = (t0<<24) >> 24; vmean  = (t0<<16) >> 24;
    umean += (t0<<8)  >> 24; vmean += t0 >> 24;
    umean += (t1<<24) >> 24; vmean += (t1<<16) >> 24;
    umean += (t1<<8)  >> 24; vmean += t1 >> 24;
    umean += (t2<<24) >> 24; vmean += (t2<<16) >> 24;
    umean += (t2<<8)  >> 24; vmean += t2 >> 24;
    umean += (t3<<24) >> 24; vmean += (t3<<16) >> 24;
    umean += (t3<<8)  >> 24; vmean += t3 >> 24;
    umean += 8*128;          vmean += 8*128;
    umean >>= 5;             vmean >>= 5;
    if (LITTLEENDIAN) {
	tmp = (umean<<6) | vmean;
    } else {
	tmp = (vmean<<6) | umean;
    }
    uvindx = cellb_uvremap[tmp];

    q0 = ((uint32 *)yp)[0];
    q1 = ((uint32 *)yp)[stride];
    q2 = ((uint32 *)yp)[2*stride];
    q3 = ((uint32 *)yp)[3*stride];
    ymean  = q0 & 0xff;     ymean += (q0<<16) >> 24;
    ymean += (q0<<8) >> 24; ymean += q0 >> 24;
    ymean += q1 & 0xff;     ymean += (q1<<16) >> 24;
    ymean += (q1<<8) >> 24; ymean += q1 >> 24;
    ymean += q2 & 0xff;     ymean += (q2<<16) >> 24;
    ymean += (q2<<8) >> 24; ymean += q2 >> 24;
    ymean += q3 & 0xff;     ymean += (q3<<16) >> 24;
    ymean += (q3<<8) >> 24; ymean += q3 >> 24;

    /*
     * .... next the bit mask for the tile is generated. for the first
     * fifteen bits this is done by comparing each luminance value to the
     * mean. if that value is greater than the mean the correnponding bit
     * in the mask is set, otherwise the counter for the number of zeros in
     * the mask, tmp, is incremented. This is handled by the SetMaskBit
     * macro. also, the values of those pixels greater than the mean are
     * accumulated in yhi, and the value of those less than the mean are
     * accumulted in ylo ....
     */
    mask = 0;
    ylo = 0;
    yhi = 0;
    tmp = 0;
    ymean >>= 4;

    if (LITTLEENDIAN) {
	yval = q3 >> 24;
	SetMaskBit(yval, 0);
	yval = (q3<<8) >> 24;
	SetMaskBit(yval, 1);
	yval = (q3<<16) >> 24;
	SetMaskBit(yval, 2);
	yval = q3 & 0xff;
	SetMaskBit(yval, 3);

	yval = q2 >> 24;
	SetMaskBit(yval, 4);
	yval = (q2<<8) >> 24;
	SetMaskBit(yval, 5);
	yval = (q2<<16) >> 24;
	SetMaskBit(yval, 6);
	yval = q2 & 0xff;
	SetMaskBit(yval, 7);

	yval = q1 >> 24;
	SetMaskBit(yval, 8);
	yval = (q1<<8) >> 24;
	SetMaskBit(yval, 9);
	yval = (q1<<16) >> 24;
	SetMaskBit(yval, 10);
	yval = q1 & 0xff;
	SetMaskBit(yval, 11);

	yval = q0 >> 24;
	SetMaskBit(yval, 12);
	yval = (q0<<8) >> 24;
	SetMaskBit(yval, 13);
	yval = (q0<<16) >> 24;
	SetMaskBit(yval, 14);
	yval = q0 & 0xff;
    } else {
	yval = q3 & 0xff;
	SetMaskBit(yval, 0);
	yval = (q3<<16) >> 24;
	SetMaskBit(yval, 1);
	yval = (q3<<8) >> 24;
	SetMaskBit(yval, 2);
	yval = q3 >> 24;
	SetMaskBit(yval, 3);

	yval = q2 & 0xff;
	SetMaskBit(yval, 4);
	yval = (q2<<16) >> 24;
	SetMaskBit(yval, 5);
	yval = (q2<<8) >> 24;
	SetMaskBit(yval, 6);
	yval = q2 >> 24;
	SetMaskBit(yval, 7);

	yval = q1 & 0xff;
	SetMaskBit(yval, 8);
	yval = (q1<<16) >> 24;
	SetMaskBit(yval, 9);
	yval = (q1<<8) >> 24;
	SetMaskBit(yval, 10);
	yval = q1 >> 24;
	SetMaskBit(yval, 11);

	yval = q0 & 0xff;
	SetMaskBit(yval, 12);
	yval = (q0<<16) >> 24;
	SetMaskBit(yval, 13);
	yval = (q0<<8) >> 24;
	SetMaskBit(yval, 14);
	yval = q0 >> 24;
    }

    /*
     * .... the last bit of the bitmask is calculated differently. if this
     * pixel, the one in the upper-left corner is less than the mean, the
     * last bit is calculated more-or-less normally. however, if this last
     * pixel is greater or equal to the mean the mask is complemented and
     * the meaning of ylo and yhi are reversed.... at this point the mean
     * value of the hi and lo pixels are calulated this is accomplished by
     * using the division lookup table. this table is index by the value of
     * the accumulated pixels and the number of pixel contributing to that
     * sum ....
     */
    if (yval < ymean) {
	ylo += yval;
	tmp += 1;
	ylo = cellb_divtable[tmp][ylo];
	tmp = 16 - tmp;
	yhi = cellb_divtable[tmp][yhi];
	ylo <<= 8;
    } else {
	yhi += yval;
	ylo = cellb_divtable[tmp][ylo];
	tmp = 16 - tmp;
	yhi = cellb_divtable[tmp][yhi];
	mask ^= 0x7fff;
	yhi <<= 8;
    }

    /*
     * .... the high mean and low mean values are finally quantized by the
     * cellb_yyremap[] table, the Y/Y vector codeword byte, the U/V
     * codeword byte and the bitmask are then assembled into a Normal Cell
     * bytecode and returned form the routine ....
     */
    yhi += ylo;
    yval = cellb_yyremap[yhi];

    mask <<= 16;
    uvindx <<= 8;
    mask += uvindx;
    mask += yval;

    return mask;
}

#define CountBits(COUNT, PATTERN)	\
    COUNT = PATTERN & mask5555; 	\
    PATTERN >>= 1;			\
    PATTERN &= mask5555;		\
    COUNT += PATTERN;			\
    PATTERN = COUNT & mask3333; 	\
    COUNT >>= 2;			\
    COUNT &= mask3333;			\
    COUNT += PATTERN;			\
    PATTERN = COUNT & 0x0f0f;	 	\
    COUNT >>= 4;			\
    COUNT &= 0x0f0f;			\
    COUNT += PATTERN;			\
    PATTERN = COUNT & 0xff;	 	\
    COUNT >>= 8;			\
    COUNT += PATTERN


static int CellB_Encode_Skip(int index, uint32 ccell)
{
    /*
     * .... this routine determines if a given cell should be skipped or
     * updated. To start, each cell should be updated approximately once
     * every 8 frames and never less that once in 10 frames. A
     * pseudo-random update strategy has been adopted since it tends to
     * equalize the number of bytes required for each frame, avoiding the a
     * sudden burst of activity every 10 frames for a relatively static
     * image .... Futher determination of when a cell should be skipped is
     * done by comparing the current, or update, cell value with the value
     * last transmitted for that cell position ....
     */

    if (cellb_count[index] != 0) {
	uint32 rmask, rtmp, ctmp;

	/*
	 * .... grab the UV fields of the reference and current cells,
	 * then check if the two fields are "close". this is
	 * accomplished with a table called cellb_uvlookup[]. this
	 * table is actually a bitvector where each bit-position is set
	 * or cleared according to whether, UV(old) is close to
	 * UV(new), The heuristic for closeness is embodied by the
	 * cellb_uvclose[] vector which in the first version was based
	 * on euclidean distances ....
	 */
	rmask = cellb_history[index];
	rtmp = (rmask<<16) >> 24;
	ctmp = (ccell<<16) >> 24;
	rtmp = (rtmp<<5) + (ctmp>>3);
	ctmp = ctmp & 7;

	if ((cellb_uvlookup[rtmp] & (1 << ctmp)) != 0) {
	    uint32 cmask, pattern;
	    int cy0, cy1, ry0, ry1;
	    int bits, diff, count;
	    uint32 mask5555 = 0x5555;
	    uint32 mask3333 = 0x3333;

	    /*
	     * .... if the colors were close then the total
	     * absolute luminance difference between the reference
	     * tile and the current tile is calculated ... The Y/Y
	     * vectors are first dequantized, via cellb_yytable[]
	     * lookups, and the difference corresponding to each of
	     * the possible bitmask differences between the
	     * reference mask and the current mask are found and
	     * multiplied by the number of occurances of each
	     * particular bit patterns.
	     * 
	     * Reference	Current Tile Bit	Tile Bit 0 |ry0
	     * - cy0| 0		1	  |ry0 - cy1| 1 |ry1 -
	     * cy0| 1		1	  |ry1 - cy1|
	     * 
	     * The multiplys and absolute value of the luminance
	     * differences are performed by a table lookup. the
	     * number of bits corresponding to a given condition is
	     * computed by the CountBits() macro, which returns the
	     * value in "count". Note: that the 0/0 condition is
	     * not actually computed but deduced from the number of
	     * bits remaining after the 1/1, 0/1, and 1/0
	     * conditions are counted....
	     * 
	     */

	    /*
	     * .... dequantize Y/Y values and get the luminance
	     * values ....
	     */
	    cy0 = ccell & 0xff;
	    ry0 = rmask & 0xff;
	    cmask = ccell >> 16;
	    rmask = rmask >> 16;
	    cy0 = cellb_yytable[cy0];
	    ry0 = cellb_yytable[ry0];
	    cy1 = cy0 & 0xff;
	    ry1 = 256 - (ry0 & 0xff);
	    cy0 = cy0 >> 8;
	    ry0 = 256 - (ry0 >> 8);

	    bits = 16;

	    /*
	     * .... find the 1/1 bits ....
	     */
	    pattern = cmask & rmask;
	    CountBits(count, pattern);
	    bits -= count;
	    diff = cellb_error[count][cy1+ry1];

	    /*
	     * .... find the 1/0 bits ....
	     */
	    pattern = cmask & ~rmask;
	    CountBits(count, pattern);
	    bits -= count;
	    diff += cellb_error[count][cy1+ry0];

	    /*
	     * .... find the 0/1 bits ....
	     */
	    pattern = ~cmask & rmask;
	    CountBits(count, pattern);
	    bits -= count;
	    diff += cellb_error[count][cy0+ry1];

	    /*
	     * .... add in error from the 0/0 bits ....
	     */
	    diff += cellb_error[bits][cy0+ry0];

	    /*
	     * .... if the total absolute luminance difference
	     * between the reference and the current cell are found
	     * to be less than 144 then skip the cell. this value
	     * of 144 was determined subjectively. 144 corresponds
	     * to an average luminace error of 9 per pixel which in
	     * most cases was barely visually noticable, and in
	     * typical cases, where the camera postion is fixed,
	     * the subject is agianist a static background and
	     * ocuppys approximately 50% of the displayed pixels,
	     * this threshold allowed ~80% of all cells to be
	     * skipped, with very little observable difference
	     * verses sending all cells ....
	     */

	    if (diff < 144) {
		cellb_count[index] -= 1;
		return 1;
	    }
	}
    }

    cellb_history[index] = ccell;
    cellb_count[index] = (randbyte() & 7) + 13;
    return 0;
}

#ifdef SUNVIDEO
static int CellB_HWEncode(vidimage_t *image, xmitproc_t *callback)
{
    int x, y, block_width=width/4;
    int len, pattern, cellx=0, celly=0;
    uint8 *data, *dataLim, *pack, *packLim;

    if (SunVideo_GrabCellB(&data, &len) == 0) return -1;

    rtp_frame_time = RTPTime();
    dataLim = data + len;
    while (data < dataLim) {
	pack = data;
	packLim = data + PACKETLEN;
	xilhdr.x = htons(cellx);
	xilhdr.y = htons(celly);

	while ((data < dataLim) && (data < packLim)) {
	    pattern = *data;
	    if (pattern >= SKIPCODE) {
		data++;
		cellx += (pattern - SKIPCODE + 1);
	    } else {
		data += 4;
		cellx++;
	    }

	    while (cellx >= block_width) {
		cellx -= block_width;
		celly++;
	    }
	}

	(*callback)((data < dataLim) ? 0 : 1, RTPCONT_CELLB, rtp_frame_time,
		    (uint8 *) &xilhdr, sizeof(xilhdr), pack, data-pack);
    }

    return 0;
}

/*ARGSUSED*/
static void CellB_HWEncode_Reconfig(int color, int w, int h)
{
    width = w;
    height = h;

    xilhdr.w = htons(width);
    xilhdr.h = htons(height);
}
#endif /* SUNVIDEO */

static int CellB_Encode(vidimage_t *image, xmitproc_t *callback)
{
    int bytes, tdiff, timeout, x, y, block_width=width/4, block_height=height/4;
    int offset, sync, pattern, res, skip=0, index=0;
    uint8 *data, *dataLim;
    uint8 *yp;
    int8 *uvp;
    struct timeval cur_time;

    if (!frame_ok) {
	rtp_frame_time = RTPTime();
	if ((*grab)(curr_y_data, curr_uv_data) == 0) return -1;
	frame_ok = 1;
    }

    offset = start_y*4*width + start_x*4;
    yp = &curr_y_data[offset];
    uvp = &curr_uv_data[offset];
    data = cellb_data;
    dataLim = cellb_data+PACKETLEN;
    index = start_y*block_width + start_x;

    xilhdr.x = htons(start_x);
    xilhdr.y = htons(start_y);
    for (y=start_y; y < block_height; y++) {
	for (x=start_x; x < block_width; x++) {
	    if (data >= dataLim) goto done;

	    res = CellB_Encode_DoCell(yp, uvp, block_width);
	    if (CellB_Encode_Skip(index, res)) {
		skip++;
		if (skip == MAXSKIP) {
		    *data++ = SKIPCODE + MAXSKIP-1;
		    skip = 0;
		}
	    } else {
		if (skip > 0) {
		    *data++ = SKIPCODE+skip-1;
		    skip = 0;
		}
		data[0] = res >> 24;
		data[1] = res >> 16;
		data[2] = res >> 8;
		data[3] = res;
		data += 4;
	    }

	    index++;
	    yp += 4;
	    uvp += 4;
	}

	yp += 3*width;
	uvp += 3*width;
	start_x = 0;
    }

done:
    if (y == block_height) {
	frame_ok = start_x = start_y = 0;
	sync = 1;
    } else {
	start_x = x;
	start_y = y;
	sync = 0;
    }

    (*callback)(sync, RTPCONT_CELLB, rtp_frame_time,
		(uint8 *)&xilhdr, sizeof(xilhdr), cellb_data, data-cellb_data);

    gettimeofday(&cur_time, NULL);
    bytes = data-cellb_data;
    tdiff = (cur_time.tv_sec-xmit_time.tv_sec)*1000 +
	    (cur_time.tv_usec-xmit_time.tv_usec)/1000;
    if ((timeout = bytes*8/bandwidth-tdiff) < 0) timeout = 0;

    if (sync && (framerate > 0)) {
	int frame_timeout;

	tdiff = (cur_time.tv_sec - frame_time.tv_sec) * 1000 +
		(cur_time.tv_usec - frame_time.tv_usec) / 1000;
	if ((frame_timeout = 1000/framerate-tdiff) > timeout)
	    timeout = frame_timeout;
    }

    xmit_time = cur_time;
    xmit_time.tv_usec += timeout*1000;
    if (sync) frame_time = xmit_time;

    return timeout;
}

/*ARGSUSED*/
static void CellB_Encode_Reconfig(int color, int w, int h)
{
    int i, size;

    width = w;
    height = h;

    xilhdr.w = htons(width);
    xilhdr.h = htons(height);

    size = height * width;
    curr_y_data = (uint8 *)malloc(size);
    curr_uv_data = (int8 *)malloc(size);
    cellb_data = (uint8 *)malloc(size);
    if (!color) memset(curr_uv_data, 0, size);

    size /= 16;
    cellb_count = (uint8 *)malloc(size*sizeof(uint8));
    cellb_history = (uint32 *)malloc(size*sizeof(int));

    for (i=0; i<size; i++) {
	cellb_count[i] = 0;
	cellb_history[i] = 0xffffffff;
    }

    frame_ok = start_x = start_y = 0;
}

/*ARGSUSED*/
int CellB_Encode_Probe(grabber_t *grabber)
{
    int i, j;
    uint8 *dptr;

    /* .... initialize division table ....  */
    cellb_divtable[0] = dptr = cellb_dtbl;
    for (i=1; i <= 16; i++) {
	cellb_divtable[i] = dptr;
	for (j=0; j < 256*i; j++) *dptr++ = j / i;
    }

    /* .... initialize uv closeness look-up table ....  */
    for (i=0; i < 252; i++) {
	int u0 = cellb_uvtable[i];
	int v0 = u0 & 0xff;

	u0 = u0 >> 8;
	for (j=0; j < 252; j++) {
	    int index;
	    int u1 = cellb_uvtable[j];
	    int v1 = u1 & 0xff;

	    u1 = u1 >> 8;
	    u1 = u1 - u0;
	    v1 = v1 - v0;
	    if (UVCLOSE(u1, v1)) {
		index = (i << 5) | (j >> 3);
		cellb_uvlookup[index] |= 1 << (j & 7);
	    }
	}
    }

    /* .... initialize error mult table ....  */
    for (j=0; j < 17; j++) {
	for (i=0; i < 512; i++) {
	    int index = i-256;
	    int absindex = (index < 0) ? -index : index;
	    int tmp = absindex * j;

	    tmp = (tmp > 255) ? 255 : tmp;
	    cellb_error[j][i] = tmp;
	}
    }

    return 1;
}

/*ARGSUSED*/
encodeproc_t *CellB_Encode_Start(grabber_t *grabber, int max_bandwidth,
				 int max_framerate, int config)
{
#ifdef SUNVIDEO
    if (!strcmp(grabber->keyword, "sunvideo")) {
	if (SunVideo_StartCellB(max_bandwidth, max_framerate, config,
				CellB_HWEncode_Reconfig) == 0) return 0;

	return CellB_HWEncode;
    }
#endif

    if ((grab = grabber->start(max_framerate, config,
			       CellB_Encode_Reconfig)) == 0) return 0;

    bandwidth = max_bandwidth;
    framerate = max_framerate;

    gettimeofday(&xmit_time, NULL);
    frame_time = xmit_time;
    return CellB_Encode;
}

void CellB_Encode_Stop(grabber_t *grabber)
{
#ifdef SUNVIDEO
    if (!strcmp(grabber->keyword, "sunvideo")) {
	SunVideo_StopCellB();
	return;
    }
#endif

    grabber->stop();

    free(curr_y_data);
    free(curr_uv_data);
    free(cellb_data);
    free(cellb_history);
    free(cellb_count);
}
