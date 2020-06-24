/*
	Netvideo version 3.3
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
#include <netinet/in.h>
#ifdef AIX
#include <net/nh.h>
#endif
#include <X11/Xlib.h>
#include "sized_types.h"
#include "vid_util.h"
#include "vid_image.h"
#include "vid_code.h"
#include "nv.h"

static uint8 *NV_DoBlock(vidimage_t *image, uint8 *data, uint8 *dataLim,
			  int color, nv_revtransform_t *revTransform)
{
    int i, j, k, lim, x0, y0, w, w0, offset, run, width=image->width/4;
    register uint32 *yp, *uvp;
    register int8 *blkp;
    static uint32 block[32];

    if (dataLim-data < 3) return 0;

    w0 = w = data[0];
    x0 = data[1];
    y0 = data[2];
    data += 3;

    if ((x0+w > width/2) || (y0 >= image->height/8)) return 0;

    offset = y0*8*width+x0*2;
    yp = &((uint32 *)image->y_data)[offset];
    uvp = image->uv_data? &((uint32 *)image->uv_data)[offset] : 0;

    while (w-- > 0) {
	if (data >= dataLim) return 0;

	lim=color? 128 : 64;
	for (i=0; i<lim; ) {
	    run = *data++;
	    j = run >> 6;
	    k = run & 0x3f;
	    if (i+j+k > lim) return 0;
	    blkp = (int8 *)block;
	    while (j--) blkp[i++] = (int8) *data++;
	    while (k--) blkp[i++] = 0;
	}

	(*revTransform)(block, (uint8 *)yp, (uint8 *)uvp, width*4);

	yp += 2;
	if (uvp) uvp += 2;
    }

    VidImage_UpdateRect(image, x0*8, y0*8, w0*8, 8);

    return data;
}

int NV_Decode(vidimage_t *image, uint8 *data, int len)
{
    int color, iscolor, encoding, width, height;
    uint8 *dataLim=data+len;

    if (dataLim-data < 4) return 1;

    width  = (data[0] << 8) + data[1];
    height = (data[2] << 8) + data[3];
    data += 4;

    color = ((width & NV_COLORFLAG) != 0);
    width &= NV_WIDTHMASK;

    encoding = height & NV_ENCODINGMASK;
    height &= NV_HEIGHTMASK;

    if ((width >= NV_MAX_WIDTH) || (height >= NV_MAX_HEIGHT)) return 1;

    if ((width != image->width) || (height != image->height))
	VidImage_SetSize(image, width, height);

    iscolor = ((image->flags & VIDIMAGE_ISCOLOR) != 0);
    if (color != iscolor)
	VidImage_SetColor(image, color, image->flags & VIDIMAGE_WANTCOLOR);

    switch (encoding) {
    case NV_ENCODING_STD:
	while ((data != 0) && (data < dataLim))
	    data = NV_DoBlock(image, data, dataLim, color, NV_RevTransform);
	break;
    case NV_ENCODING_DCT:
	while ((data != 0) && (data < dataLim))
	    data = NV_DoBlock(image, data, dataLim, color, NVDCT_RevTransform);
	break;
    default:
	return 1;
    }

    return (data != dataLim);
}
