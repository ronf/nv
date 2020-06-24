/*
        Netvideo version 3.2
        Written by Ron Frederick <frederick@parc.xerox.com>
 
        Video TK widget color rectangle update routines
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

extern u_long yuv2rgb[65536], color_lut[32768];

static u_long color_dither[16] =
  { 0x180a00, 0x080010, 0x1c0804, 0x0c0214,
    0x060618, 0x160c08, 0x02041c, 0x120e0c,
    0x1e0906, 0x0e0316, 0x1a0b02, 0x0a0112,
    0x00051e, 0x100f0e, 0x04071a, 0x140d0a };

#define COLORDITHER(l, c, base, d) \
	{ c = (base) + (d); \
	  c = ((c>>9) & 0x7c00) + ((c>>6) & 0x3e0) + ((c>>3) & 0x1f); \
	  c = (l)[c]; }

void VidColor_MSB8bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line/4;
    register u_char y0, y1, y2, *yp;
    register u_short uv, *uvp;
    register u_char *greymap=vidPtr->image->greymap;
    register u_long *cd=color_dither, *lut=color_lut, *xip, c, base, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[(y*2+1)*ximagewidth + x/2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[y0] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out << 8) + c;

		y1 = yp[1];

		base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out << 8) + c;

		base = yuv2rgb[uv + (greymap[y1] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[0] = (out << 8) + c;

		y0 = yp[2];
		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[y0] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out << 8) + c;

		y1 = yp[3];

		base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out << 8) + c;

		base = yuv2rgb[uv + (greymap[y1] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out << 8) + c;

		yp += 4;
		uvp += 2;
		xip += 2;
	    }

	    yp -= width;
	    uvp -= width/2;
	    xip -= (ximagewidth+width/2);
	    z += 4;

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    uv = uvp[0];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[y0] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    out = (out << 8) + c;

		    y1 = yp[1];

		    base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+2]);
		    out = (out << 8) + c;

		    base = yuv2rgb[uv + (greymap[y1] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[0] = (out << 8) + c;

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[y0] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    out = (out << 8) + c;

		    y1 = yp[3];

		    base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+2]);
		    out = (out << 8) + c;

		    base = yuv2rgb[uv + (greymap[y1] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[1] = (out << 8) + c;

		    yp += 4;
		    uvp += 2;
		    xip += 2;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    uv = uvp[0];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    out = (out << 8) + c;

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+2]);
		    out = (out << 8) + c;

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[0] = (out << 8) + c;

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    out = (out << 8) + c;

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+2]);
		    out = (out << 8) + c;

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[1] = (out << 8) + c;

		    yp += 4;
		    uvp += 2;
		    xip += 2;
		}
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth*3-width/2;
	    z = (z + 4) % 16;
	}
    } else if (scalebits == 0) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[y*ximagewidth + x/4];
	z = (y % 4) * 4;
	while (height--) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[yp[1]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out << 8) + c;

		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[2]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out << 8) + c;

		base = yuv2rgb[uv + (greymap[yp[3]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[0] = (out << 8) + c;

		uv = uvp[2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[4]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[yp[5]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out << 8) + c;

		uv = uvp[3];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[6]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out << 8) + c;

		base = yuv2rgb[uv + (greymap[yp[7]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out << 8) + c;

		yp += 8;
		uvp += 4;
		xip += 2;
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth-width/4;
	    z = (z + 4) % 16;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+2))];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out << 8) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out << 8) + c;

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[0] = (out << 8) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out << 8) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out << 8) + c;

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out << 8) + c;

		yp += 2*scaler;
		uvp += scaler;
		xip += 2;
	    }
	    yp += (imagewidth << scalebits)-width;
	    uvp += ((imagewidth << scalebits)-width)/2;
	    xip += ximagewidth-(width >> (scalebits+2));
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}

void VidColor_LSB8bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line/4;
    register u_char y0, y1, y2, *yp;
    register u_short uv, *uvp;
    register u_char *greymap=vidPtr->image->greymap;
    register u_long *cd=color_dither, *lut=color_lut, *xip, c, base, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[(y*2+1)*ximagewidth + x/2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[y0] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out >> 8) + c;

		y1 = yp[1];

		base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out >> 8) + c;

		base = yuv2rgb[uv + (greymap[y1] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[0] = (out >> 8) + c;

		y0 = yp[2];
		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[y0] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out >> 8) + c;

		y1 = yp[3];

		base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out >> 8) + c;

		base = yuv2rgb[uv + (greymap[y1] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out >> 8) + c;

		yp += 4;
		uvp += 2;
		xip += 2;
	    }

	    yp -= width;
	    uvp -= width/2;
	    xip -= (ximagewidth+width/2);
	    z += 4;

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    uv = uvp[0];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[y0] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    out = (out >> 8) + c;

		    y1 = yp[1];

		    base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+2]);
		    out = (out >> 8) + c;

		    base = yuv2rgb[uv + (greymap[y1] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[0] = (out >> 8) + c;

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[y0] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    out = (out >> 8) + c;

		    y1 = yp[3];

		    base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+2]);
		    out = (out >> 8) + c;

		    base = yuv2rgb[uv + (greymap[y1] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[1] = (out >> 8) + c;

		    yp += 4;
		    uvp += 2;
		    xip += 2;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    uv = uvp[0];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    out = (out >> 8) + c;

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+2]);
		    out = (out >> 8) + c;

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[0] = (out >> 8) + c;

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    out = (out >> 8) + c;

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+2]);
		    out = (out >> 8) + c;

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[1] = (out >> 8) + c;

		    yp += 4;
		    uvp += 2;
		    xip += 2;
		}
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth*3-width/2;
	    z = (z + 4) % 16;
	}
    } else if (scalebits == 0) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[y*ximagewidth + x/4];
	z = (y % 4) * 4;
	while (height--) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[yp[1]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out >> 8) + c;

		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[2]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out >> 8) + c;

		base = yuv2rgb[uv + (greymap[yp[3]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[0] = (out >> 8) + c;

		uv = uvp[2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[4]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[yp[5]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out >> 8) + c;

		uv = uvp[3];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[6]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out >> 8) + c;

		base = yuv2rgb[uv + (greymap[yp[7]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out >> 8) + c;

		yp += 8;
		uvp += 4;
		xip += 2;
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth-width/4;
	    z = (z + 4) % 16;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+2))];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out >> 8) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out >> 8) + c;

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[0] = (out >> 8) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		out = (out >> 8) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+2]);
		out = (out >> 8) + c;

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out >> 8) + c;

		yp += 2*scaler;
		uvp += scaler;
		xip += 2;
	    }
	    yp += (imagewidth << scalebits)-width;
	    uvp += ((imagewidth << scalebits)-width)/2;
	    xip += ximagewidth-(width >> (scalebits+2));
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}

void VidColor_MSB16bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line/4;
    register u_char y0, y1, y2, *yp;
    register u_short uv, *uvp;
    register u_char *greymap=vidPtr->image->greymap;
    register u_long *cd=color_dither, *lut=color_lut, *xip, c, base, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[(y*2+1)*ximagewidth + x];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[y0] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[0] = (out << 16) + c;

		y1 = yp[1];

		base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		base = yuv2rgb[uv + (greymap[y1] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out << 16) + c;

		y0 = yp[2];
		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[y0] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[2] = (out << 16) + c;

		y1 = yp[3];

		base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		base = yuv2rgb[uv + (greymap[y1] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[3] = (out << 16) + c;

		yp += 4;
		uvp += 2;
		xip += 4;
	    }

	    yp -= width;
	    uvp -= width/2;
	    xip -= (ximagewidth+width);
	    z += 4;

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    uv = uvp[0];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[y0] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    xip[0] = (out << 16) + c;

		    y1 = yp[1];

		    base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z+2]);

		    base = yuv2rgb[uv + (greymap[y1] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[1] = (out << 16) + c;

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[y0] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    xip[2] = (out << 16) + c;

		    y1 = yp[3];

		    base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z+2]);

		    base = yuv2rgb[uv + (greymap[y1] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[3] = (out << 16) + c;

		    yp += 4;
		    uvp += 2;
		    xip += 4;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    uv = uvp[0];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    xip[0] = (out << 16) + c;

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z+2]);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[1] = (out << 16) + c;

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    xip[2] = (out << 16) + c;

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z+2]);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[3] = (out << 16) + c;

		    yp += 4;
		    uvp += 2;
		    xip += 4;
		}
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth*3-width;
	    z = (z + 4) % 16;
	}
    } else if (scalebits == 0) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[y*ximagewidth + x/2];
	z = (y % 4) * 4;
	while (height--) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[yp[1]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[0] = (out << 16) + c;

		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[2]] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		base = yuv2rgb[uv + (greymap[yp[3]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out << 16) + c;

		uv = uvp[2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[4]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[yp[5]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[2] = (out << 16) + c;

		uv = uvp[3];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[6]] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		base = yuv2rgb[uv + (greymap[yp[7]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[3] = (out << 16) + c;

		yp += 8;
		uvp += 4;
		xip += 4;
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth-width/2;
	    z = (z + 4) % 16;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+1))];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[0] = (out << 16) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out << 16) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[2] = (out << 16) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[3] = (out << 16) + c;

		yp += 2*scaler;
		uvp += scaler;
		xip += 4;
	    }
	    yp += (imagewidth << scalebits)-width;
	    uvp += ((imagewidth << scalebits)-width)/2;
	    xip += ximagewidth-(width >> (scalebits+1));
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}

void VidColor_LSB16bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line/4;
    register u_char y0, y1, y2, *yp;
    register u_short uv, *uvp;
    register u_char *greymap=vidPtr->image->greymap;
    register u_long *cd=color_dither, *lut=color_lut, *xip, c, base, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[(y*2+1)*ximagewidth + x];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[y0] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[0] = (out >> 16) + c;

		y1 = yp[1];

		base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		base = yuv2rgb[uv + (greymap[y1] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out >> 16) + c;

		y0 = yp[2];
		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[y0] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[2] = (out >> 16) + c;

		y1 = yp[3];

		base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		base = yuv2rgb[uv + (greymap[y1] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[3] = (out >> 16) + c;

		yp += 4;
		uvp += 2;
		xip += 4;
	    }

	    yp -= width;
	    uvp -= width/2;
	    xip -= (ximagewidth+width);
	    z += 4;

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    uv = uvp[0];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[y0] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    xip[0] = (out >> 16) + c;

		    y1 = yp[1];

		    base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z+2]);

		    base = yuv2rgb[uv + (greymap[y1] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[1] = (out >> 16) + c;

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[y0] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    xip[2] = (out >> 16) + c;

		    y1 = yp[3];

		    base = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z+2]);

		    base = yuv2rgb[uv + (greymap[y1] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[3] = (out >> 16) + c;

		    yp += 4;
		    uvp += 2;
		    xip += 4;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    uv = uvp[0];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    xip[0] = (out >> 16) + c;

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z+2]);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[1] = (out >> 16) + c;

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z]);

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+1]);
		    xip[2] = (out >> 16) + c;

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];

		    base = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    COLORDITHER(lut, out, base, cd[z+2]);

		    base = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    COLORDITHER(lut, c, base, cd[z+3]);
		    xip[3] = (out >> 16) + c;

		    yp += 4;
		    uvp += 2;
		    xip += 4;
		}
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth*3-width;
	    z = (z + 4) % 16;
	}
    } else if (scalebits == 0) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[y*ximagewidth + x/2];
	z = (y % 4) * 4;
	while (height--) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[yp[1]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[0] = (out >> 16) + c;

		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[2]] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		base = yuv2rgb[uv + (greymap[yp[3]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out >> 16) + c;

		uv = uvp[2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[4]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		base = yuv2rgb[uv + (greymap[yp[5]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[2] = (out >> 16) + c;

		uv = uvp[3];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		base = yuv2rgb[uv + (greymap[yp[6]] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		base = yuv2rgb[uv + (greymap[yp[7]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[3] = (out >> 16) + c;

		yp += 8;
		uvp += 4;
		xip += 4;
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth-width/2;
	    z = (z + 4) % 16;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+1))];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[0] = (out >> 16) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[1] = (out >> 16) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z]);

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+1]);
		xip[2] = (out >> 16) + c;

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		COLORDITHER(lut, out, base, cd[z+2]);

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		base = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];
		COLORDITHER(lut, c, base, cd[z+3]);
		xip[3] = (out >> 16) + c;

		yp += 2*scaler;
		uvp += scaler;
		xip += 4;
	    }
	    yp += (imagewidth << scalebits)-width;
	    uvp += ((imagewidth << scalebits)-width)/2;
	    xip += ximagewidth-(width >> (scalebits+1));
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}

void VidColor_MSB24bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line/4;
    register u_char y0, y1, y2, *yp;
    register u_short uv, *uvp;
    register u_char *greymap=vidPtr->image->greymap;
    register u_long *xip;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[(y*2+1)*ximagewidth + x*2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[0] = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		xip[1] = yuv2rgb[uv + (greymap[y0] >> 2)];

		y1 = yp[1];
		xip[2] = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		xip[3] = yuv2rgb[uv + (greymap[y1] >> 2)];

		y0 = yp[2];
		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[4] = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		xip[5] = yuv2rgb[uv + (greymap[y0] >> 2)];

		y1 = yp[3];
		xip[6] = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		xip[7] = yuv2rgb[uv + (greymap[y1] >> 2)];

		yp += 4;
		uvp += 2;
		xip += 8;
	    }

	    yp -= width;
	    uvp -= width/2;
	    xip -= (ximagewidth+width*2);
	    z += 4;

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    uv = uvp[0];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		    xip[0] = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    xip[1] = yuv2rgb[uv + (greymap[y0] >> 2)];

		    y1 = yp[1];
		    xip[2] = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    xip[3] = yuv2rgb[uv + (greymap[y1] >> 2)];

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		    xip[4] = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    xip[5] = yuv2rgb[uv + (greymap[y0] >> 2)];

		    y1 = yp[3];
		    xip[6] = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    xip[7] = yuv2rgb[uv + (greymap[y1] >> 2)];

		    yp += 4;
		    uvp += 2;
		    xip += 8;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    uv = uvp[0];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		    xip[0] = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    xip[1] = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    xip[2] = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    xip[3] = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		    xip[4] = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    xip[5] = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    xip[6] = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    xip[7] = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];

		    yp += 4;
		    uvp += 2;
		    xip += 8;
		}
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth*3-width*2;
	    z = (z + 4) % 16;
	}
    } else if (scalebits == 0) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[y*ximagewidth + x];
	z = (y % 4) * 4;
	while (height--) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[0] = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		xip[1] = yuv2rgb[uv + (greymap[yp[1]] >> 2)];

		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[2] = yuv2rgb[uv + (greymap[yp[2]] >> 2)];
		xip[3] = yuv2rgb[uv + (greymap[yp[3]] >> 2)];

		uv = uvp[2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[4] = yuv2rgb[uv + (greymap[yp[4]] >> 2)];
		xip[5] = yuv2rgb[uv + (greymap[yp[5]] >> 2)];

		uv = uvp[3];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[6] = yuv2rgb[uv + (greymap[yp[6]] >> 2)];
		xip[7] = yuv2rgb[uv + (greymap[yp[7]] >> 2)];

		yp += 8;
		uvp += 4;
		xip += 8;
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth-width;
	    z = (z + 4) % 16;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)
		  [(y >> scalebits)*ximagewidth + (x >> scalebits)];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[0] = yuv2rgb[uv + (greymap[yp[0]] >> 2)];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[1] = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[2] = yuv2rgb[uv + (greymap[yp[0]] >> 2)];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[3] = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[4] = yuv2rgb[uv + (greymap[yp[0]] >> 2)];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[5] = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[6] = yuv2rgb[uv + (greymap[yp[0]] >> 2)];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[7] = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];

		yp += 2*scaler;
		uvp += scaler;
		xip += 8;
	    }
	    yp += (imagewidth << scalebits)-width;
	    uvp += ((imagewidth << scalebits)-width)/2;
	    xip += ximagewidth-(width >> scalebits);
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}

void VidColor_LSB24bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    register int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->bytes_per_line/4;
    register u_char y0, y1, y2, *yp;
    register u_short uv, *uvp;
    register u_char *greymap=vidPtr->image->greymap;
    register u_long *xip;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[(y*2+1)*ximagewidth + x*2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[0] = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		xip[1] = yuv2rgb[uv + (greymap[y0] >> 2)];

		y1 = yp[1];
		xip[2] = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		xip[3] = yuv2rgb[uv + (greymap[y1] >> 2)];

		y0 = yp[2];
		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[4] = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		xip[5] = yuv2rgb[uv + (greymap[y0] >> 2)];

		y1 = yp[3];
		xip[6] = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		xip[7] = yuv2rgb[uv + (greymap[y1] >> 2)];

		yp += 4;
		uvp += 2;
		xip += 8;
	    }

	    yp -= width;
	    uvp -= width/2;
	    xip -= (ximagewidth+width*2);
	    z += 4;

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    uv = uvp[0];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		    xip[0] = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    xip[1] = yuv2rgb[uv + (greymap[y0] >> 2)];

		    y1 = yp[1];
		    xip[2] = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    xip[3] = yuv2rgb[uv + (greymap[y1] >> 2)];

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		    xip[4] = yuv2rgb[uv + (greymap[(y1+y0)/2] >> 2)];
		    xip[5] = yuv2rgb[uv + (greymap[y0] >> 2)];

		    y1 = yp[3];
		    xip[6] = yuv2rgb[uv + (greymap[(y0+y1)/2] >> 2)];
		    xip[7] = yuv2rgb[uv + (greymap[y1] >> 2)];

		    yp += 4;
		    uvp += 2;
		    xip += 8;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    uv = uvp[0];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		    xip[0] = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    xip[1] = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    xip[2] = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    xip[3] = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		    xip[4] = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];
		    xip[5] = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    xip[6] = yuv2rgb[uv + (greymap[(y1+y2)/2] >> 2)];
		    xip[7] = yuv2rgb[uv + (greymap[(y0+y2)/2] >> 2)];

		    yp += 4;
		    uvp += 2;
		    xip += 8;
		}
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth*3-width*2;
	    z = (z + 4) % 16;
	}
    } else if (scalebits == 0) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)[y*ximagewidth + x];
	z = (y % 4) * 4;
	while (height--) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[0] = yuv2rgb[uv + (greymap[yp[0]] >> 2)];
		xip[1] = yuv2rgb[uv + (greymap[yp[1]] >> 2)];

		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[2] = yuv2rgb[uv + (greymap[yp[2]] >> 2)];
		xip[3] = yuv2rgb[uv + (greymap[yp[3]] >> 2)];

		uv = uvp[2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[4] = yuv2rgb[uv + (greymap[yp[4]] >> 2)];
		xip[5] = yuv2rgb[uv + (greymap[yp[5]] >> 2)];

		uv = uvp[3];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[6] = yuv2rgb[uv + (greymap[yp[6]] >> 2)];
		xip[7] = yuv2rgb[uv + (greymap[yp[7]] >> 2)];

		yp += 8;
		uvp += 4;
		xip += 8;
	    }
	    yp += imagewidth-width;
	    uvp += (imagewidth-width)/2;
	    xip += ximagewidth-width;
	    z = (z + 4) % 16;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (u_short *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((u_long *)vidPtr->ximage->data)
		  [(y >> scalebits)*ximagewidth + (x >> scalebits)];
	z = ((y >> scalebits) % 4) * 4;
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[0] = yuv2rgb[uv + (greymap[yp[0]] >> 2)];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[1] = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[2] = yuv2rgb[uv + (greymap[yp[0]] >> 2)];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[3] = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[4] = yuv2rgb[uv + (greymap[yp[0]] >> 2)];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[5] = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[6] = yuv2rgb[uv + (greymap[yp[0]] >> 2)];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[7] = yuv2rgb[uv + (greymap[yp[scaler]] >> 2)];

		yp += 2*scaler;
		uvp += scaler;
		xip += 8;
	    }
	    yp += (imagewidth << scalebits)-width;
	    uvp += ((imagewidth << scalebits)-width)/2;
	    xip += ximagewidth-(width >> scalebits);
	    z = (z + 4) % 16;

	    height -= scaler;
	}
    }
}
