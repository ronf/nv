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
#include <netinet/in.h>
#ifdef AIX
#include <net/nh.h>
#endif
#include <X11/Xlib.h>
#include "sized_types.h"
#include "vid_util.h"
#include "vid_image.h"
#include "vid_code.h"
#include "cellb.h"

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

static uint8 cellb_dtbl[34816], *cellb_divtable[17];
static uint8 cellb_uvlookup[8192], cellb_error[17][512];

typedef struct {
    grabber_t *grabber;
    grabproc_t *grab;
    int max_bandwidth, min_framespacing, config, hw_encode;
    int grabtype, width, height, start_x, start_y, frame_ok;
    uint32 rtp_frame_time;
    uint8 *grabdata;
    uint8 *cellb, *cellbLim;
    uint8 *cellb_count;
    uint32 *cellb_history;
    struct timeval xmit_time, frame_time;
    uint32 (*docell)(uint8 *imgp, int stride);
} cellb_state_t;

typedef struct {
    uint16 x, y;
    uint16 w, h;
} cellbhdr_t;

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

static uint32 CellB_Encode_DoCell_MSB_YUYV(uint8 *imgp, int stride)
{
    int ymean, umean, vmean, uvindx;
    uint32 mask, yhi, ylo, tmp, yval;
    uint32 t0, t1, t2, t3, t4, t5, t6, t7;

    t0 = ((uint32 *)imgp)[0];
    t1 = ((uint32 *)imgp)[1];
    t2 = ((uint32 *)imgp)[2 * stride];
    t3 = ((uint32 *)imgp)[2 * stride + 1];
    t4 = ((uint32 *)imgp)[4 * stride];
    t5 = ((uint32 *)imgp)[4 * stride + 1];
    t6 = ((uint32 *)imgp)[6 * stride];
    t7 = ((uint32 *)imgp)[6 * stride + 1];

    ymean  = t0 >> 24;       ymean += (t0<<16) >> 24;
    umean  = (t0<<8)  >> 24; vmean  = t0 & 0xff;
    ymean += t1 >> 24;       ymean += (t1<<16) >> 24;
    umean += (t1<<8)  >> 24; vmean += t1 & 0xff;
    ymean += t2 >> 24;       ymean += (t2<<16) >> 24;
    umean += (t2<<8)  >> 24; vmean += t2 & 0xff;
    ymean += t3 >> 24;       ymean += (t3<<16) >> 24;
    umean += (t3<<8)  >> 24; vmean += t3 & 0xff;
    ymean += t4 >> 24;       ymean += (t4<<16) >> 24;
    umean += (t4<<8)  >> 24; vmean += t4 & 0xff;
    ymean += t5 >> 24;       ymean += (t5<<16) >> 24;
    umean += (t5<<8)  >> 24; vmean += t5 & 0xff;
    ymean += t6 >> 24;       ymean += (t6<<16) >> 24;
    umean += (t6<<8)  >> 24; vmean += t6 & 0xff;
    ymean += t7 >> 24;       ymean += (t7<<16) >> 24;
    umean += (t7<<8)  >> 24; vmean += t7 & 0xff;

    umean >>= 5; vmean >>= 5;
    tmp = (umean<<6) | vmean;
    uvindx = cellb_uvremap[tmp];

    mask = 0;
    ylo = 0;
    yhi = 0;
    tmp = 0;
    ymean >>= 4;

    yval = (t7<<16) >> 24;
    SetMaskBit(yval, 0);
    yval = t7 >> 24;
    SetMaskBit(yval, 1);
    yval = (t6<<16) >> 24;
    SetMaskBit(yval, 2);
    yval = t6 >> 24;
    SetMaskBit(yval, 3);

    yval = (t5<<16) >> 24;
    SetMaskBit(yval, 4);
    yval = t5 >> 24;
    SetMaskBit(yval, 5);
    yval = (t4<<16) >> 24;
    SetMaskBit(yval, 6);
    yval = t4 >> 24;
    SetMaskBit(yval, 7);

    yval = (t3<<16) >> 24;
    SetMaskBit(yval, 8);
    yval = t3 >> 24;
    SetMaskBit(yval, 9);
    yval = (t2<<16) >> 24;
    SetMaskBit(yval, 10);
    yval = t2 >> 24;
    SetMaskBit(yval, 11);

    yval = (t1<<16) >> 24;
    SetMaskBit(yval, 12);
    yval = t1 >> 24;
    SetMaskBit(yval, 13);
    yval = (t0<<16) >> 24;
    SetMaskBit(yval, 14);
    yval = t0 >> 24;

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

    yhi += ylo;
    yval = cellb_yyremap[yhi];

    mask <<= 16;
    uvindx <<= 8;
    mask += uvindx;
    mask += yval;

    return mask;
}

static uint32 CellB_Encode_DoCell_LSB_YUYV(uint8 *imgp, int stride)
{
    int ymean, umean, vmean, uvindx;
    uint32 mask, yhi, ylo, tmp, yval;
    uint32 t0, t1, t2, t3, t4, t5, t6, t7;

    t0 = ((uint32 *)imgp)[0];
    t1 = ((uint32 *)imgp)[1];
    t2 = ((uint32 *)imgp)[2 * stride];
    t3 = ((uint32 *)imgp)[2 * stride + 1];
    t4 = ((uint32 *)imgp)[4 * stride];
    t5 = ((uint32 *)imgp)[4 * stride + 1];
    t6 = ((uint32 *)imgp)[6 * stride];
    t7 = ((uint32 *)imgp)[6 * stride + 1];

    ymean  = (t0<<8)  >> 24; ymean += t0 & 0xff;
    vmean  = t0 >> 24;       umean  = (t0<<16) >> 24;
    ymean += (t1<<8)  >> 24; ymean += t1 & 0xff;
    vmean += t1 >> 24;       umean += (t1<<16) >> 24;
    ymean += (t2<<8)  >> 24; ymean += t2 & 0xff;
    vmean += t2 >> 24;       umean += (t2<<16) >> 24;
    ymean += (t3<<8)  >> 24; ymean += t3 & 0xff;
    vmean += t3 >> 24;       umean += (t3<<16) >> 24;
    ymean += (t4<<8)  >> 24; ymean += t4 & 0xff;
    vmean += t4 >> 24;       umean += (t4<<16) >> 24;
    ymean += (t5<<8)  >> 24; ymean += t5 & 0xff;
    vmean += t5 >> 24;       umean += (t5<<16) >> 24;
    ymean += (t6<<8)  >> 24; ymean += t6 & 0xff;
    vmean += t6 >> 24;       umean += (t6<<16) >> 24;
    ymean += (t7<<8)  >> 24; ymean += t7 & 0xff;
    vmean += t7 >> 24;       umean += (t7<<16) >> 24;

    umean >>= 5; vmean >>= 5;
    tmp = (umean<<6) | vmean;
    uvindx = cellb_uvremap[tmp];

    mask = 0;
    ylo = 0;
    yhi = 0;
    tmp = 0;
    ymean >>= 4;

    yval = (t7<<8) >> 24;
    SetMaskBit(yval, 0);
    yval = t7 & 0xff;
    SetMaskBit(yval, 1);
    yval = (t6<<8) >> 24;
    SetMaskBit(yval, 2);
    yval = t6 & 0xff;
    SetMaskBit(yval, 3);

    yval = (t5<<8) >> 24;
    SetMaskBit(yval, 4);
    yval = t5 & 0xff;
    SetMaskBit(yval, 5);
    yval = (t4<<8) >> 24;
    SetMaskBit(yval, 6);
    yval = t4 & 0xff;
    SetMaskBit(yval, 7);

    yval = (t3<<8) >> 24;
    SetMaskBit(yval, 8);
    yval = t3 & 0xff;
    SetMaskBit(yval, 9);
    yval = (t2<<8) >> 24;
    SetMaskBit(yval, 10);
    yval = t2 & 0xff;
    SetMaskBit(yval, 11);

    yval = (t1<<8) >> 24;
    SetMaskBit(yval, 12);
    yval = t1 & 0xff;
    SetMaskBit(yval, 13);
    yval = (t0<<8) >> 24;
    SetMaskBit(yval, 14);
    yval = t0 & 0xff;

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

    yhi += ylo;
    yval = cellb_yyremap[yhi];

    mask <<= 16;
    uvindx <<= 8;
    mask += uvindx;
    mask += yval;

    return mask;
}

static uint32 CellB_Encode_DoCell_MSB_UYVY(uint8 *imgp, int stride)
{
    int ymean, umean, vmean, uvindx;
    uint32 mask, yhi, ylo, tmp, yval;
    uint32 t0, t1, t2, t3, t4, t5, t6, t7;

    t0 = ((uint32 *)imgp)[0];
    t1 = ((uint32 *)imgp)[1];
    t2 = ((uint32 *)imgp)[2 * stride];
    t3 = ((uint32 *)imgp)[2 * stride + 1];
    t4 = ((uint32 *)imgp)[4 * stride];
    t5 = ((uint32 *)imgp)[4 * stride + 1];
    t6 = ((uint32 *)imgp)[6 * stride];
    t7 = ((uint32 *)imgp)[6 * stride + 1];

    ymean  = (t0<<8)  >> 24; ymean += t0 & 0xff;
    umean  = t0 >> 24;       vmean  = (t0<<16) >> 24;
    ymean += (t1<<8)  >> 24; ymean += t1 & 0xff;
    umean += t1 >> 24;       vmean += (t1<<16) >> 24;
    ymean += (t2<<8)  >> 24; ymean += t2 & 0xff;
    umean += t2 >> 24;       vmean += (t2<<16) >> 24;
    ymean += (t3<<8)  >> 24; ymean += t3 & 0xff;
    umean += t3 >> 24;       vmean += (t3<<16) >> 24;
    ymean += (t4<<8)  >> 24; ymean += t4 & 0xff;
    umean += t4 >> 24;       vmean += (t4<<16) >> 24;
    ymean += (t5<<8)  >> 24; ymean += t5 & 0xff;
    umean += t5 >> 24;       vmean += (t5<<16) >> 24;
    ymean += (t6<<8)  >> 24; ymean += t6 & 0xff;
    umean += t6 >> 24;       vmean += (t6<<16) >> 24;
    ymean += (t7<<8)  >> 24; ymean += t7 & 0xff;
    umean += t7 >> 24;       vmean += (t7<<16) >> 24;

    umean >>= 5; vmean >>= 5;
    tmp = (umean<<6) | vmean;
    uvindx = cellb_uvremap[tmp];

    mask = 0;
    ylo = 0;
    yhi = 0;
    tmp = 0;
    ymean >>= 4;

    yval = t7 & 0xff;
    SetMaskBit(yval, 0);
    yval = (t7<<8) >> 24;
    SetMaskBit(yval, 1);
    yval = t6 & 0xff;
    SetMaskBit(yval, 2);
    yval = (t6<<8) >> 24;
    SetMaskBit(yval, 3);

    yval = t5 & 0xff;
    SetMaskBit(yval, 4);
    yval = (t5<<8) >> 24;
    SetMaskBit(yval, 5);
    yval = t4 & 0xff;
    SetMaskBit(yval, 6);
    yval = (t4<<8) >> 24;
    SetMaskBit(yval, 7);

    yval = t3 & 0xff;
    SetMaskBit(yval, 8);
    yval = (t3<<8) >> 24;
    SetMaskBit(yval, 9);
    yval = t2 & 0xff;
    SetMaskBit(yval, 10);
    yval = (t2<<8) >> 24;
    SetMaskBit(yval, 11);

    yval = t1 & 0xff;
    SetMaskBit(yval, 12);
    yval = (t1<<8) >> 24;
    SetMaskBit(yval, 13);
    yval = t0 & 0xff;
    SetMaskBit(yval, 14);
    yval = (t0<<8) >> 24;

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

    yhi += ylo;
    yval = cellb_yyremap[yhi];

    mask <<= 16;
    uvindx <<= 8;
    mask += uvindx;
    mask += yval;

    return mask;
}

static uint32 CellB_Encode_DoCell_LSB_UYVY(uint8 *imgp, int stride)
{
    int ymean, umean, vmean, uvindx;
    uint32 mask, yhi, ylo, tmp, yval;
    uint32 t0, t1, t2, t3, t4, t5, t6, t7;

    t0 = ((uint32 *)imgp)[0];
    t1 = ((uint32 *)imgp)[1];
    t2 = ((uint32 *)imgp)[2 * stride];
    t3 = ((uint32 *)imgp)[2 * stride + 1];
    t4 = ((uint32 *)imgp)[4 * stride];
    t5 = ((uint32 *)imgp)[4 * stride + 1];
    t6 = ((uint32 *)imgp)[6 * stride];
    t7 = ((uint32 *)imgp)[6 * stride + 1];

    ymean  = t0 >> 24;       ymean += (t0<<16) >> 24;
    vmean  = (t0<<8)  >> 24; umean  = t0 & 0xff;
    ymean += t1 >> 24;       ymean += (t1<<16) >> 24;
    vmean += (t1<<8)  >> 24; umean += t1 & 0xff;
    ymean += t2 >> 24;       ymean += (t2<<16) >> 24;
    vmean += (t2<<8)  >> 24; umean += t2 & 0xff;
    ymean += t3 >> 24;       ymean += (t3<<16) >> 24;
    vmean += (t3<<8)  >> 24; umean += t3 & 0xff;
    ymean += t4 >> 24;       ymean += (t4<<16) >> 24;
    vmean += (t4<<8)  >> 24; umean += t4 & 0xff;
    ymean += t5 >> 24;       ymean += (t5<<16) >> 24;
    vmean += (t5<<8)  >> 24; umean += t5 & 0xff;
    ymean += t6 >> 24;       ymean += (t6<<16) >> 24;
    vmean += (t6<<8)  >> 24; umean += t6 & 0xff;
    ymean += t7 >> 24;       ymean += (t7<<16) >> 24;
    vmean += (t7<<8)  >> 24; umean += t7 & 0xff;

    umean >>= 5; vmean >>= 5;
    tmp = (umean<<6) | vmean;
    uvindx = cellb_uvremap[tmp];

    mask = 0;
    ylo = 0;
    yhi = 0;
    tmp = 0;
    ymean >>= 4;

    yval = t7 >> 24;
    SetMaskBit(yval, 0);
    yval = (t7<<16) >> 24;
    SetMaskBit(yval, 1);
    yval = t6 >> 24;
    SetMaskBit(yval, 2);
    yval = (t6<<16) >> 24;
    SetMaskBit(yval, 3);

    yval = t5 >> 24;
    SetMaskBit(yval, 4);
    yval = (t5<<16) >> 24;
    SetMaskBit(yval, 5);
    yval = t4 >> 24;
    SetMaskBit(yval, 6);
    yval = (t4<<16) >> 24;
    SetMaskBit(yval, 7);

    yval = t3 >> 24;
    SetMaskBit(yval, 8);
    yval = (t3<<16) >> 24;
    SetMaskBit(yval, 9);
    yval = t2 >> 24;
    SetMaskBit(yval, 10);
    yval = (t2<<16) >> 24;
    SetMaskBit(yval, 11);

    yval = t1 >> 24;
    SetMaskBit(yval, 12);
    yval = (t1<<16) >> 24;
    SetMaskBit(yval, 13);
    yval = t0 >> 24;
    SetMaskBit(yval, 14);
    yval = (t0<<16) >> 24;

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


static int CellB_Encode_Skip(cellb_state_t *state, int index, uint32 ccell)
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

    if (state->cellb_count[index] != 0) {
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
	rmask = state->cellb_history[index];
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
		state->cellb_count[index] -= 1;
		return 1;
	    }
	}
    }

    state->cellb_history[index] = ccell;
    state->cellb_count[index] = (randbyte() & 7) + 13;
    return 0;
}

static int CellB_HWEncode(cellb_state_t *state, vidimage_t *image, uint8 *data,
			  int *lenp, int *markerp, uint32 *timestampp)
{
    int bytes, tdiff, timeout, x, y, block_width=state->width/4;
    int sync, len, pattern;
    cellbhdr_t *cellbhdr;
    uint8 *p, *packLim;
    struct timeval cur_time;

    if (!state->frame_ok) {
	state->rtp_frame_time = RTPTime();
	if ((*state->grab)(&state->cellb, &len) == 0) return -1;
	state->cellbLim = state->cellb+len;
	state->start_x = state->start_y = 0;
	state->frame_ok = 1;
    }

    cellbhdr = (cellbhdr_t *)data;
    cellbhdr->x = htons(state->start_x);
    cellbhdr->y = htons(state->start_y);
    cellbhdr->w = htons(state->width);
    cellbhdr->h = htons(state->height);

    p = state->cellb;
    packLim = p+*lenp-sizeof(*cellbhdr)-5;
    while ((p < packLim) && (p < state->cellbLim)) {
	pattern = *p;
	if (pattern >= SKIPCODE) {
	    p++;
	    state->start_x += (pattern - SKIPCODE + 1);
	} else {
	    p += 4;
	    state->start_x++;
	}

	while (state->start_x >= block_width) {
	    state->start_x -= block_width;
	    state->start_y++;
	}
    }

    bytes = p-state->cellb;
    memcpy(cellbhdr+1, state->cellb, bytes);
    bytes += sizeof(*cellbhdr);

    *markerp = sync = (p == state->cellbLim);
    *timestampp = state->rtp_frame_time;
    *lenp = bytes;
    state->cellb = p;
    state->frame_ok = !sync;

    gettimeofday(&cur_time, NULL);
    tdiff = (cur_time.tv_sec-state->xmit_time.tv_sec)*1000 +
	    (cur_time.tv_usec-state->xmit_time.tv_usec)/1000;
    if ((timeout = bytes*8/state->max_bandwidth-tdiff) < 0) timeout = 0;

    if (sync && (state->min_framespacing > 0)) {
	int frame_timeout;

	tdiff = (cur_time.tv_sec - state->frame_time.tv_sec) * 1000 +
		(cur_time.tv_usec - state->frame_time.tv_usec) / 1000;
	if ((frame_timeout = state->min_framespacing-tdiff) > timeout)
	    timeout = frame_timeout;
    }

    state->xmit_time = cur_time;
    state->xmit_time.tv_usec += timeout*1000;
    if (sync) state->frame_time = state->xmit_time;

    return timeout;
}

static int CellB_SWEncode(cellb_state_t *state, vidimage_t *image, uint8 *data,
			  int *lenp, int *markerp, uint32 *timestampp)
{
    int len, bytes, tdiff, timeout, x, y, w=state->width, h=state->height;
    int block_width=w/4, block_height=h/4;
    int offset, sync, pattern, res, skip=0, index=0;
    cellbhdr_t *cellbhdr;
    uint8 *p, *dataLim;
    uint8 *imgp;
    struct timeval cur_time;

    dataLim = data+*lenp-5;

    if (!state->frame_ok) {
	state->rtp_frame_time = RTPTime();
	if ((*state->grab)(&state->grabdata, &len) == 0) return -1;
	state->frame_ok = 1;
    }

    offset = state->start_y*8*w + state->start_x*8;
    imgp = &state->grabdata[offset];
    index = state->start_y*block_width + state->start_x;

    cellbhdr = (cellbhdr_t *)data;
    cellbhdr->x = htons(state->start_x);
    cellbhdr->y = htons(state->start_y);
    cellbhdr->w = htons(w);
    cellbhdr->h = htons(h);

    p = (uint8 *)(cellbhdr+1);
    for (y=state->start_y; y < block_height; y++) {
	for (x=state->start_x; x < block_width; x++) {
	    if (p >= dataLim) goto done;

	    res = (*state->docell)(imgp, block_width);
	    if (CellB_Encode_Skip(state, index, res)) {
		skip++;
		if (skip == MAXSKIP) {
		    *p++ = SKIPCODE + MAXSKIP-1;
		    skip = 0;
		}
	    } else {
		if (skip > 0) {
		    *p++ = SKIPCODE+skip-1;
		    skip = 0;
		}
		p[0] = res >> 24;
		p[1] = res >> 16;
		p[2] = res >> 8;
		p[3] = res;
		p += 4;
	    }

	    index++;
	    imgp += 8;
	}

	imgp += 6*w;
	state->start_x = 0;
    }

done:
    if (y == block_height) {
	state->start_x = state->start_y = state->frame_ok = 0;
	sync = 1;
    } else {
	state->start_x = x;
	state->start_y = y;
	sync = 0;
    }

    bytes = p-data;
    *markerp = sync;
    *timestampp = state->rtp_frame_time;
    *lenp = bytes;

    gettimeofday(&cur_time, NULL);
    tdiff = (cur_time.tv_sec-state->xmit_time.tv_sec)*1000 +
	    (cur_time.tv_usec-state->xmit_time.tv_usec)/1000;
    if ((timeout = bytes*8/state->max_bandwidth-tdiff) < 0) timeout = 0;

    if (sync && (state->min_framespacing > 0)) {
	int frame_timeout;

	tdiff = (cur_time.tv_sec - state->frame_time.tv_sec) * 1000 +
		(cur_time.tv_usec - state->frame_time.tv_usec) / 1000;
	if ((frame_timeout = state->min_framespacing-tdiff) > timeout)
	    timeout = frame_timeout;
    }

    state->xmit_time = cur_time;
    state->xmit_time.tv_usec += timeout*1000;
    if (sync) state->frame_time = state->xmit_time;

    return timeout;
}

static void CellB_Encode_Reconfig(void *enc_state, int w, int h)
{
    cellb_state_t *state=enc_state;
    int i, image_size, block_count;

    state->width = w;
    state->height = h;

    image_size = w*h;
    block_count = w*h/16;

    if (state->cellb_count != NULL) free(state->cellb_count);
    state->cellb_count = (uint8 *)malloc(block_count*sizeof(uint8));

    if (state->cellb_history != NULL) free(state->cellb_history);
    state->cellb_history = (uint32 *)malloc(block_count*sizeof(uint32));

    for (i=0; i<block_count; i++) {
	state->cellb_count[i] = 0;
	state->cellb_history[i] = 0xffffffff;
    }

    state->start_x = state->start_y = state->frame_ok = 0;
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
void *CellB_Encode_Start(grabber_t *grabber, int max_bandwidth,
			 int min_framespacing, int config)
{
    cellb_state_t *state;

    if ((state = (cellb_state_t *)malloc(sizeof(cellb_state_t))) == NULL)
	return 0;
    memset(state, 0, sizeof(cellb_state_t));

    if ((state->grab = (*grabber->start)(VIDIMAGE_CELLB, min_framespacing,
					 config, CellB_Encode_Reconfig,
					 state)) != 0) {
	state->grabtype = VIDIMAGE_CELLB;
    } else if ((state->grab = (*grabber->start)(VIDIMAGE_YUYV,
						min_framespacing,
						config, CellB_Encode_Reconfig,
						state)) != 0) {
	state->grabtype = VIDIMAGE_YUYV;
	if (LITTLEENDIAN)
	    state->docell = CellB_Encode_DoCell_LSB_YUYV;
	else
	    state->docell = CellB_Encode_DoCell_MSB_YUYV;
    } else if ((state->grab = (*grabber->start)(VIDIMAGE_UYVY,
						min_framespacing,
						config, CellB_Encode_Reconfig,
						state)) != 0) {
	state->grabtype = VIDIMAGE_UYVY;
	if (LITTLEENDIAN)
	    state->docell = CellB_Encode_DoCell_LSB_UYVY;
	else
	    state->docell = CellB_Encode_DoCell_MSB_UYVY;
    } else {
	free(state);
	return 0;
    }

    state->grabber = grabber;
    state->max_bandwidth = max_bandwidth;
    state->min_framespacing = min_framespacing;
    state->config = config;

    gettimeofday(&state->xmit_time, NULL);
    state->frame_time = state->xmit_time;

    return state;
}

void *CellB_Encode_Restart(void *enc_state, int max_bandwidth,
			   int min_framespacing, int config)
{
    cellb_state_t *state=enc_state;

    if ((min_framespacing != state->min_framespacing) ||
	(config != state->config)) {
	if ((state->grab = (*state->grabber->start)(state->grabtype,
						    min_framespacing, config,
						    CellB_Encode_Reconfig,
						    state)) == 0) {
	    free(state);
	    return 0;
	}
    }

    state->max_bandwidth = max_bandwidth;
    state->min_framespacing = min_framespacing;
    state->config = config;

    gettimeofday(&state->xmit_time, NULL);
    state->frame_time = state->xmit_time;

    return state;
}

int CellB_Encode(void *enc_state, vidimage_t *image, uint8 *data,
		 int *lenp, int *markerp, uint32 *timestampp)
{
    cellb_state_t *state=enc_state;

    if (state->grabtype == VIDIMAGE_CELLB) {
	return CellB_HWEncode(state, image, data, lenp, markerp, timestampp);
    } else {
	return CellB_SWEncode(state, image, data, lenp, markerp, timestampp);
    }
}

void CellB_Encode_Stop(void *enc_state)
{
    cellb_state_t *state=enc_state;

    (*state->grabber->stop)();

    if (state->cellb_history != NULL) free(state->cellb_history);
    if (state->cellb_count != NULL) free(state->cellb_count);

    free(state);
}
