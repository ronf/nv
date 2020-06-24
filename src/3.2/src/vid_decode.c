/*
	Netvideo version 3.2
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video decode routines
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

#include <sys/types.h>
#include "video.h"
#include "vidcode.h"

static u_char *VidDecode_DoBlock(vidimage_t *image, u_char *data,
				 u_char *dataLim, int color)
{
    int i, j, k, x0, y0, w, w0, offset, bg, run, width=image->width;
    register u_char *yp, *bgyp;
    register s_char *uvp, *bguvp, *blkp;
    static u_long block[32];

    if (dataLim-data < 3) return 0;

    w = *data++;
    bg = w & VIDCODE_BGFLAG;
    w0 = w  = w & ~VIDCODE_BGFLAG;
    x0 = *data++;
    y0 = *data++;

    if ((x0+w > width/8) || (y0 >= image->height/8)) return 0;

    offset = y0*8*width+x0*8;
    yp = &image->y_data[offset];
    bgyp = &image->bg_y_data[offset];
    if (image->uv_data) {
	uvp = &image->uv_data[offset];
	bguvp = &image->bg_uv_data[offset];
    } else {
	uvp = bguvp = 0;
    }
    while (w-- > 0) {
	if (data >= dataLim) return 0;

	if (*data == VIDCODE_BGBLOCK) {
	    data++;
	    for (i=0; i<8; i++) {
		yp[0] = bgyp[0];
		yp[1] = bgyp[1];
		yp[2] = bgyp[2];
		yp[3] = bgyp[3];
		yp[4] = bgyp[4];
		yp[5] = bgyp[5];
		yp[6] = bgyp[6];
		yp[7] = bgyp[7];
		yp += width;
		bgyp += width;
	    }
	    yp -= 8*width;
	    bgyp -= 8*width;
	    if (uvp) {
		for (i=0; i<8; i++) {
		    uvp[0] = bguvp[0];
		    uvp[1] = bguvp[1];
		    uvp[2] = bguvp[2];
		    uvp[3] = bguvp[3];
		    uvp[4] = bguvp[4];
		    uvp[5] = bguvp[5];
		    uvp[6] = bguvp[6];
		    uvp[7] = bguvp[7];
		    uvp += width;
		    bguvp += width;
		}
		uvp -= 8*width;
		bguvp -= 8*width;
	    }
	} else {
	    int lim=color? 128 : 64;
	    for (i=0; i<lim; ) {
		run = *data++;
		j = run >> 6;
		k = run & 0x3f;
		if (i+j+k > lim) return 0;
		blkp = (s_char *)block;
		while (j--) blkp[i++] = (s_char) *data++;
		while (k--) blkp[i++] = 0;
	    }

	    VidTransform_Rev(block, yp, uvp, width);
	    if (bg) {
		for (i=0; i<8; i++) {
		    bgyp[0] = yp[0];
		    bgyp[1] = yp[1];
		    bgyp[2] = yp[2];
		    bgyp[3] = yp[3];
		    bgyp[4] = yp[4];
		    bgyp[5] = yp[5];
		    bgyp[6] = yp[6];
		    bgyp[7] = yp[7];
		    yp += width;
		    bgyp += width;
		    if (uvp) {
			bguvp[0] = uvp[0];
			bguvp[1] = uvp[1];
			bguvp[2] = uvp[2];
			bguvp[3] = uvp[3];
			bguvp[4] = uvp[4];
			bguvp[5] = uvp[5];
			bguvp[6] = uvp[6];
			bguvp[7] = uvp[7];
			uvp += width;
			bguvp += width;
		    }
		}
		yp -= 8*width;
		bgyp -= 8*width;
		if (uvp) {
		    uvp -= 8*width;
		    bguvp -= 8*width;
		}
	    }
	}

	yp += 8;
	bgyp += 8;
	if (uvp) {
	    uvp += 8;
	    bguvp += 8;
	}
    }

    VidImage_UpdateRect(image, x0*8, y0*8, w0*8, 8);

    return data;
}

void VidDecode(vidimage_t *image, u_char *data, int len)
{
    int color, iscolor, width, height;
    u_char *p=data, *dataLim=data+len;

    width  = (p[0] << 8) + p[1];
    height = (p[2] << 8) + p[3];
    p += 4;

    color = ((width & VIDCODE_COLORFLAG) != 0);
    width &= VIDCODE_WIDTHMASK;
    if ((width >= VIDCODE_MAX_WIDTH) || (height >= VIDCODE_MAX_HEIGHT)) return;

    if ((width != image->width) || (height != image->height))
	VidImage_SetSize(image, width, height);

    iscolor = ((image->flags & VIDIMAGE_ISCOLOR) != 0);
    if (color != iscolor)
	VidImage_SetColor(image, color, image->flags & VIDIMAGE_WANTCOLOR);

    while ((p != 0) && (p < data+len))
	p = VidDecode_DoBlock(image, p, dataLim, color);
}
