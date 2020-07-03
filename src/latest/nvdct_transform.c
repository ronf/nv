/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>

	NV DCT video transform routines
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
#include "sized_types.h"
#include "vid_image.h"
#include "vid_code.h"

/* Sick little macro which will limit x to [0..255] with logical ops */
#define UCLIMIT(x) ((t = (x)), (t &= ~(t>>31)), (t | ~((t-256) >> 31)))

static void NVDCT_FwdTransform_Grey(uint8 *imgp, int width, uint32 *out)
{
    int i, t0, t1, *dataptr;
    int a0, a1, a2, a3, b0, b1, b2, b3, c0, c1, c2, c3;
    int8 *outcptr=(int8 *)out;
    static int block[64];

    dataptr = block;
    for (i=0; i<8; i++) {
	t0 = imgp[0];
	t1 = imgp[7*width];
	a0 = t0+t1;
	c3 = t0-t1;
	imgp += width;

	t0 = imgp[0];
	t1 = imgp[5*width];
	a1 = t0+t1;
	c2 = t0-t1;
	imgp += width;

	t0 = imgp[0];
	t1 = imgp[3*width];
	a2 = t0+t1;
	c1 = t0-t1;
	imgp += width;

	t0 = imgp[0];
	t1 = imgp[width];
	a3 = t0+t1;
	c0 = t0-t1;
	imgp -= 3*width;

	b0 = a0+a3;
	b1 = a1+a2;
	b2 = a1-a2;
	b3 = a0-a3;

	dataptr[0]  = (362 * (b0+b1-1024)) >> 7; /* Correct for DC offset */
	dataptr[32] = (362 * (b0-b1))      >> 7;
	dataptr[16] = (196*b2 + 473*b3)    >> 7;
	dataptr[48] = (196*b3 - 473*b2)    >> 7;

	b0 = (362 * (c2-c1)) >> 7;
	b1 = (362 * (c2+c1)) >> 7;
	c0 = c0 << 2;
	c3 = c3 << 2;

	a0 = c0+b0;
	a1 = c0-b0;
	a2 = c3-b1;
	a3 = c3+b1;

	dataptr[8]  = (100*a0 + 502*a3) >> 9;
	dataptr[24] = (426*a2 - 284*a1) >> 9;
	dataptr[40] = (426*a1 + 284*a2) >> 9;
	dataptr[56] = (100*a3 - 502*a0) >> 9;

	imgp++;
	dataptr++;
    }

    dataptr = block;
    for (i=0; i<8; i++) {
	t0 = dataptr[0];
	t1 = dataptr[7];
	a0 = t0+t1;
	c3 = t0-t1;

	t0 = dataptr[1];
	t1 = dataptr[6];
	a1 = t0+t1;
	c2 = t0-t1;

	t0 = dataptr[2];
	t1 = dataptr[5];
	a2 = t0+t1;
	c1 = t0-t1;

	t0 = dataptr[3];
	t1 = dataptr[4];
	a3 = t0+t1;
	c0 = t0-t1;

	b0 = a0+a3;
	b1 = a1+a2;
	b2 = a1-a2;
	b3 = a0-a3;

	outcptr[0] = (362 * (b0+b1)   + 32768) >> 16;
	outcptr[4] = (362 * (b0-b1)   + 32768) >> 16;
	outcptr[2] = (196*b2 + 473*b3 + 32768) >> 16;
	outcptr[6] = (196*b3 - 473*b2 + 32768) >> 16;

	b0 = (362 * (c2-c1)) >> 9;
	b1 = (362 * (c2+c1)) >> 9;

	a0 = c0+b0;
	a1 = c0-b0;
	a2 = c3-b1;
	a3 = c3+b1;

	outcptr[1] = (100*a0 + 502*a3 + 32768) >> 16;
	outcptr[3] = (426*a2 - 284*a1 + 32768) >> 16;
	outcptr[5] = (426*a1 + 284*a2 + 32768) >> 16;
	outcptr[7] = (100*a3 - 502*a0 + 32768) >> 16;

	dataptr += 8;
	outcptr += 8;
    }
}

void NVDCT_FwdTransform(int grabtype, uint8 *imgp, int width, uint32 *out)
{
    int i, t0, t1, *dataptr;
    int a0, a1, a2, a3, b0, b1, b2, b3, c0, c1, c2, c3;
    int8 *outcptr=(int8 *)out;
    static int block[64];

    if (grabtype == VIDIMAGE_GREY) {
	NVDCT_FwdTransform_Grey(imgp, width, out);
	return;
    }

    width *= 2;

    dataptr = block;
    if (grabtype == VIDIMAGE_UYVY) imgp++;
    for (i=0; i<8; i++) {
	t0 = imgp[0];
	t1 = imgp[7*width];
	a0 = t0+t1;
	c3 = t0-t1;
	imgp += width;

	t0 = imgp[0];
	t1 = imgp[5*width];
	a1 = t0+t1;
	c2 = t0-t1;
	imgp += width;

	t0 = imgp[0];
	t1 = imgp[3*width];
	a2 = t0+t1;
	c1 = t0-t1;
	imgp += width;

	t0 = imgp[0];
	t1 = imgp[width];
	a3 = t0+t1;
	c0 = t0-t1;
	imgp -= 3*width;

	b0 = a0+a3;
	b1 = a1+a2;
	b2 = a1-a2;
	b3 = a0-a3;

	dataptr[0]  = (362 * (b0+b1-1024)) >> 7; /* Correct for DC offset */
	dataptr[32] = (362 * (b0-b1))      >> 7;
	dataptr[16] = (196*b2 + 473*b3)    >> 7;
	dataptr[48] = (196*b3 - 473*b2)    >> 7;

	b0 = (362 * (c2-c1)) >> 7;
	b1 = (362 * (c2+c1)) >> 7;
	c0 = c0 << 2;
	c3 = c3 << 2;

	a0 = c0+b0;
	a1 = c0-b0;
	a2 = c3-b1;
	a3 = c3+b1;

	dataptr[8]  = (100*a0 + 502*a3) >> 9;
	dataptr[24] = (426*a2 - 284*a1) >> 9;
	dataptr[40] = (426*a1 + 284*a2) >> 9;
	dataptr[56] = (100*a3 - 502*a0) >> 9;

	imgp += 2;
	dataptr++;
    }

    dataptr = block;
    for (i=0; i<8; i++) {
	t0 = dataptr[0];
	t1 = dataptr[7];
	a0 = t0+t1;
	c3 = t0-t1;

	t0 = dataptr[1];
	t1 = dataptr[6];
	a1 = t0+t1;
	c2 = t0-t1;

	t0 = dataptr[2];
	t1 = dataptr[5];
	a2 = t0+t1;
	c1 = t0-t1;

	t0 = dataptr[3];
	t1 = dataptr[4];
	a3 = t0+t1;
	c0 = t0-t1;

	b0 = a0+a3;
	b1 = a1+a2;
	b2 = a1-a2;
	b3 = a0-a3;

	outcptr[0] = (362 * (b0+b1)   + 32768) >> 16;
	outcptr[4] = (362 * (b0-b1)   + 32768) >> 16;
	outcptr[2] = (196*b2 + 473*b3 + 32768) >> 16;
	outcptr[6] = (196*b3 - 473*b2 + 32768) >> 16;

	b0 = (362 * (c2-c1)) >> 9;
	b1 = (362 * (c2+c1)) >> 9;

	a0 = c0+b0;
	a1 = c0-b0;
	a2 = c3-b1;
	a3 = c3+b1;

	outcptr[1] = (100*a0 + 502*a3 + 32768) >> 16;
	outcptr[3] = (426*a2 - 284*a1 + 32768) >> 16;
	outcptr[5] = (426*a1 + 284*a2 + 32768) >> 16;
	outcptr[7] = (100*a3 - 502*a0 + 32768) >> 16;

	dataptr += 8;
	outcptr += 8;
    }

    dataptr = block;
    if (grabtype == VIDIMAGE_YUYV)
	imgp -= 15;
    else
	imgp -= 17;
    for (i=0; i<8; i++) {
	t0 = imgp[0];
	t1 = imgp[7*width];
	a0 = t0+t1;
	c3 = t0-t1;
	imgp += width;

	t0 = imgp[0];
	t1 = imgp[5*width];
	a1 = t0+t1;
	c2 = t0-t1;
	imgp += width;

	t0 = imgp[0];
	t1 = imgp[3*width];
	a2 = t0+t1;
	c1 = t0-t1;
	imgp += width;

	t0 = imgp[0];
	t1 = imgp[width];
	a3 = t0+t1;
	c0 = t0-t1;
	imgp -= 3*width;

	b0 = a0+a3;
	b1 = a1+a2;
	b2 = a1-a2;
	b3 = a0-a3;

	dataptr[0]  = (362 * (b0+b1-1024)) >> 7; /* Correct for DC offset */
	dataptr[32] = (362 * (b0-b1))      >> 7;
	dataptr[16] = (196*b2 + 473*b3)    >> 7;
	dataptr[48] = (196*b3 - 473*b2)    >> 7;

	b0 = (362 * (c2-c1)) >> 7;
	b1 = (362 * (c2+c1)) >> 7;
	c0 = c0 << 2;
	c3 = c3 << 2;

	a0 = c0+b0;
	a1 = c0-b0;
	a2 = c3-b1;
	a3 = c3+b1;

	dataptr[8]  = (100*a0 + 502*a3) >> 9;
	dataptr[24] = (426*a2 - 284*a1) >> 9;
	dataptr[40] = (426*a1 + 284*a2) >> 9;
	dataptr[56] = (100*a3 - 502*a0) >> 9;


	imgp += 2;
	dataptr++;
    }

    dataptr = block;
    for (i=0; i<8; i++) {
	a0 = dataptr[0];
	a3 = dataptr[6];

	a1 = dataptr[2];
	a2 = dataptr[4];

	b0 = a0+a3;
	b1 = a1+a2;
	b2 = a1-a2;
	b3 = a0-a3;

	outcptr[0] = (362 * (b0+b1)   + 16384) >> 15;
	outcptr[4] = (362 * (b0-b1)   + 16384) >> 15;
	outcptr[2] = (196*b2 + 473*b3 + 16384) >> 15;
	outcptr[6] = (196*b3 - 473*b2 + 16384) >> 15;

	a0 = dataptr[1];
	a3 = dataptr[7];

	a1 = dataptr[3];
	a2 = dataptr[5];

	b0 = a0+a3;
	b1 = a1+a2;
	b2 = a1-a2;
	b3 = a0-a3;

	outcptr[1] = (362 * (b0+b1)   + 16384) >> 15;
	outcptr[5] = (362 * (b0-b1)   + 16384) >> 15;
	outcptr[3] = (196*b2 + 473*b3 + 16384) >> 15;
	outcptr[7] = (196*b3 - 473*b2 + 16384) >> 15;

	dataptr += 8;
	outcptr += 8;
    }
}

void NVDCT_RevTransform(uint32 *inp, uint8 *yp, uint8 *uvp, int width)
{
    int i, t, *dataptr;
    int a0, a1, a2, a3, b0, b1, b2, b3, c0, c1, c2, c3;
    int8 *inpcptr=(int8 *)inp;
    static int block[64];

    dataptr = block;
    for (i=0; i<8; i++) {
	if ((inp[0]|inp[1]) == 0) {
	    dataptr[0] = dataptr[1] = dataptr[2] = dataptr[3] =
		dataptr[4] = dataptr[5] = dataptr[6] = dataptr[7] = 0;
	} else {
	    b0 = inpcptr[0] << 4;
	    b1 = inpcptr[4] << 4;
	    b2 = inpcptr[2] << 4;
	    b3 = inpcptr[6] << 4;

	    a0 = (362 * (b0+b1)) >> 9;
	    a1 = (362 * (b0-b1)) >> 9;
	    a2 = (196*b2 - 473*b3) >> 9;
	    a3 = (473*b2 + 196*b3) >> 9;

	    b0 = a0+a3;
	    b1 = a1+a2;
	    b2 = a1-a2;
	    b3 = a0-a3;

	    a0 = inpcptr[1] << 4;
	    a1 = inpcptr[3] << 4;
	    a2 = inpcptr[5] << 4;
	    a3 = inpcptr[7] << 4;

	    c0 = (100*a0 - 502*a3) >> 9;
	    c1 = (426*a2 - 284*a1) >> 9;
	    c2 = (426*a1 + 284*a2) >> 9;
	    c3 = (502*a0 + 100*a3) >> 9;

	    a0 = c0+c1;
	    a1 = c0-c1;
	    a2 = c3-c2;
	    a3 = c3+c2;

	    c0 = a0;
	    c1 = (362 * (a2-a1)) >> 9;
	    c2 = (362 * (a1+a2)) >> 9;
	    c3 = a3;

	    dataptr[0] = b0+c3;
	    dataptr[1] = b1+c2;
	    dataptr[2] = b2+c1;
	    dataptr[3] = b3+c0;
	    dataptr[4] = b3-c0;
	    dataptr[5] = b2-c1;
	    dataptr[6] = b1-c2;
	    dataptr[7] = b0-c3;
	}

	inp += 2;
	inpcptr += 8;
	dataptr += 8;
    }

    dataptr = block;
    for (i=0; i<8; i++) {
	b0 = dataptr[0]+1448; /* Add back DC offset */
	b1 = dataptr[32];
	b2 = dataptr[16];
	b3 = dataptr[48];

	a0 = (362 * (b0+b1)) >> 9;
	a1 = (362 * (b0-b1)) >> 9;
	a2 = (196*b2 - 473*b3) >> 9;
	a3 = (473*b2 + 196*b3) >> 9;

	b0 = a0+a3;
	b1 = a1+a2;
	b2 = a1-a2;
	b3 = a0-a3;

	a0 = dataptr[8];
	a1 = dataptr[24];
	a2 = dataptr[40];
	a3 = dataptr[56];

	c0 = (100*a0 - 502*a3) >> 9;
	c1 = (426*a2 - 284*a1) >> 9;
	c2 = (426*a1 + 284*a2) >> 9;
	c3 = (502*a0 + 100*a3) >> 9;

	a0 = c0+c1;
	a1 = c0-c1;
	a2 = c3-c2;
	a3 = c3+c2;

	c0 = a0;
	c1 = (362 * (a2-a1)) >> 9;
	c2 = (362 * (a2+a1)) >> 9;
	c3 = a3;

	yp[0]     = UCLIMIT((b0+c3+4) >> 3);
	yp[width] = UCLIMIT((b1+c2+4) >> 3);
	yp += 2*width;

	yp[0]     = UCLIMIT((b2+c1+4) >> 3);
	yp[width] = UCLIMIT((b3+c0+4) >> 3);
	yp += 2*width;

	yp[0]     = UCLIMIT((b3-c0+4) >> 3);
	yp[width] = UCLIMIT((b2-c1+4) >> 3);
	yp += 2*width;

	yp[0]     = UCLIMIT((b1-c2+4) >> 3);
	yp[width] = UCLIMIT((b0-c3+4) >> 3);
	yp -= 6*width;

	yp++;
	dataptr++;
    }

    if (uvp) {
	dataptr = block;
	for (i=0; i<8; i++) {
	    if ((inp[0]|inp[1]) == 0) {
		dataptr[0] = dataptr[1] = dataptr[2] = dataptr[3] =
		    dataptr[4] = dataptr[5] = dataptr[6] = dataptr[7] = 0;
	    } else {
		b0 = inpcptr[0] << 4;
		b2 = inpcptr[2] << 4;
		b1 = inpcptr[4] << 4;
		b3 = inpcptr[6] << 4;

		a0 = (362 * (b0+b1)) >> 9;
		a1 = (362 * (b0-b1)) >> 9;
		a2 = (196*b2 - 473*b3) >> 9;
		a3 = (473*b2 + 196*b3) >> 9;

		dataptr[0] = a0+a3;
		dataptr[2] = a1+a2;
		dataptr[4] = a1-a2;
		dataptr[6] = a0-a3;

		b0 = inpcptr[1] << 4;
		b2 = inpcptr[3] << 4;
		b1 = inpcptr[5] << 4;
		b3 = inpcptr[7] << 4;

		a0 = (362 * (b0+b1)) >> 9;
		a1 = (362 * (b0-b1)) >> 9;
		a2 = (196*b2 - 473*b3) >> 9;
		a3 = (473*b2 + 196*b3) >> 9;

		dataptr[1] = a0+a3;
		dataptr[3] = a1+a2;
		dataptr[5] = a1-a2;
		dataptr[7] = a0-a3;
	    }

	    inp += 2;
	    inpcptr += 8;
	    dataptr += 8;
	}

	dataptr = block;
	for (i=0; i<8; i++) {
	    b0 = dataptr[0]+1448; /* Add back DC offset */
	    b1 = dataptr[32];
	    b2 = dataptr[16];
	    b3 = dataptr[48];

	    a0 = (362 * (b0+b1)) >> 9;
	    a1 = (362 * (b0-b1)) >> 9;
	    a2 = (196*b2 - 473*b3) >> 9;
	    a3 = (473*b2 + 196*b3) >> 9;

	    b0 = a0+a3;
	    b1 = a1+a2;
	    b2 = a1-a2;
	    b3 = a0-a3;

	    a0 = dataptr[8];
	    a1 = dataptr[24];
	    a2 = dataptr[40];
	    a3 = dataptr[56];

	    c0 = (100*a0 - 502*a3) >> 9;
	    c1 = (426*a2 - 284*a1) >> 9;
	    c2 = (426*a1 + 284*a2) >> 9;
	    c3 = (502*a0 + 100*a3) >> 9;

	    a0 = c0+c1;
	    a1 = c0-c1;
	    a2 = c3-c2;
	    a3 = c3+c2;

	    c0 = a0;
	    c1 = (362 * (a2-a1)) >> 9;
	    c2 = (362 * (a2+a1)) >> 9;
	    c3 = a3;

	    uvp[0]     = UCLIMIT((b0+c3+4) >> 3);
	    uvp[width] = UCLIMIT((b1+c2+4) >> 3);
	    uvp += 2*width;

	    uvp[0]     = UCLIMIT((b2+c1+4) >> 3);
	    uvp[width] = UCLIMIT((b3+c0+4) >> 3);
	    uvp += 2*width;

	    uvp[0]     = UCLIMIT((b3-c0+4) >> 3);
	    uvp[width] = UCLIMIT((b2-c1+4) >> 3);
	    uvp += 2*width;

	    uvp[0]     = UCLIMIT((b1-c2+4) >> 3);
	    uvp[width] = UCLIMIT((b0-c3+4) >> 3);
	    uvp -= 6*width;

	    uvp++;
	    dataptr++;
	}
    }
}
