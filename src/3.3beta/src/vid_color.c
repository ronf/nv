/*
        Netvideo version 3.3
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
#include <X11/Xlib.h>
#include <tk.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_util.h"
#include "vid_widget.h"

extern uint8 yuv_dither8[65536*16];
extern uint32 yuv_cmap[65536];

void VidColor_MSB8bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line/4;
    uint8 y0, y1, y2, *yp, *greymap=vidPtr->image->greymap, *dith=yuv_dither8;
    uint16 uv, *uvp, yuv;
    uint32 *xip, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y*2+1)*ximagewidth + x/2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		yuv = uv + greymap[(y1+y0)/2];
		out = dith[yuv*16+z];

		yuv = uv + greymap[y0];
		out = (out << 8) + dith[yuv*16+z+1];

		y1 = yp[1];

		yuv = uv + greymap[(y0+y1)/2];
		out = (out << 8) + dith[yuv*16+z+2];

		yuv = uv + greymap[y1];
		xip[0] = (out << 8) + dith[yuv*16+z+3];

		y0 = yp[2];
		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		yuv = uv + greymap[(y1+y0)/2];
		out = dith[yuv*16+z];

		yuv = uv + greymap[y0];
		out = (out << 8) + dith[yuv*16+z+1];

		y1 = yp[3];

		yuv = uv + greymap[(y0+y1)/2];
		out = (out << 8) + dith[yuv*16+z+2];

		yuv = uv + greymap[y1];
		xip[1] = (out << 8) + dith[yuv*16+z+3];

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

		    yuv = uv + greymap[(y1+y0)/2];
		    out = dith[yuv*16+z];

		    yuv = uv + greymap[y0];
		    out = (out << 8) + dith[yuv*16+z+1];

		    y1 = yp[1];

		    yuv = uv + greymap[(y0+y1)/2];
		    out = (out << 8) + dith[yuv*16+z+2];

		    yuv = uv + greymap[y1];
		    xip[0] = (out << 8) + dith[yuv*16+z+3];

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    yuv = uv + greymap[(y1+y0)/2];
		    out = dith[yuv*16+z];

		    yuv = uv + greymap[y0];
		    out = (out << 8) + dith[yuv*16+z+1];

		    y1 = yp[3];

		    yuv = uv + greymap[(y0+y1)/2];
		    out = (out << 8) + dith[yuv*16+z+2];

		    yuv = uv + greymap[y1];
		    xip[1] = (out << 8) + dith[yuv*16+z+3];

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

		    yuv = uv + greymap[(y0+y2)/2];
		    out = dith[yuv*16+z];

		    yuv = uv + greymap[(y1+y2)/2];
		    out = (out << 8) + dith[yuv*16+z+1];

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];

		    yuv = uv + greymap[(y1+y2)/2];
		    out = (out << 8) + dith[yuv*16+z+2];

		    yuv = uv + greymap[(y0+y2)/2];
		    xip[0] = (out << 8) + dith[yuv*16+z+3];

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    yuv = uv + greymap[(y0+y2)/2];
		    out = dith[yuv*16+z];

		    yuv = uv + greymap[(y1+y2)/2];
		    out = (out << 8) + dith[yuv*16+z+1];

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];

		    yuv = uv + greymap[(y1+y2)/2];
		    out = (out << 8) + dith[yuv*16+z+2];

		    yuv = uv + greymap[(y0+y2)/2];
		    xip[1] = (out << 8) + dith[yuv*16+z+3];

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[y*ximagewidth + x/4];
	z = (y % 4) * 4;
	while (height-- > 0) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		yuv = uv + greymap[yp[0]];
		out = dith[yuv*16+z];

		yuv = uv + greymap[yp[1]];
		out = (out << 8) + dith[yuv*16+z+1];

		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		yuv = uv + greymap[yp[2]];
		out = (out << 8) + dith[yuv*16+z+2];

		yuv = uv + greymap[yp[3]];
		xip[0] = (out << 8) + dith[yuv*16+z+3];

		uv = uvp[2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		yuv = uv + greymap[yp[4]];
		out = dith[yuv*16+z];

		yuv = uv + greymap[yp[5]];
		out = (out << 8) + dith[yuv*16+z+1];

		uv = uvp[3];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		yuv = uv + greymap[yp[6]];
		out = (out << 8) + dith[yuv*16+z+2];

		yuv = uv + greymap[yp[7]];
		xip[1] = (out << 8) + dith[yuv*16+z+3];

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+2))];
	z = ((y >> scalebits) % 4) * 4;
	while (height > 0) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		yuv = uv + greymap[yp[0]];
		out = dith[yuv*16+z];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		yuv = uv + greymap[yp[scaler]];
		out = (out << 8) + dith[yuv*16+z+1];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		yuv = uv + greymap[yp[0]];
		out = (out << 8) + dith[yuv*16+z+2];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		yuv = uv + greymap[yp[scaler]];
		xip[0] = (out << 8) + dith[yuv*16+z+3];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		yuv = uv + greymap[yp[0]];
		out = dith[yuv*16+z];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		yuv = uv + greymap[yp[scaler]];
		out = (out << 8) + dith[yuv*16+z+1];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		yuv = uv + greymap[yp[0]];
		out = (out << 8) + dith[yuv*16+z+2];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		yuv = uv + greymap[yp[scaler]];
		xip[1] = (out << 8) + dith[yuv*16+z+3];

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
    int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line/4;
    uint8 y0, y1, y2, *yp, *greymap=vidPtr->image->greymap, *dith=yuv_dither8;
    uint16 uv, *uvp, yuv;
    uint32 *xip, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y*2+1)*ximagewidth + x/2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		yuv = uv + greymap[(y1+y0)/2];
		out = dith[yuv*16+z];

		yuv = uv + greymap[y0];
		out += (dith[yuv*16+z+1] << 8);

		y1 = yp[1];

		yuv = uv + greymap[(y0+y1)/2];
		out += (dith[yuv*16+z+2] << 16);

		yuv = uv + greymap[y1];
		xip[0] = out + (dith[yuv*16+z+3] << 24);

		y0 = yp[2];
		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		yuv = uv + greymap[(y1+y0)/2];
		out = dith[yuv*16+z];

		yuv = uv + greymap[y0];
		out += (dith[yuv*16+z+1] << 8);

		y1 = yp[3];

		yuv = uv + greymap[(y0+y1)/2];
		out += (dith[yuv*16+z+2] << 16);

		yuv = uv + greymap[y1];
		xip[1] = out + (dith[yuv*16+z+3] << 24);

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

		    yuv = uv + greymap[(y1+y0)/2];
		    out = dith[yuv*16+z];

		    yuv = uv + greymap[y0];
		    out += (dith[yuv*16+z+1] << 8);

		    y1 = yp[1];

		    yuv = uv + greymap[(y0+y1)/2];
		    out += (dith[yuv*16+z+2] << 16);

		    yuv = uv + greymap[y1];
		    xip[0] = out + (dith[yuv*16+z+3] << 24);

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    yuv = uv + greymap[(y1+y0)/2];
		    out = dith[yuv*16+z];

		    yuv = uv + greymap[y0];
		    out += (dith[yuv*16+z+1] << 8);

		    y1 = yp[3];

		    yuv = uv + greymap[(y0+y1)/2];
		    out += (dith[yuv*16+z+2] << 16);

		    yuv = uv + greymap[y1];
		    xip[1] = out + (dith[yuv*16+z+3] << 24);

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

		    yuv = uv + greymap[(y0+y2)/2];
		    out = dith[yuv*16+z];

		    yuv = uv + greymap[(y1+y2)/2];
		    out += (dith[yuv*16+z+1] << 8);

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];

		    yuv = uv + greymap[(y1+y2)/2];
		    out += (dith[yuv*16+z+2] << 16);

		    yuv = uv + greymap[(y0+y2)/2];
		    xip[0] = out + (dith[yuv*16+z+3] << 24);

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    yuv = uv + greymap[(y0+y2)/2];
		    out = dith[yuv*16+z];

		    yuv = uv + greymap[(y1+y2)/2];
		    out += (dith[yuv*16+z+1] << 8);

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];

		    yuv = uv + greymap[(y1+y2)/2];
		    out += (dith[yuv*16+z+2] << 16);

		    yuv = uv + greymap[(y0+y2)/2];
		    xip[1] = out + (dith[yuv*16+z+3] << 24);

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[y*ximagewidth + x/4];
	z = (y % 4) * 4;
	while (height-- > 0) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		yuv = uv + greymap[yp[0]];
		out = dith[yuv*16+z];

		yuv = uv + greymap[yp[1]];
		out += (dith[yuv*16+z+1] << 8);

		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		yuv = uv + greymap[yp[2]];
		out += (dith[yuv*16+z+2] << 16);

		yuv = uv + greymap[yp[3]];
		xip[0] = out + (dith[yuv*16+z+3] << 24);

		uv = uvp[2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		yuv = uv + greymap[yp[4]];
		out = dith[yuv*16+z];

		yuv = uv + greymap[yp[5]];
		out += (dith[yuv*16+z+1] << 8);

		uv = uvp[3];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		yuv = uv + greymap[yp[6]];
		out += (dith[yuv*16+z+2] << 16);

		yuv = uv + greymap[yp[7]];
		xip[1] = out + (dith[yuv*16+z+3] << 24);

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+2))];
	z = ((y >> scalebits) % 4) * 4;
	while (height > 0) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		yuv = uv + greymap[yp[0]];
		out = dith[yuv*16+z];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		yuv = uv + greymap[yp[scaler]];
		out += (dith[yuv*16+z+1] << 8);

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		yuv = uv + greymap[yp[0]];
		out += (dith[yuv*16+z+2] << 16);

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		yuv = uv + greymap[yp[scaler]];
		xip[0] = out + (dith[yuv*16+z+3] << 24);

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		yuv = uv + greymap[yp[0]];
		out = dith[yuv*16+z];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		yuv = uv + greymap[yp[scaler]];
		out += (dith[yuv*16+z+1] << 8);

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		yuv = uv + greymap[yp[0]];
		out += (dith[yuv*16+z+2] << 16);

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		yuv = uv + greymap[yp[scaler]];
		xip[1] = out + (dith[yuv*16+z+3] << 24);

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
    int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line/4;
    uint8 y0, y1, y2, *yp, *greymap=vidPtr->image->greymap;
    uint16 uv, *uvp;
    uint32 *xip, out, *cmap=yuv_cmap;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[(y*2+1)*ximagewidth + x];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		out = cmap[uv + greymap[(y1+y0)/2]];
		xip[0] = (out << 16) + cmap[uv + greymap[y0]];

		y1 = yp[1];

		out = cmap[uv + greymap[(y0+y1)/2]];
		xip[1] = (out << 16) + cmap[uv + greymap[y1]];

		y0 = yp[2];
		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		out = cmap[uv + greymap[(y1+y0)/2]];
		xip[2] = (out << 16) + cmap[uv + greymap[y0]];

		y1 = yp[3];

		out = cmap[uv + greymap[(y0+y1)/2]];
		xip[3] = (out << 16) + cmap[uv + greymap[y1]];

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

		    out = cmap[uv + greymap[(y1+y0)/2]];
		    xip[0] = (out << 16) + cmap[uv + greymap[y0]];

		    y1 = yp[1];

		    out = cmap[uv + greymap[(y0+y1)/2]];
		    xip[1] = (out << 16) + cmap[uv + greymap[y1]];

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    out = cmap[uv + greymap[(y1+y0)/2]];
		    xip[2] = (out << 16) + cmap[uv + greymap[y0]];

		    y1 = yp[3];

		    out = cmap[uv + greymap[(y0+y1)/2]];
		    xip[3] = (out << 16) + cmap[uv + greymap[y1]];

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

		    out = cmap[uv + greymap[(y0+y2)/2]];
		    xip[0] = (out << 16) + cmap[uv + greymap[(y1+y2)/2]];

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];

		    out = cmap[uv + greymap[(y1+y2)/2]];
		    xip[1] = (out << 16) + cmap[uv + greymap[(y0+y2)/2]];

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		    out = cmap[uv + greymap[(y0+y2)/2]];
		    xip[2] = (out << 16) + cmap[uv + greymap[(y1+y2)/2]];

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];

		    out = cmap[uv + greymap[(y1+y2)/2]];
		    xip[3] = (out << 16) + cmap[uv + greymap[(y0+y2)/2]];

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[y*ximagewidth + x/2];
	z = (y % 4) * 4;
	while (height-- > 0) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		out = cmap[uv + greymap[yp[0]]];
		xip[0] = (out << 16) + cmap[uv + greymap[yp[1]]];

		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		out = cmap[uv + greymap[yp[2]]];
		xip[1] = (out << 16) + cmap[uv + greymap[yp[3]]];

		uv = uvp[2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		out = cmap[uv + greymap[yp[4]]];
		xip[2] = (out << 16) + cmap[uv + greymap[yp[5]]];

		uv = uvp[3];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);

		out = cmap[uv + greymap[yp[6]]];
		xip[3] = (out << 16) + cmap[uv + greymap[yp[7]]];

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+1))];
	z = ((y >> scalebits) % 4) * 4;
	while (height > 0) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		out = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[0] = (out << 16) + cmap[uv + greymap[yp[scaler]]];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		out = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[1] = (out << 16) + cmap[uv + greymap[yp[scaler]]];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		out = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[2] = (out << 16) + cmap[uv + greymap[yp[scaler]]];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		out = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[3] = (out << 16) + cmap[uv + greymap[yp[scaler]]];

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
    int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line/4;
    uint8 y0, y1, y2, *yp, *greymap=vidPtr->image->greymap;
    uint16 uv, *uvp;
    uint32 *xip, out, *cmap=yuv_cmap;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[(y*2+1)*ximagewidth + x];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		out = cmap[uv + greymap[(y1+y0)/2]];
		xip[0] = out + (cmap[uv + greymap[y0]] << 16);

		y1 = yp[1];

		out = cmap[uv + greymap[(y0+y1)/2]];
		xip[1] = out + (cmap[uv + greymap[y1]] << 16);

		y0 = yp[2];
		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		out = cmap[uv + greymap[(y1+y0)/2]];
		xip[2] = out + (cmap[uv + greymap[y0]] << 16);

		y1 = yp[3];

		out = cmap[uv + greymap[(y0+y1)/2]];
		xip[3] = out + (cmap[uv + greymap[y1]] << 16);

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

		    out = cmap[uv + greymap[(y1+y0)/2]];
		    xip[0] = out + (cmap[uv + greymap[y0]] << 16);

		    y1 = yp[1];

		    out = cmap[uv + greymap[(y0+y1)/2]];
		    xip[1] = out + (cmap[uv + greymap[y1]] << 16);

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    out = cmap[uv + greymap[(y1+y0)/2]];
		    xip[2] = out + (cmap[uv + greymap[y0]] << 16);

		    y1 = yp[3];

		    out = cmap[uv + greymap[(y0+y1)/2]];
		    xip[3] = out + (cmap[uv + greymap[y1]] << 16);

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

		    out = cmap[uv + greymap[(y0+y2)/2]];
		    xip[0] = out + (cmap[uv + greymap[(y1+y2)/2]] << 16);

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];

		    out = cmap[uv + greymap[(y1+y2)/2]];
		    xip[1] = out + (cmap[uv + greymap[(y0+y2)/2]] << 16);

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		    out = cmap[uv + greymap[(y0+y2)/2]];
		    xip[2] = out + (cmap[uv + greymap[(y1+y2)/2]] << 16);

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];

		    out = cmap[uv + greymap[(y1+y2)/2]];
		    xip[3] = out + (cmap[uv + greymap[(y0+y2)/2]] << 16);

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[y*ximagewidth + x/2];
	z = (y % 4) * 4;
	while (height-- > 0) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		out = cmap[uv + greymap[yp[0]]];
		xip[0] = out + (cmap[uv + greymap[yp[1]]] << 16);

		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		out = cmap[uv + greymap[yp[2]]];
		xip[1] = out + (cmap[uv + greymap[yp[3]]] << 16);

		uv = uvp[2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		out = cmap[uv + greymap[yp[4]]];
		xip[2] = out + (cmap[uv + greymap[yp[5]]] << 16);

		uv = uvp[3];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);

		out = cmap[uv + greymap[yp[6]]];
		xip[3] = out + (cmap[uv + greymap[yp[7]]] << 16);

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+1))];
	z = ((y >> scalebits) % 4) * 4;
	while (height > 0) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		out = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[0] = out + (cmap[uv + greymap[yp[scaler]]] << 16);

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		out = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[1] = out + (cmap[uv + greymap[yp[scaler]]] << 16);

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		out = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[2] = out + (cmap[uv + greymap[yp[scaler]]] << 16);

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		out = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[3] = out + (cmap[uv + greymap[yp[scaler]]] << 16);

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
    int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line/4;
    uint8 y0, y1, y2, *yp, *greymap=vidPtr->image->greymap;
    uint16 uv, *uvp;
    uint32 *xip, *cmap=yuv_cmap;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y*2+1)*ximagewidth + x*2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[0] = cmap[uv + greymap[(y1+y0)/2]];
		xip[1] = cmap[uv + greymap[y0]];

		y1 = yp[1];
		xip[2] = cmap[uv + greymap[(y0+y1)/2]];
		xip[3] = cmap[uv + greymap[y1]];

		y0 = yp[2];
		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[4] = cmap[uv + greymap[(y1+y0)/2]];
		xip[5] = cmap[uv + greymap[y0]];

		y1 = yp[3];
		xip[6] = cmap[uv + greymap[(y0+y1)/2]];
		xip[7] = cmap[uv + greymap[y1]];

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
		    xip[0] = cmap[uv + greymap[(y1+y0)/2]];
		    xip[1] = cmap[uv + greymap[y0]];

		    y1 = yp[1];
		    xip[2] = cmap[uv + greymap[(y0+y1)/2]];
		    xip[3] = cmap[uv + greymap[y1]];

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		    xip[4] = cmap[uv + greymap[(y1+y0)/2]];
		    xip[5] = cmap[uv + greymap[y0]];

		    y1 = yp[3];
		    xip[6] = cmap[uv + greymap[(y0+y1)/2]];
		    xip[7] = cmap[uv + greymap[y1]];

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
		    xip[0] = cmap[uv + greymap[(y0+y2)/2]];
		    xip[1] = cmap[uv + greymap[(y1+y2)/2]];

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    xip[2] = cmap[uv + greymap[(y1+y2)/2]];
		    xip[3] = cmap[uv + greymap[(y0+y2)/2]];

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		    xip[4] = cmap[uv + greymap[(y0+y2)/2]];
		    xip[5] = cmap[uv + greymap[(y1+y2)/2]];

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    xip[6] = cmap[uv + greymap[(y1+y2)/2]];
		    xip[7] = cmap[uv + greymap[(y0+y2)/2]];

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[y*ximagewidth + x];
	z = (y % 4) * 4;
	while (height-- > 0) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[0] = cmap[uv + greymap[yp[0]]];
		xip[1] = cmap[uv + greymap[yp[1]]];

		uv = uvp[1];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[2] = cmap[uv + greymap[yp[2]]];
		xip[3] = cmap[uv + greymap[yp[3]]];

		uv = uvp[2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[4] = cmap[uv + greymap[yp[4]]];
		xip[5] = cmap[uv + greymap[yp[5]]];

		uv = uvp[3];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[6] = cmap[uv + greymap[yp[6]]];
		xip[7] = cmap[uv + greymap[yp[7]]];

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y >> scalebits)*ximagewidth + (x >> scalebits)];
	z = ((y >> scalebits) % 4) * 4;
	while (height > 0) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[0] = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[1] = cmap[uv + greymap[yp[scaler]]];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[2] = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[3] = cmap[uv + greymap[yp[scaler]]];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[4] = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[5] = cmap[uv + greymap[yp[scaler]]];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[6] = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = (uv & 0xf800) + ((uv << 3) & 0x7c0);
		xip[7] = cmap[uv + greymap[yp[scaler]]];

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
    int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line/4;
    uint8 y0, y1, y2, *yp, *greymap=vidPtr->image->greymap;
    uint16 uv, *uvp;
    uint32 *xip, *cmap=yuv_cmap;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y*2+1)*ximagewidth + x*2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[0] = cmap[uv + greymap[(y1+y0)/2]];
		xip[1] = cmap[uv + greymap[y0]];

		y1 = yp[1];
		xip[2] = cmap[uv + greymap[(y0+y1)/2]];
		xip[3] = cmap[uv + greymap[y1]];

		y0 = yp[2];
		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[4] = cmap[uv + greymap[(y1+y0)/2]];
		xip[5] = cmap[uv + greymap[y0]];

		y1 = yp[3];
		xip[6] = cmap[uv + greymap[(y0+y1)/2]];
		xip[7] = cmap[uv + greymap[y1]];

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
		    xip[0] = cmap[uv + greymap[(y1+y0)/2]];
		    xip[1] = cmap[uv + greymap[y0]];

		    y1 = yp[1];
		    xip[2] = cmap[uv + greymap[(y0+y1)/2]];
		    xip[3] = cmap[uv + greymap[y1]];

		    y0 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		    xip[4] = cmap[uv + greymap[(y1+y0)/2]];
		    xip[5] = cmap[uv + greymap[y0]];

		    y1 = yp[3];
		    xip[6] = cmap[uv + greymap[(y0+y1)/2]];
		    xip[7] = cmap[uv + greymap[y1]];

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
		    xip[0] = cmap[uv + greymap[(y0+y2)/2]];
		    xip[1] = cmap[uv + greymap[(y1+y2)/2]];

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    xip[2] = cmap[uv + greymap[(y1+y2)/2]];
		    xip[3] = cmap[uv + greymap[(y0+y2)/2]];

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    uv = uvp[1];
		    uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		    xip[4] = cmap[uv + greymap[(y0+y2)/2]];
		    xip[5] = cmap[uv + greymap[(y1+y2)/2]];

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    xip[6] = cmap[uv + greymap[(y1+y2)/2]];
		    xip[7] = cmap[uv + greymap[(y0+y2)/2]];

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[y*ximagewidth + x];
	z = (y % 4) * 4;
	while (height-- > 0) {
	    for (w=width; w > 0; w -= 8) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[0] = cmap[uv + greymap[yp[0]]];
		xip[1] = cmap[uv + greymap[yp[1]]];

		uv = uvp[1];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[2] = cmap[uv + greymap[yp[2]]];
		xip[3] = cmap[uv + greymap[yp[3]]];

		uv = uvp[2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[4] = cmap[uv + greymap[yp[4]]];
		xip[5] = cmap[uv + greymap[yp[5]]];

		uv = uvp[3];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[6] = cmap[uv + greymap[yp[6]]];
		xip[7] = cmap[uv + greymap[yp[7]]];

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
	uvp = (uint16 *) &vidPtr->image->uv_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y >> scalebits)*ximagewidth + (x >> scalebits)];
	z = ((y >> scalebits) % 4) * 4;
	while (height > 0) {
	    for (w=width; w > 0; w -= 8*scaler) {
		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[0] = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[1] = cmap[uv + greymap[yp[scaler]]];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[2] = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[3] = cmap[uv + greymap[yp[scaler]]];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[4] = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[5] = cmap[uv + greymap[yp[scaler]]];

		yp += 2*scaler;
		uvp += scaler;

		uv = uvp[0];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[6] = cmap[uv + greymap[yp[0]]];

		uv = uvp[scaler/2];
		uv = ((uv << 8) & 0xf800) + ((uv >> 5) & 0x7c0);
		xip[7] = cmap[uv + greymap[yp[scaler]]];

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
