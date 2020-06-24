/*
        Netvideo version 3.2
        Written by Ron Frederick <frederick@parc.xerox.com>
 
        Video TK widget greyscale rectangle update routines
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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <default.h>
#include <tk.h>
#include "video.h"
#include "vidwidget.h"

extern u_long black_pix, white_pix, grey_lut[256];

static u_char halftone4[16] =
    { 112,  48, 240, 176,
      192,  64, 128,  32,
      144, 224,  16,  80,
        0, 160,  96, 208 };
 
static u_char halftone8[64] =
    {   4, 236,  60, 220,   8, 224,  48, 208,
      132,  68, 188, 124, 136,  72, 176, 112,
       36, 196,  20, 252,  40, 200,  24, 240,
      164, 100, 148,  84, 168, 104, 152,  88,
       12, 228,  52, 212,   0, 232,  56, 216,
      140,  76, 180, 116, 128,  64, 184, 120,
       44, 204,  28, 244,  32, 192,  16, 248,
      172, 108, 156,  92, 160,  96, 144,  80 };
 
#define HALFTONE(c, h) (((c) > (h)) ? white_pix : black_pix)

static u_char grey_dither[16] =
    { 6, 2, 7, 3,
      1, 5, 0, 4,
      7, 3, 6, 2,
      0, 4, 1, 5 };

#define GREYDITHER(l, c, d) ((l)[((c) + (d)) >> (8-GREY_BITS)])

void VidGrey_MSB1bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line;
    register u_char *yp, *xip, row, *h4=halftone4, *h8=halftone8;
    register u_char *greymap=vidPtr->image->greymap;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = (u_char *) &vidPtr->ximage->data[y*2*ximagewidth+x/4];
	z = (y % 4) * 16;
	while (height--) {
	    for (w=width; w > 0; w -= 4) {
		row = HALFTONE(greymap[*yp], h8[z]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h8[z+1]);
		row = (row<<1) + HALFTONE(greymap[*yp], h8[z+2]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h8[z+3]);
		row = (row<<1) + HALFTONE(greymap[*yp], h8[z+4]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h8[z+5]);
		row = (row<<1) + HALFTONE(greymap[*yp], h8[z+6]);
		*xip++ = (row<<1) + HALFTONE(greymap[*yp++], h8[z+7]);
	    }
	    yp -= width;
	    xip += ximagewidth-width/4;
	    z += 8;

	    for (w=width; w > 0; w -= 4) {
		row = HALFTONE(greymap[*yp], h8[z]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h8[z+1]);
		row = (row<<1) + HALFTONE(greymap[*yp], h8[z+2]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h8[z+3]);
		row = (row<<1) + HALFTONE(greymap[*yp], h8[z+4]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h8[z+5]);
		row = (row<<1) + HALFTONE(greymap[*yp], h8[z+6]);
		*xip++ = (row<<1) + HALFTONE(greymap[*yp++], h8[z+7]);
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth-width/4;
	    z = (z + 8) % 64;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = (u_char *) &vidPtr->ximage->data[(y >> scalebits)*ximagewidth +
					       (x >> (scalebits+3))];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		row = HALFTONE(greymap[*yp], h4[z]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h4[z+1]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h4[z+2]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h4[z+3]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h4[z]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h4[z+1]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h4[z+2]);
		yp += scaler;
		*xip++ = (row<<1) + HALFTONE(greymap[*yp], h4[z+3]);
		yp += scaler;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> (scalebits+3));
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}

void VidGrey_LSB1bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line;
    register u_char *yp, *xip, row, *h4=halftone4, *h8=halftone8;
    register u_char *greymap=vidPtr->image->greymap;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = (u_char *) &vidPtr->ximage->data[y*2*ximagewidth+x/4];
	z = (y % 4) * 16;
	while (height--) {
	    for (w=width; w > 0; w -= 4) {
		row = HALFTONE(greymap[*yp], h8[z]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h8[z+1]);
		row = (row>>1) + HALFTONE(greymap[*yp], h8[z+2]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h8[z+3]);
		row = (row>>1) + HALFTONE(greymap[*yp], h8[z+4]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h8[z+5]);
		row = (row>>1) + HALFTONE(greymap[*yp], h8[z+6]);
		*xip++ = (row>>1) + HALFTONE(greymap[*yp++], h8[z+7]);
	    }
	    yp -= width;
	    xip += ximagewidth-width/4;
	    z += 8;

	    for (w=width; w > 0; w -= 4) {
		row = HALFTONE(greymap[*yp], h8[z]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h8[z+1]);
		row = (row>>1) + HALFTONE(greymap[*yp], h8[z+2]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h8[z+3]);
		row = (row>>1) + HALFTONE(greymap[*yp], h8[z+4]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h8[z+5]);
		row = (row>>1) + HALFTONE(greymap[*yp], h8[z+6]);
		*xip++ = (row>>1) + HALFTONE(greymap[*yp++], h8[z+7]);
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth-width/4;
	    z = (z + 8) % 64;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = (u_char *) &vidPtr->ximage->data[(y >> scalebits)*ximagewidth +
					       (x >> (scalebits+3))];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		row = HALFTONE(greymap[*yp], h4[z]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h4[z+1]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h4[z+2]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h4[z+3]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h4[z]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h4[z+1]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h4[z+2]);
		yp += scaler;
		*xip++ = (row>>1) + HALFTONE(greymap[*yp], h4[z+3]);
		yp += scaler;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> (scalebits+3));
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}

void VidGrey_MSB8bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line/4;
    register u_char y0, y1, y2, *yp;
    register u_char *gd=grey_dither, *greymap=vidPtr->image->greymap;
    register u_long *lut=grey_lut, *xip, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[(y*2+1)*ximagewidth + x/2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		out = GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		out = (out << 8) + GREYDITHER(lut, greymap[y0], gd[z+1]);
		y1 = yp[1];
		out = (out << 8) + GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		xip[0] = (out << 8) + GREYDITHER(lut, greymap[y1], gd[z+3]);
		y0 = yp[2];
		out = GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		out = (out << 8) + GREYDITHER(lut, greymap[y0], gd[z+1]);
		y1 = yp[3];
		out = (out << 8) + GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		xip[1] = (out << 8) + GREYDITHER(lut, greymap[y1], gd[z+3]);
		yp += 4;
		xip += 2;
	    }
	    yp -= width;
	    xip -= (ximagewidth+width/2);
	    z += 4;

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    out =
			GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		    out = (out << 8) +
			GREYDITHER(lut, greymap[y0], gd[z+1]);
		    y1 = yp[1];
		    out = (out << 8) +
			GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		    xip[0] = (out << 8) +
			GREYDITHER(lut, greymap[y1], gd[z+3]);
		    y0 = yp[2];
		    out =
			GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		    out = (out << 8) +
			GREYDITHER(lut, greymap[y0], gd[z+1]);
		    y1 = yp[3];
		    out = (out << 8) +
			GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		    xip[1] = (out << 8) +
			GREYDITHER(lut, greymap[y1], gd[z+3]);
		    yp += 4;
		    xip += 2;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    out =
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z]);
		    out = (out << 8) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+1]);
		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    out = (out << 8) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+2]);
		    xip[0] = (out << 8) +
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z+3]);
		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    out =
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z]);
		    out = (out << 8) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+1]);
		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    out = (out << 8) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+2]);
		    xip[1] = (out << 8) +
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z+3]);
		    yp += 4;
		    xip += 2;
		}
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth*3-width/2;
	    z = (z + 4) % 16;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+2))];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z]);
		out = (out << 8) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+1]);
		yp += 2*scaler;
		out = (out << 8) +
		    GREYDITHER(lut, greymap[yp[0]], gd[z+2]);
		xip[0] = (out << 8) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+3]);
		yp += 2*scaler;
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z]);
		out = (out << 8) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+1]);
		yp += 2*scaler;
		out = (out << 8) +
		    GREYDITHER(lut, greymap[yp[0]], gd[z+2]);
		xip[1] = (out << 8) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+3]);
		yp += 2*scaler;
		xip += 2;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> (scalebits+2));
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}

void VidGrey_LSB8bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line/4;
    register u_char y0, y1, y2, *yp;
    register u_char *gd=grey_dither, *greymap=vidPtr->image->greymap;
    register u_long *lut=grey_lut, *xip, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[(y*2+1)*ximagewidth + x/2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		out = GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		out = (out >> 8) + GREYDITHER(lut, greymap[y0], gd[z+1]);
		y1 = yp[1];
		out = (out >> 8) + GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		xip[0] = (out >> 8) + GREYDITHER(lut, greymap[y1], gd[z+3]);
		y0 = yp[2];
		out = GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		out = (out >> 8) + GREYDITHER(lut, greymap[y0], gd[z+1]);
		y1 = yp[3];
		out = (out >> 8) + GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		xip[1] = (out >> 8) + GREYDITHER(lut, greymap[y1], gd[z+3]);
		yp += 4;
		xip += 2;
	    }
	    yp -= width;
	    xip -= (ximagewidth+width/2);
	    z += 4;

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    out =
			GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		    out = (out >> 8) +
			GREYDITHER(lut, greymap[y0], gd[z+1]);
		    y1 = yp[1];
		    out = (out >> 8) +
			GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		    xip[0] = (out >> 8) +
			GREYDITHER(lut, greymap[y1], gd[z+3]);
		    y0 = yp[2];
		    out =
			GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		    out = (out >> 8) +
			GREYDITHER(lut, greymap[y0], gd[z+1]);
		    y1 = yp[3];
		    out = (out >> 8) +
			GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		    xip[1] = (out >> 8) +
			GREYDITHER(lut, greymap[y1], gd[z+3]);
		    yp += 4;
		    xip += 2;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    out =
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z]);
		    out = (out >> 8) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+1]);
		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    out = (out >> 8) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+2]);
		    xip[0] = (out >> 8) +
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z+3]);
		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    out =
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z]);
		    out = (out >> 8) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+1]);
		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    out = (out >> 8) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+2]);
		    xip[1] = (out >> 8) +
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z+3]);
		    yp += 4;
		    xip += 2;
		}
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth*3-width/2;
	    z = (z + 4) % 16;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+2))];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z]);
		out = (out >> 8) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+1]);
		yp += 2*scaler;
		out = (out >> 8) +
		    GREYDITHER(lut, greymap[yp[0]], gd[z+2]);
		xip[0] = (out >> 8) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+3]);
		yp += 2*scaler;
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z]);
		out = (out >> 8) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+1]);
		yp += 2*scaler;
		out = (out >> 8) +
		    GREYDITHER(lut, greymap[yp[0]], gd[z+2]);
		xip[1] = (out >> 8) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+3]);
		yp += 2*scaler;
		xip += 2;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> (scalebits+2));
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}

void VidGrey_MSB16bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line/4;
    register u_char y0, y1, y2, *yp;
    register u_char *gd=grey_dither, *greymap=vidPtr->image->greymap;
    register u_long *lut=grey_lut, *xip, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[(y*2+1)*ximagewidth + x];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		out = GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		xip[0] = (out << 16) + GREYDITHER(lut, greymap[y0], gd[z+1]);
		y1 = yp[1];
		out = GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		xip[1] = (out << 16) + GREYDITHER(lut, greymap[y1], gd[z+3]);
		y0 = yp[2];
		out = GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		xip[2] = (out << 16) + GREYDITHER(lut, greymap[y0], gd[z+1]);
		y1 = yp[3];
		out = GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		xip[1] = (out << 16) + GREYDITHER(lut, greymap[y1], gd[z+3]);
		yp += 4;
		xip += 4;
	    }
	    yp -= width;
	    xip -= (ximagewidth+width);
	    z += 4;

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    out =
			GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		    xip[0] = (out << 16) +
			GREYDITHER(lut, greymap[y0], gd[z+1]);
		    y1 = yp[1];
		    out =
			GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		    xip[1] = (out << 16) +
			GREYDITHER(lut, greymap[y1], gd[z+3]);
		    y0 = yp[2];
		    out =
			GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		    xip[2] = (out << 16) +
			GREYDITHER(lut, greymap[y0], gd[z+1]);
		    y1 = yp[3];
		    out =
			GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		    xip[3] = (out << 16) +
			GREYDITHER(lut, greymap[y1], gd[z+3]);
		    yp += 4;
		    xip += 4;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    out =
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z]);
		    xip[0] = (out << 16) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+1]);
		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    out =
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+2]);
		    xip[1] = (out << 16) +
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z+3]);
		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    out =
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z]);
		    xip[2] = (out << 16) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+1]);
		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    out =
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+2]);
		    xip[3] = (out << 16) +
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z+3]);
		    yp += 4;
		    xip += 4;
		}
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth*3-width;
	    z = (z + 4) % 16;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+1))];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z]);
		xip[0] = (out << 16) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+1]);
		yp += 2*scaler;
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z+2]);
		xip[1] = (out << 16) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+3]);
		yp += 2*scaler;
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z]);
		xip[2] = (out << 16) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+1]);
		yp += 2*scaler;
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z+2]);
		xip[3] = (out << 16) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+3]);
		yp += 2*scaler;
		xip += 4;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> (scalebits+1));
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}

void VidGrey_LSB16bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line/4;
    register u_char y0, y1, y2, *yp;
    register u_char *gd=grey_dither, *greymap=vidPtr->image->greymap;
    register u_long *lut=grey_lut, *xip, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[(y*2+1)*ximagewidth + x];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		out = GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		xip[0] = (out >> 16) + GREYDITHER(lut, greymap[y0], gd[z+1]);
		y1 = yp[1];
		out = GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		xip[1] = (out >> 16) + GREYDITHER(lut, greymap[y1], gd[z+3]);
		y0 = yp[2];
		out = GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		xip[2] = (out >> 16) + GREYDITHER(lut, greymap[y0], gd[z+1]);
		y1 = yp[3];
		out = GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		xip[1] = (out >> 16) + GREYDITHER(lut, greymap[y1], gd[z+3]);
		yp += 4;
		xip += 4;
	    }
	    yp -= width;
	    xip -= (ximagewidth+width);
	    z += 4;

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    out =
			GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		    xip[0] = (out >> 16) +
			GREYDITHER(lut, greymap[y0], gd[z+1]);
		    y1 = yp[1];
		    out =
			GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		    xip[1] = (out >> 16) +
			GREYDITHER(lut, greymap[y1], gd[z+3]);
		    y0 = yp[2];
		    out =
			GREYDITHER(lut, greymap[(y1+y0)/2], gd[z]);
		    xip[2] = (out >> 16) +
			GREYDITHER(lut, greymap[y0], gd[z+1]);
		    y1 = yp[3];
		    out =
			GREYDITHER(lut, greymap[(y0+y1)/2], gd[z+2]);
		    xip[3] = (out >> 16) +
			GREYDITHER(lut, greymap[y1], gd[z+3]);
		    yp += 4;
		    xip += 4;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    out =
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z]);
		    xip[0] = (out >> 16) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+1]);
		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    out =
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+2]);
		    xip[1] = (out >> 16) +
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z+3]);
		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    out =
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z]);
		    xip[2] = (out >> 16) +
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+1]);
		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    out =
			GREYDITHER(lut, greymap[(y1+y2)/2], gd[z+2]);
		    xip[3] = (out >> 16) +
			GREYDITHER(lut, greymap[(y0+y2)/2], gd[z+3]);
		    yp += 4;
		    xip += 4;
		}
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth*3-width;
	    z = (z + 4) % 16;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+1))];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z]);
		xip[0] = (out >> 16) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+1]);
		yp += 2*scaler;
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z+2]);
		xip[1] = (out >> 16) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+3]);
		yp += 2*scaler;
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z]);
		xip[2] = (out >> 16) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+1]);
		yp += 2*scaler;
		out =
		    GREYDITHER(lut, greymap[yp[0]], gd[z+2]);
		xip[3] = (out >> 16) +
		    GREYDITHER(lut, greymap[yp[scaler]], gd[z+3]);
		yp += 2*scaler;
		xip += 4;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> (scalebits+1));
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}

void VidGrey_24bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line/4;
    register u_char y0, y1, y2, *yp;
    register u_char *greymap=vidPtr->image->greymap;
    register u_long *lut=grey_lut, *xip;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[(y*2+1)*ximagewidth + x*2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		xip[0] = lut[greymap[(y1+y0)/2]];
		xip[1] = lut[greymap[y0]];
		y1 = yp[1];
		xip[2] = lut[greymap[(y0+y1)/2]];
		xip[3] = lut[greymap[y1]];
		y0 = yp[2];
		xip[4] = lut[greymap[(y1+y0)/2]];
		xip[5] = lut[greymap[y0]];
		y1 = yp[3];
		xip[6] = lut[greymap[(y0+y1)/2]];
		xip[7] = lut[greymap[y1]];
		yp += 4;
		xip += 8;
	    }
	    yp -= width;
	    xip -= (ximagewidth+width*2);
	    z += 4;

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    xip[0] = lut[greymap[(y1+y0)/2]];
		    xip[1] = lut[greymap[y0]];
		    y1 = yp[1];
		    xip[2] = lut[greymap[(y0+y1)/2]];
		    xip[3] = lut[greymap[y1]];
		    y0 = yp[2];
		    xip[4] = lut[greymap[(y1+y0)/2]];
		    xip[5] = lut[greymap[y0]];
		    y1 = yp[3];
		    xip[6] = lut[greymap[(y0+y1)/2]];
		    xip[7] = lut[greymap[y1]];
		    yp += 4;
		    xip += 8;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    xip[0] = lut[greymap[(y0+y2)/2]];
		    xip[1] = lut[greymap[(y1+y2)/2]];
		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    xip[2] = lut[greymap[(y1+y2)/2]];
		    xip[3] = lut[greymap[(y0+y2)/2]];
		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    xip[4] = lut[greymap[(y0+y2)/2]];
		    xip[5] = lut[greymap[(y1+y2)/2]];
		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    xip[6] = lut[greymap[(y1+y2)/2]];
		    xip[7] = lut[greymap[(y0+y2)/2]];
		    yp += 4;
		    xip += 8;
		}
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth*3-width*2;
	    z = (z + 4) % 16;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)
		  [(y >> scalebits)*ximagewidth + (x >> scalebits)];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		xip[0] = lut[greymap[yp[0]]];
		xip[1] = lut[greymap[yp[scaler]]];
		yp += 2*scaler;
		xip[2] = lut[greymap[yp[0]]];
		xip[3] = lut[greymap[yp[scaler]]];
		yp += 2*scaler;
		xip[4] = lut[greymap[yp[0]]];
		xip[5] = lut[greymap[yp[scaler]]];
		yp += 2*scaler;
		xip[6] = lut[greymap[yp[0]]];
		xip[7] = lut[greymap[yp[scaler]]];
		yp += 2*scaler;
		xip += 8;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> scalebits);
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}
