/*
	Netvideo version 3.2
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video transform routines
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
#include "video.h"
#include "vidcode.h"

/* Sick little macro which will limit x to [0..255] with logical ops */
#define UCLIMIT(x) ((t = (x)), (t &= ~(t>>31)), (t | ~((t-256) >> 31)))

/* A variant of above which will limit x to [-128..127] */
#define SCLIMIT(x) (UCLIMIT((x)+128)-128)

void VidTransform_Fwd(u_char *yp, s_char *uvp, int width, u_long *out)
{
    register int i, t0, t1, t2, t3, t4, t5, *dataptr;
    register int width2=2*width, width6=6*width;
    register s_char *outcptr=(s_char *)out;
    static int block[64];

    dataptr = block;
    for (i=0; i<8; i++) {
	t0 = yp[0];
	t1 = yp[width];
	t2 = t0 + t1;
	dataptr[32] = (t1 - t0) << 2;
	yp += width2;

	t0 = yp[0];
	t1 = yp[width];
	t3 = t0 + t1;
	dataptr[40] = (t1 - t0) << 2;
	yp += width2;

	t0 = yp[0];
	t1 = yp[width];
	t4 = t0 + t1;
	dataptr[48] = (t1 - t0) << 2;
	yp += width2;

	t0 = yp[0];
	t1 = yp[width];
	t5 = t0 + t1;
	dataptr[56] = (t1 - t0) << 2;
	yp -= width6;

	t0 = t2 + t3;
	t1 = t4 + t5;
	dataptr[0]  = t0 + t1 - 1024; /* Correct for DC offset */
	dataptr[8]  = t1 - t0;
	dataptr[16] = (t3 - t2) << 1;
	dataptr[24] = (t5 - t4) << 1;

	yp++;
	dataptr++;
    }

    dataptr = block;
    for (i=0; i<8; i++) {
	t0 = dataptr[0];
	t1 = dataptr[1];
	t2 = t0 + t1;
	outcptr[4] = (t1 - t0 + 7) >> 4;

	t0 = dataptr[2];
	t1 = dataptr[3];
	t3 = t0 + t1;
	outcptr[5] = (t1 - t0 + 7) >> 4;

	t0 = dataptr[4];
	t1 = dataptr[5];
	t4 = t0 + t1;
	outcptr[6] = (t1 - t0 + 7) >> 4;

	t0 = dataptr[6];
	t1 = dataptr[7];
	t5 = t0 + t1;
	outcptr[7] = (t1 - t0 + 7) >> 4;

	t0 = t2 + t3;
	t1 = t4 + t5;
	outcptr[0] = (t0 + t1 + 31) >> 6;
	outcptr[1] = (t1 - t0 + 31) >> 6;
	outcptr[2] = (t3 - t2 + 15) >> 5;
	outcptr[3] = (t5 - t4 + 15) >> 5;

	dataptr += 8;
	outcptr += 8;
    }

    if (uvp) {
	dataptr = block;
	for (i=0; i<8; i++) {
	    t0 = uvp[0];
	    t1 = uvp[width];
	    t2 = t0 + t1;
	    dataptr[32] = (t1 - t0) << 2;
	    uvp += width2;

	    t0 = uvp[0];
	    t1 = uvp[width];
	    t3 = t0 + t1;
	    dataptr[40] = (t1 - t0) << 2;
	    uvp += width2;

	    t0 = uvp[0];
	    t1 = uvp[width];
	    t4 = t0 + t1;
	    dataptr[48] = (t1 - t0) << 2;
	    uvp += width2;

	    t0 = uvp[0];
	    t1 = uvp[width];
	    t5 = t0 + t1;
	    dataptr[56] = (t1 - t0) << 2;
	    uvp -= width6;

	    t0 = t2 + t3;
	    t1 = t4 + t5;
	    dataptr[0]  = t0 + t1;
	    dataptr[8]  = t1 - t0;
	    dataptr[16] = (t3 - t2) << 1;
	    dataptr[24] = (t5 - t4) << 1;

	    uvp++;
	    dataptr++;
	}

	dataptr = block;
	for (i=0; i<8; i++) {
	    t0 = dataptr[0];
	    t1 = dataptr[2];
	    t2 = t0 + t1;
	    outcptr[4] = (t1 - t0 + 7) >> 4;

	    t0 = dataptr[4];
	    t1 = dataptr[6];
	    t3 = t0 + t1;
	    outcptr[5] = (t1 - t0 + 7) >> 4;

	    t0 = dataptr[1];
	    t1 = dataptr[3];
	    t4 = t0 + t1;
	    outcptr[6] = (t1 - t0 + 7) >> 4;

	    t0 = dataptr[5];
	    t1 = dataptr[7];
	    t5 = t0 + t1;
	    outcptr[7] = (t1 - t0 + 7) >> 4;

	    outcptr[0] = (t2 + t3 + 15) >> 5;
	    outcptr[1] = (t4 + t5 + 15) >> 5;
	    outcptr[2] = (t3 - t2 + 15) >> 5;
	    outcptr[3] = (t5 - t4 + 15) >> 5;

	    dataptr += 8;
	    outcptr += 8;
	}
    }
}

void VidTransform_Rev(u_long *inp, u_char *yp, s_char *uvp, int width)
{
    register int i, t, t0, t1, t2, t3, t4, t5, *dataptr;
    register int width2=2*width, width6=6*width;
    register s_char *inpcptr=(s_char *)inp;
    static int block[64];

    dataptr = block;
    for (i=0; i<8; i++) {
	if ((inp[0] << 8) == 0) {
	    t2 = t3 = t4 = t5 = inpcptr[0];
	} else {
	    t4 = inpcptr[0];
	    t5 = inpcptr[1];
	    t0 = t4 - t5;
	    t1 = t4 + t5;

	    t4 = inpcptr[2];
	    t5 = inpcptr[3];
	    t2 = t0 - t4;
	    t3 = t0 + t4;
	    t4 = t1 - t5;
	    t5 = t1 + t5;
	}

	if (inp[1] == 0) {
	    dataptr[0] = dataptr[1] = t2;
	    dataptr[2] = dataptr[3] = t3;
	    dataptr[4] = dataptr[5] = t4;
	    dataptr[6] = dataptr[7] = t5;
	} else {
	    t0 = inpcptr[4];
	    t1 = inpcptr[5];
	    dataptr[0] = t2 - t0;
	    dataptr[1] = t2 + t0;
	    dataptr[2] = t3 - t1;
	    dataptr[3] = t3 + t1;

	    t0 = inpcptr[6];
	    t1 = inpcptr[7];
	    dataptr[4] = t4 - t0;
	    dataptr[5] = t4 + t0;
	    dataptr[6] = t5 - t1;
	    dataptr[7] = t5 + t1;
	}

	inp += 2;
	inpcptr += 8;
	dataptr += 8;
    }

    dataptr = block;
    for (i=0; i<8; i++) {
	t4 = dataptr[0] + 128; /* Add back DC offset */
	t5 = dataptr[8];
	t0 = t4 - t5;
	t1 = t4 + t5;

	t4 = dataptr[16];
	t5 = dataptr[24];
	t2 = t0 - t4;
	t3 = t0 + t4;
	t4 = t1 - t5;
	t5 = t1 + t5;

	t0 = dataptr[32];
	t1 = dataptr[40];
	yp[0] = UCLIMIT(t2 - t0);
	yp[width] = UCLIMIT(t2 + t0);
	yp += width2;
	yp[0] = UCLIMIT(t3 - t1);
	yp[width] = UCLIMIT(t3 + t1);
	yp += width2;

	t0 = dataptr[48];
	t1 = dataptr[56];
	yp[0] = UCLIMIT(t4 - t0);
	yp[width] = UCLIMIT(t4 + t0);
	yp += width2;
	yp[0] = UCLIMIT(t5 - t1);
	yp[width] = UCLIMIT(t5 + t1);
	yp -= width6;

	yp++;
	dataptr++;
    }

    if (uvp) {
	dataptr = block;
	for (i=0; i<8; i++) {
	    if ((inp[0] << 16) == 0) {
		t2 = t3 = inpcptr[0];
		t4 = t5 = inpcptr[1];
	    } else {
		t0 = inpcptr[0];
		t1 = inpcptr[1];

		t4 = inpcptr[2];
		t5 = inpcptr[3];
		t2 = t0 - t4;
		t3 = t0 + t4;
		t4 = t1 - t5;
		t5 = t1 + t5;
	    }

	    if (inp[1] == 0) {
		dataptr[0] = dataptr[2] = t2;
		dataptr[4] = dataptr[6] = t3;
		dataptr[1] = dataptr[3] = t4;
		dataptr[5] = dataptr[7] = t5;
	    } else {
		t0 = inpcptr[4];
		t1 = inpcptr[5];
		dataptr[0] = t2 - t0;
		dataptr[2] = t2 + t0;
		dataptr[4] = t3 - t1;
		dataptr[6] = t3 + t1;

		t0 = inpcptr[6];
		t1 = inpcptr[7];
		dataptr[1] = t4 - t0;
		dataptr[3] = t4 + t0;
		dataptr[5] = t5 - t1;
		dataptr[7] = t5 + t1;
	    }

	    inp += 2;
	    inpcptr += 8;
	    dataptr += 8;
	}

	dataptr = block;
	for (i=0; i<8; i++) {
	    t4 = dataptr[0];
	    t5 = dataptr[8];
	    t0 = t4 - t5;
	    t1 = t4 + t5;

	    t4 = dataptr[16];
	    t5 = dataptr[24];
	    t2 = t0 - t4;
	    t3 = t0 + t4;
	    t4 = t1 - t5;
	    t5 = t1 + t5;

	    t0 = dataptr[32];
	    t1 = dataptr[40];
	    uvp[0] = SCLIMIT(t2 - t0);
	    uvp[width] = SCLIMIT(t2 + t0);
	    uvp += width2;
	    uvp[0] = SCLIMIT(t3 - t1);
	    uvp[width] = SCLIMIT(t3 + t1);
	    uvp += width2;

	    t0 = dataptr[48];
	    t1 = dataptr[56];
	    uvp[0] = SCLIMIT(t4 - t0);
	    uvp[width] = SCLIMIT(t4 + t0);
	    uvp += width2;
	    uvp[0] = SCLIMIT(t5 - t1);
	    uvp[width] = SCLIMIT(t5 + t1);
	    uvp -= width6;

	    uvp++;
	    dataptr++;
	}
    }
}
