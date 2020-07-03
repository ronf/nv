/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>

	Sun CellB decode routine
	Based heavily on code by Michael Speer <Michael.Speer@eng.sun.com>
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
#include "cellb.h"

extern uint16 cellb_yytable[256], cellb_uvtable[256];

int CellB_Decode(vidimage_t *image, uint8 *data, int len)
{
    int orig_cellx, orig_celly, cellx, celly, width, height, offset;
    int pattern, yyindx, uvindx;
    uint32 *yorigin, *uvorigin, *yptr, *uvptr, yout, uvout;
    uint8 *dataLim=data+len, y1, y2, uu, vv;

    if (dataLim-data < 8) return 1;

    /* Get cell x, y location */
    orig_cellx = cellx = (data[0] << 8) + data[1];
    orig_celly = celly = (data[2] << 8) + data[3];
    data += 4;

    /* Get width and height */
    width = (data[0] << 8) + data[1];
    height = (data[2] << 8) + data[3];
    data += 4;

    /* Make sure image is the correct size */
    if ((width != image->width) || (height != image->height))
	VidImage_SetSize(image, width, height);

    /* Convert width and height to count in cells */
    width /= 4;
    height /= 4;

    /* If cellx or celly are out of range, exit */
    if ((cellx >= width) || (celly >= height)) return 1;

    yorigin  = (uint32 *) image->y_data;
    uvorigin = (uint32 *) image->uv_data;

    while (data < dataLim) {
	if (celly >= height) {
	    cellx = 0;
	    celly = height;
	    break;
	}

	pattern = *data;
	if (pattern >= 128) {
	    cellx += (pattern-127);
	    while (cellx >= width) {
		cellx -= width;
		celly++;
	    }
	    data++;
	} else {
	    if (dataLim-data < 4) return 1;

	    pattern = (pattern << 8) + data[1];
	    uvindx = data[2];
	    yyindx = data[3];

	    y1 = (uint8)(cellb_yytable[yyindx] >> 8);
	    y2 = (uint8)(cellb_yytable[yyindx] & 0xff);
	    uu = (uint8)(cellb_uvtable[uvindx] >> 8);
	    vv = (uint8)(cellb_uvtable[uvindx] & 0xff);

	    offset = celly*4*width + cellx;
	    yptr = &yorigin[offset];
	    uvptr = &uvorigin[offset];

	    if (LITTLEENDIAN) {
		yout = (pattern & 0x1000) ? y2 : y1;
		yout = (yout << 8) + ((pattern & 0x2000) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x4000) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x8000) ? y2 : y1);
		yptr[0] = yout;

		yout = (pattern & 0x0100) ? y2 : y1;
		yout = (yout << 8) + ((pattern & 0x0200) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0400) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0800) ? y2 : y1);
		yptr[width] = yout;
		yptr += 2*width;

		yout = (pattern & 0x0010) ? y2 : y1;
		yout = (yout << 8) + ((pattern & 0x0020) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0040) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0080) ? y2 : y1);
		yptr[0] = yout;

		yout = (pattern & 0x0001) ? y2 : y1;
		yout = (yout << 8) + ((pattern & 0x0002) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0004) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0008) ? y2 : y1);
		yptr[width] = yout;

		uvout = uu + (vv << 8) + (uu << 16) + (vv << 24);
		uvptr[0] = uvptr[width] = uvout;
		uvptr += 2*width;
		uvptr[0] = uvptr[width] = uvout;
	    } else {
		yout = (pattern & 0x8000) ? y2 : y1;
		yout = (yout << 8) + ((pattern & 0x4000) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x2000) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x1000) ? y2 : y1);
		yptr[0] = yout;

		yout = (pattern & 0x0800) ? y2 : y1;
		yout = (yout << 8) + ((pattern & 0x0400) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0200) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0100) ? y2 : y1);
		yptr[width] = yout;
		yptr += 2*width;

		yout = (pattern & 0x0080) ? y2 : y1;
		yout = (yout << 8) + ((pattern & 0x0040) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0020) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0010) ? y2 : y1);
		yptr[0] = yout;

		yout = (pattern & 0x0008) ? y2 : y1;
		yout = (yout << 8) + ((pattern & 0x0004) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0002) ? y2 : y1);
		yout = (yout << 8) + ((pattern & 0x0001) ? y2 : y1);
		yptr[width] = yout;

		uvout = (uu << 24) + (vv << 16) + (uu << 8) + vv;
		uvptr[0] = uvptr[width] = uvout;
		uvptr += 2*width;
		uvptr[0] = uvptr[width] = uvout;
	    }

	    cellx++;
	    if (cellx >= width) {
		cellx -= width;
		celly++;
	    }
	    data += 4;
	}
    }

    if (cellx == 0) celly--;
    VidImage_UpdateRect(image, 0, orig_celly*4, width*4,
	(celly-orig_celly+1)*4);

    return 0;
}
