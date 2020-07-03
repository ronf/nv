/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>
 
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
#include <netinet/in.h>
#ifdef AIX
#include <net/nh.h>
#endif
#include <X11/Xlib.h>
#include <tk.h>
#include "sized_types.h"
#include "vid_util.h"
#include "vid_image.h"
#include "vid_widget.h"

extern uint8 y_dither8[256*16];
extern uint32 y_cmap[256];

static uint8 halftone[1024] =
    { 108, 217, 192,  15, 103,  65, 129,  17, 
      191, 151, 206, 136, 182,  14, 197, 135, 
       54, 216,  96, 139, 157,  54, 190, 214, 
       45,  90, 199,   4, 253,  30,  82, 244, 
        1, 147,  87, 227, 179,  34, 240, 214, 
       49, 231,   2,  61, 235, 113, 156,  28, 
      170, 118, 185,  38, 236, 117,  76, 167, 
       12, 224,  53, 147, 102, 166, 207,  62, 
      116, 168,  29,  56, 114, 155,  79, 108, 
      167,  95, 126, 171,  86,  41, 252,  92, 
      225,   2, 247,  84,  17, 204,  29, 250, 
      109, 128, 186,  75, 215,  47, 135, 188, 
       43, 245, 203, 139, 233, 193,   8, 140, 
       33,  71, 248,  26, 216, 137,  58, 203, 
       74, 144,  49, 163, 132, 178,  93, 141, 
       62, 162,  22, 230, 123,   9,  92, 228, 
       78, 129,  93,  22,  74,  47, 254, 178, 
      222, 195, 149, 103, 177,   7, 159, 121, 
       21, 180, 209,  99, 221,  57, 231,  40, 
      208, 240,  98,  39, 174, 246, 161,  21, 
      221,  58, 185, 157, 218, 127,  99,  60, 
      115,  11,  53, 209,  68, 233, 194,  98, 
      240, 127,  68,  23, 115,  10, 150, 191, 
        0,  80, 145, 198, 110,  61, 144, 200, 
      173,  14, 249, 109,   3, 172, 205,  26, 
      153, 243, 131,  36, 122,  85,  18,  51, 
      168,  34, 228, 185, 253, 172,  67, 105, 
      125, 183,  57,  16, 212,  83,  29, 104, 
      120, 141,  40, 197,  84,  43, 239,  77, 
      191,  89, 166, 189, 220, 152, 248, 114, 
      217,  86, 148,  48,  80, 130, 212, 242, 
       44, 225, 155, 254, 132, 180, 241,  48, 
       87, 235,  69, 148, 228, 131, 162, 112, 
       17, 229,  64,   1, 100,  31, 177,  63, 
      138,   3, 199, 110, 160,  37,  16,  91, 
      167,  25, 114,  90,  46,   3, 157, 218, 
       10, 202, 166,  19,  97,  58,  30, 210, 
      146,  47, 111, 253, 145,  76, 208,  38, 
      189,  94, 246,  19, 206, 234, 187,  62, 
      140, 208,  71, 192, 233, 109,  73, 188, 
      125,  55, 111, 219, 185, 252, 174,  72, 
      223, 183, 128, 200,  20, 234, 159, 121, 
      230, 171,  57, 131,  71,  99, 149, 118, 
      246,  13, 176,  33, 151, 213, 140,  37, 
      153, 248,  78,  34, 123,  11, 137, 103, 
        6,  84,  35, 169,  92,  50, 105,   9, 
       74,  32, 154, 227, 179,  42,   6, 196, 
       48, 100, 224, 122,  59,  20,  95, 232, 
      180,  17, 195, 158,  88, 207,  44, 197, 
      244, 160, 230,  67, 218, 142, 196, 245, 
      136, 215, 104,  23, 122, 252, 162, 215, 
       75, 137, 163,  81, 251, 170, 201,  68, 
       45, 100, 138, 241,  52, 228, 150,  64, 
      121,  23, 136, 106,   5, 176,  36,  86, 
      167,  49, 190,  83, 207,  55,  87, 115, 
       31, 234,   7, 205,  40, 134,   5, 115, 
      225, 204,  66,   0, 169, 106,  15, 177, 
       90, 209,  52, 189, 243, 116,  63, 202, 
       19, 120, 237,   3, 142, 173,  23, 241, 
      156, 187,  53, 111, 182,  91, 242, 164, 
      143,  26, 126, 219,  77, 133, 193, 251, 
       36, 144, 233,  82,  29, 156, 226, 143, 
      249,  97, 156,  69, 225, 107, 194, 135, 
       63,  96, 218, 145,  65, 214,  32,  79, 
      176, 254,  89, 184,  37, 231,  25,  69, 
      108, 164,  11, 182, 130,  47,  91,  14, 
       73, 179,  30, 188,  44,  78,  10, 211, 
       42, 128,  27, 237,  12, 158, 123,  56, 
        8, 110,  50, 148, 203,  96, 161, 129, 
      205, 224,  59, 102, 254, 169, 213, 190, 
      128,  56, 215, 125, 250, 147, 231,  93, 
      180, 254, 165,  83, 187, 101, 232, 191, 
      222, 160, 212,  12, 119,  61, 239,   4, 
       81,  41, 124, 199,  72,   0, 109,  38, 
      154, 235, 103,  20, 170, 114,  33, 158, 
       71,   9, 107, 207,  40, 143,  22,  86, 
      130,  36,  73, 247, 174,  32, 195, 145, 
      181, 237, 153,  28, 217, 138, 238,  80, 
      200,   7, 147,  85,  64, 208,  55, 198, 
      125, 221,  52, 134,  72, 247, 202,  59, 
      242, 104, 198, 136,  88, 220, 112,  46, 
       98,  15, 175,  94,  54, 165,  22, 120, 
      172,  49, 210, 243, 181,   1, 236, 104, 
       21, 183, 155, 227,   4, 171, 113, 153, 
      183,   2, 166,  51,  20, 157,  75, 250, 
      201, 121,  74, 246, 192, 101, 223,  65, 
      252,  93, 116,  31, 127, 155,  87, 144, 
      244,  79,  33, 117, 190,  90,  46,  24, 
       69,  94, 226, 118, 238, 188,   8, 133, 
       60, 229,  39, 133,  16, 152,  44, 181, 
      141,  12, 163, 196,  72, 226,  42, 175, 
       58, 211,  98, 251,  62, 139, 236, 211, 
      134, 201,  35, 154,  67, 102, 213, 168, 
       25, 150, 181, 216,  85, 205, 124,  24, 
      226,  81, 214,  50, 106,  18, 202, 118, 
        6, 135, 162,  16, 203,  31, 165, 108, 
       51, 243,  82, 192,  27, 141,  43,  84, 
      222, 101,  10, 113,  55, 241,  70, 194, 
      112,  38, 146, 239, 184, 138, 253,  76, 
      193, 234,  51, 179, 122,  73, 220,  13, 
      142, 168,   9, 126, 249, 175, 229, 119, 
      196,  67, 254, 143, 174,   2, 158,  92, 
      248, 169,  66,   4,  89, 165,  26, 101, 
      152,  35,  81, 105, 240, 154,  95, 186, 
       66, 219, 111, 210,  60,  95,   1,  52, 
      159,  28, 189,  41,  89, 211, 129,  48, 
       13, 200, 131, 223, 119,  45, 219,  61, 
      209, 124, 186, 212,   0,  45, 229,  28, 
      254,  91,  43, 164,  24, 151, 204, 245, 
      133,  99, 232, 120, 224,  30, 186, 235, 
      150, 102,  25, 187,  75, 242, 132, 175, 
        8, 249,  27, 146,  64, 132, 197, 116, 
        5, 146, 182,  76, 237, 124,  70, 177, 
       13,  77, 171,  59, 149,  80, 110,  63, 
      216,  82, 232,  53, 152,  15, 199,  85, 
      113,  54, 164,  96, 244, 176,  78, 161, 
       60, 198,  21, 220,  97, 193,  37, 106, 
      217, 142,  24, 201,   7, 250, 178,  18, 
      139,  42, 172, 126, 210, 107,  39, 160, 
      227, 195,  79, 221,  14, 107,  35, 213, 
       94, 238, 117, 137,  50,   6, 148, 236, 
       57, 184, 245, 105, 161, 127,  39, 206, 
      163, 112, 254,  27,  88, 178, 247,  65, 
       18, 123,  34, 140, 204,  56, 230, 130, 
      173,  41,  70, 159, 251, 206, 170,  83, 
      119,  32,  88,  46, 222,  68,  97, 238, 
       77,  11, 194,  66, 223,   5, 134, 100, 
      151, 239, 173,  70, 117, 184, 149,  19 };
 
#define HALFTONE(c, h) (((c) > (h)) ? white_pix : black_pix)

void VidGrey_MSB1bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    int w, z, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line;
    uint8 *yp, *xip, row, black_pix=0, white_pix=1, *h=halftone;
    uint8 *greymap=vidPtr->image->greymap;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = (uint8 *) &vidPtr->ximage->image->data[y*2*ximagewidth+x/4];
	while (height-- > 0) {
	    for (w=0; w < width; w += 4) {
		z = (y % 16) * 64 + ((x+w) % 16) * 2;
		row = HALFTONE(greymap[*yp], h[z]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h[z+1]);
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+2]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h[z+3]);
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+4]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h[z+5]);
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+6]);
		*xip++ = (row<<1) + HALFTONE(greymap[*yp++], h[z+7]);
	    }
	    yp -= width;
	    xip += ximagewidth-width/4;

	    for (w=0; w < width; w += 4) {
		z = (y % 16) * 64 + 32 + ((x+w) % 16) * 2;
		row = HALFTONE(greymap[*yp], h[z]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h[z+1]);
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+2]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h[z+3]);
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+4]);
		row = (row<<1) + HALFTONE(greymap[*yp++], h[z+5]);
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+6]);
		*xip++ = (row<<1) + HALFTONE(greymap[*yp++], h[z+7]);
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth-width/4;
	    y++;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = (uint8 *) &vidPtr->ximage->image->data
		[(y >> scalebits)*ximagewidth + (x >> (scalebits+3))];
	while (height > 0) {
	    for (w=0; w < width; w += 8*scaler) {
		z = ((y >> scalebits) % 32) * 32 + (((x+w) >> scalebits) % 32);
		row = HALFTONE(greymap[*yp], h[z]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+1]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+2]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+3]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+4]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+5]);
		yp += scaler;
		row = (row<<1) + HALFTONE(greymap[*yp], h[z+6]);
		yp += scaler;
		*xip++ = (row<<1) + HALFTONE(greymap[*yp], h[z+7]);
		yp += scaler;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> (scalebits+3));
	    y += scaler;

	    height -= scaler;
	}
    }
}

void VidGrey_LSB1bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    int w, z, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line;
    uint8 *yp, *xip, row, black_pix=0, white_pix=128, *h=halftone;
    uint8 *greymap=vidPtr->image->greymap;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = (uint8 *) &vidPtr->ximage->image->data[y*2*ximagewidth+x/4];
	while (height-- > 0) {
	    for (w=0; w < width; w += 4) {
		z = (y % 16) * 64 + ((x+w) % 16) * 2;
		row = HALFTONE(greymap[*yp], h[z]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h[z+1]);
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+2]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h[z+3]);
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+4]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h[z+5]);
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+6]);
		*xip++ = (row>>1) + HALFTONE(greymap[*yp++], h[z+7]);
	    }
	    yp -= width;
	    xip += ximagewidth-width/4;

	    for (w=0; w < width; w += 4) {
		z = (y % 16) * 64 + 32 + ((x+w) % 16) * 2;
		row = HALFTONE(greymap[*yp], h[z]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h[z+1]);
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+2]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h[z+3]);
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+4]);
		row = (row>>1) + HALFTONE(greymap[*yp++], h[z+5]);
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+6]);
		*xip++ = (row>>1) + HALFTONE(greymap[*yp++], h[z+7]);
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth-width/4;
	    y++;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = (uint8 *) &vidPtr->ximage->image->data
		[(y >> scalebits)*ximagewidth + (x >> (scalebits+3))];
	while (height > 0) {
	    for (w=0; w < width; w += 8*scaler) {
		z = ((y >> scalebits) % 32) * 32 + (((x+w) >> scalebits) % 32);
		row = HALFTONE(greymap[*yp], h[z]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+1]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+2]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+3]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+4]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+5]);
		yp += scaler;
		row = (row>>1) + HALFTONE(greymap[*yp], h[z+6]);
		yp += scaler;
		*xip++ = (row>>1) + HALFTONE(greymap[*yp], h[z+7]);
		yp += scaler;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> (scalebits+3));
	    y += scaler;

	    height -= scaler;
	}
    }
}

void VidGrey_MSB8bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line/4;
    uint8 y0, y1, y2, *yp, *greymap=vidPtr->image->greymap, *dith=y_dither8;
    uint32 *xip, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[(y*2+1)*ximagewidth+x/2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		out = dith[greymap[(y1+y0)/2]*16+z];
		out = (out << 8) + dith[greymap[y0]*16+z+1];

		y1 = yp[1];
		out = (out << 8) + dith[greymap[(y0+y1)/2]*16+z+2];
		xip[0] = (out << 8) + dith[greymap[y1]*16+z+3];

		y0 = yp[2];
		out = dith[greymap[(y1+y0)/2]*16+z];
		out = (out << 8) + dith[greymap[y0]*16+z+1];

		y1 = yp[3];
		out = (out << 8) + dith[greymap[(y0+y1)/2]*16+z+2];
		xip[1] = (out << 8) + dith[greymap[y1]*16+z+3];

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
		    out = dith[greymap[(y1+y0)/2]*16+z];
		    out = (out << 8) + dith[greymap[y0]*16+z+1];

		    y1 = yp[1];
		    out = (out << 8) + dith[greymap[(y0+y1)/2]*16+z+2];
		    xip[0] = (out << 8) + dith[greymap[y1]*16+z+3];

		    y0 = yp[2];
		    out = dith[greymap[(y1+y0)/2]*16+z];
		    out = (out << 8) + dith[greymap[y0]*16+z+1];

		    y1 = yp[3];
		    out = (out << 8) + dith[greymap[(y0+y1)/2]*16+z+2];
		    xip[1] = (out << 8) + dith[greymap[y1]*16+z+3];

		    yp += 4;
		    xip += 2;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    out = dith[greymap[(y0+y2)/2]*16+z];
		    out = (out << 8) + dith[greymap[(y1+y2)/2]*16+z+1];

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    out = (out << 8) + dith[greymap[(y1+y2)/2]*16+z+2];
		    xip[0] = (out << 8) + dith[greymap[(y0+y2)/2]*16+z+3];

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    out = dith[greymap[(y0+y2)/2]*16+z];
		    out = (out << 8) + dith[greymap[(y1+y2)/2]*16+z+1];

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    out = (out << 8) + dith[greymap[(y1+y2)/2]*16+z+2];
		    xip[1] = (out << 8) + dith[greymap[(y0+y2)/2]*16+z+3];

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
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+2))];
	z = ((y >> scalebits) % 4) * 4;
	while (height > 0) {
	    for (w=width; w > 0; w -= 8*scaler) {
		out = dith[greymap[yp[0]]*16+z];
		out = (out << 8) + dith[greymap[yp[scaler]]*16+z+1];
		yp += 2*scaler;

		out = (out << 8) + dith[greymap[yp[0]]*16+z+2];
		xip[0] = (out << 8) + dith[greymap[yp[scaler]]*16+z+3];
		yp += 2*scaler;

		out = dith[greymap[yp[0]]*16+z];
		out = (out << 8) + dith[greymap[yp[scaler]]*16+z+1];
		yp += 2*scaler;

		out = (out << 8) + dith[greymap[yp[0]]*16+z+2];
		xip[1] = (out << 8) + dith[greymap[yp[scaler]]*16+z+3];
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
    int w, z, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line/4;
    uint8 y0, y1, y2, *yp, *greymap=vidPtr->image->greymap, *dith=y_dither8;
    uint32 *xip, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[(y*2+1)*ximagewidth+x/2];
	z = (y % 2) * 8;
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		out = dith[greymap[(y1+y0)/2]*16+z];
		out += (dith[greymap[y0]*16+z+1] << 8);

		y1 = yp[1];
		out += (dith[greymap[(y0+y1)/2]*16+z+2] << 16);
		xip[0] = out + (dith[greymap[y1]*16+z+3] << 24);

		y0 = yp[2];
		out = dith[greymap[(y1+y0)/2]*16+z];
		out += (dith[greymap[y0]*16+z+1] << 8);

		y1 = yp[3];
		out += (dith[greymap[(y0+y1)/2]*16+z+2] << 16);
		xip[1] = out + (dith[greymap[y1]*16+z+3] << 24);

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
		    out = dith[greymap[(y1+y0)/2]*16+z];
		    out += (dith[greymap[y0]*16+z+1] << 8);

		    y1 = yp[1];
		    out += (dith[greymap[(y0+y1)/2]*16+z+2] << 16);
		    xip[0] = out + (dith[greymap[y1]*16+z+3] << 24);

		    y0 = yp[2];
		    out = dith[greymap[(y1+y0)/2]*16+z];
		    out += (dith[greymap[y0]*16+z+1] << 8);

		    y1 = yp[3];
		    out += (dith[greymap[(y0+y1)/2]*16+z+2] << 16);
		    xip[1] = out + (dith[greymap[y1]*16+z+3] << 24);

		    yp += 4;
		    xip += 2;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    out = dith[greymap[(y0+y2)/2]*16+z];
		    out += (dith[greymap[(y1+y2)/2]*16+z+1] << 8);

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    out += (dith[greymap[(y1+y2)/2]*16+z+2] << 16);
		    xip[0] = out + (dith[greymap[(y0+y2)/2]*16+z+3] << 24);

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    out = dith[greymap[(y0+y2)/2]*16+z];
		    out += (dith[greymap[(y1+y2)/2]*16+z+1] << 8);

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    out += (dith[greymap[(y1+y2)/2]*16+z+2] << 16);
		    xip[1] = out + (dith[greymap[(y0+y2)/2]*16+z+3] << 24);

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
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+2))];
	z = ((y >> scalebits) % 4) * 4;
	while (height > 0) {
	    for (w=width; w > 0; w -= 8*scaler) {
		out = dith[greymap[yp[0]]*16+z];
		out += (dith[greymap[yp[scaler]]*16+z+1] << 8);
		yp += 2*scaler;

		out += (dith[greymap[yp[0]]*16+z+2] << 16);
		xip[0] = out + (dith[greymap[yp[scaler]]*16+z+3] << 24);
		yp += 2*scaler;

		out = dith[greymap[yp[0]]*16+z];
		out += (dith[greymap[yp[scaler]]*16+z+1] << 8);
		yp += 2*scaler;

		out += (dith[greymap[yp[0]]*16+z+2] << 16);
		xip[1] = out + (dith[greymap[yp[scaler]]*16+z+3] << 24);
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
    int w, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line/4;
    uint8 y0, y1, y2, *yp, *greymap=vidPtr->image->greymap;
    uint32 *cmap=y_cmap, *xip, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[(y*2+1)*ximagewidth + x];
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		out = cmap[greymap[(y1+y0)/2]];
		xip[0] = (out << 16) + cmap[greymap[y0]];

		y1 = yp[1];
		out = cmap[greymap[(y0+y1)/2]];
		xip[1] = (out << 16) + cmap[greymap[y1]];

		y0 = yp[2];
		out = cmap[greymap[(y1+y0)/2]];
		xip[2] = (out << 16) + cmap[greymap[y0]];

		y1 = yp[3];
		out = cmap[greymap[(y0+y1)/2]];
		xip[1] = (out << 16) + cmap[greymap[y1]];

		yp += 4;
		xip += 4;
	    }
	    yp -= width;
	    xip -= (ximagewidth+width);

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    out = cmap[greymap[(y1+y0)/2]];
		    xip[0] = (out << 16) + cmap[greymap[y0]];

		    y1 = yp[1];
		    out = cmap[greymap[(y0+y1)/2]];
		    xip[1] = (out << 16) + cmap[greymap[y1]];

		    y0 = yp[2];
		    out = cmap[greymap[(y1+y0)/2]];
		    xip[2] = (out << 16) + cmap[greymap[y0]];

		    y1 = yp[3];
		    out = cmap[greymap[(y0+y1)/2]];
		    xip[3] = (out << 16) + cmap[greymap[y1]];

		    yp += 4;
		    xip += 4;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    out = cmap[greymap[(y0+y2)/2]];
		    xip[0] = (out << 16) + cmap[greymap[(y1+y2)/2]];

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    out = cmap[greymap[(y1+y2)/2]];
		    xip[1] = (out << 16) + cmap[greymap[(y0+y2)/2]];

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    out = cmap[greymap[(y0+y2)/2]];
		    xip[2] = (out << 16) + cmap[greymap[(y1+y2)/2]];

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    out = cmap[greymap[(y1+y2)/2]];
		    xip[3] = (out << 16) + cmap[greymap[(y0+y2)/2]];

		    yp += 4;
		    xip += 4;
		}
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth*3-width;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+1))];
	while (height > 0) {
	    for (w=width; w > 0; w -= 8*scaler) {
		out = cmap[greymap[yp[0]]];
		xip[0] = (out << 16) + cmap[greymap[yp[scaler]]];
		yp += 2*scaler;

		out = cmap[greymap[yp[0]]];
		xip[1] = (out << 16) + cmap[greymap[yp[scaler]]];
		yp += 2*scaler;

		out = cmap[greymap[yp[0]]];
		xip[2] = (out << 16) + cmap[greymap[yp[scaler]]];
		yp += 2*scaler;

		out = cmap[greymap[yp[0]]];
		xip[3] = (out << 16) + cmap[greymap[yp[scaler]]];
		yp += 2*scaler;
		xip += 4;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> (scalebits+1));

	    height -= scaler;
	}
    }
}

void VidGrey_LSB16bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    int w, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line/4;
    uint8 y0, y1, y2, *yp, *greymap=vidPtr->image->greymap;
    uint32 *cmap=y_cmap, *xip, out;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[(y*2+1)*ximagewidth + x];
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		out = cmap[greymap[(y1+y0)/2]];
		xip[0] = out + (cmap[greymap[y0]] << 16);

		y1 = yp[1];
		out = cmap[greymap[(y0+y1)/2]];
		xip[1] = out + (cmap[greymap[y1]] << 16);

		y0 = yp[2];
		out = cmap[greymap[(y1+y0)/2]];
		xip[2] = out + (cmap[greymap[y0]] << 16);

		y1 = yp[3];
		out = cmap[greymap[(y0+y1)/2]];
		xip[3] = out + (cmap[greymap[y1]] << 16);

		yp += 4;
		xip += 4;
	    }
	    yp -= width;
	    xip -= (ximagewidth+width);

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    out = cmap[greymap[(y1+y0)/2]];
		    xip[0] = out + (cmap[greymap[y0]] << 16);

		    y1 = yp[1];
		    out = cmap[greymap[(y0+y1)/2]];
		    xip[1] = out + (cmap[greymap[y1]] << 16);

		    y0 = yp[2];
		    out = cmap[greymap[(y1+y0)/2]];
		    xip[2] = out + (cmap[greymap[y0]] << 16);

		    y1 = yp[3];
		    out = cmap[greymap[(y0+y1)/2]];
		    xip[3] = out + (cmap[greymap[y1]] << 16);

		    yp += 4;
		    xip += 4;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    out = cmap[greymap[(y0+y2)/2]];
		    xip[0] = out + (cmap[greymap[(y1+y2)/2]] << 16);

		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    out = cmap[greymap[(y1+y2)/2]];
		    xip[1] = out + (cmap[greymap[(y0+y2)/2]] << 16);

		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    out = cmap[greymap[(y0+y2)/2]];
		    xip[2] = out + (cmap[greymap[(y1+y2)/2]] << 16);

		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    out = cmap[greymap[(y1+y2)/2]];
		    xip[3] = out + (cmap[greymap[(y0+y2)/2]] << 16);

		    yp += 4;
		    xip += 4;
		}
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth*3-width;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y >> scalebits)*ximagewidth + (x >> (scalebits+1))];
	while (height > 0) {
	    for (w=width; w > 0; w -= 8*scaler) {
		out = cmap[greymap[yp[0]]];
		xip[0] = out + (cmap[greymap[yp[scaler]]] << 16);
		yp += 2*scaler;

		out = cmap[greymap[yp[0]]];
		xip[1] = out + (cmap[greymap[yp[scaler]]] << 16);
		yp += 2*scaler;

		out = cmap[greymap[yp[0]]];
		xip[2] = out + (cmap[greymap[yp[scaler]]] << 16);
		yp += 2*scaler;

		out = cmap[greymap[yp[0]]];
		xip[3] = out + (cmap[greymap[yp[scaler]]] << 16);
		yp += 2*scaler;
		xip += 4;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> (scalebits+1));

	    height -= scaler;
	}
    }
}

void VidGrey_24bit(vidwidget_t *vidPtr, int x, int y, int width, int height)
{
    int w, yy, scalebits=vidPtr->scalebits, scaler=1<<scalebits;
    int imagewidth=vidPtr->image->width;
    int ximagewidth=vidPtr->ximage->image->bytes_per_line/4;
    uint8 y0, y1, y2, *yp, *greymap=vidPtr->image->greymap;
    uint32 *cmap=y_cmap, *xip;
 
    if (scalebits == -1) {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)[(y*2+1)*ximagewidth+x*2];
	for (yy=y; yy<y+height; yy++) {
	    y1 = yp[0];
	    for (w=width; w > 0; w -= 4) {
		y0 = yp[0];
		xip[0] = cmap[greymap[(y1+y0)/2]];
		xip[1] = cmap[greymap[y0]];
		y1 = yp[1];
		xip[2] = cmap[greymap[(y0+y1)/2]];
		xip[3] = cmap[greymap[y1]];
		y0 = yp[2];
		xip[4] = cmap[greymap[(y1+y0)/2]];
		xip[5] = cmap[greymap[y0]];
		y1 = yp[3];
		xip[6] = cmap[greymap[(y0+y1)/2]];
		xip[7] = cmap[greymap[y1]];
		yp += 4;
		xip += 8;
	    }
	    yp -= width;
	    xip -= (ximagewidth+width*2);

	    if (yy == y) {
		y1 = yp[0];
		for (w=width; w > 0; w -= 4) {
		    y0 = yp[0];
		    xip[0] = cmap[greymap[(y1+y0)/2]];
		    xip[1] = cmap[greymap[y0]];
		    y1 = yp[1];
		    xip[2] = cmap[greymap[(y0+y1)/2]];
		    xip[3] = cmap[greymap[y1]];
		    y0 = yp[2];
		    xip[4] = cmap[greymap[(y1+y0)/2]];
		    xip[5] = cmap[greymap[y0]];
		    y1 = yp[3];
		    xip[6] = cmap[greymap[(y0+y1)/2]];
		    xip[7] = cmap[greymap[y1]];
		    yp += 4;
		    xip += 8;
		}
	    } else {
		y0 = yp[-imagewidth];
		for (w=width; w > 0; w -= 4) {
		    y1 = yp[-imagewidth];
		    y2 = yp[0];
		    xip[0] = cmap[greymap[(y0+y2)/2]];
		    xip[1] = cmap[greymap[(y1+y2)/2]];
		    y0 = yp[-imagewidth+1];
		    y2 = yp[1];
		    xip[2] = cmap[greymap[(y1+y2)/2]];
		    xip[3] = cmap[greymap[(y0+y2)/2]];
		    y1 = yp[-imagewidth+2];
		    y2 = yp[2];
		    xip[4] = cmap[greymap[(y0+y2)/2]];
		    xip[5] = cmap[greymap[(y1+y2)/2]];
		    y0 = yp[-imagewidth+3];
		    y2 = yp[3];
		    xip[6] = cmap[greymap[(y1+y2)/2]];
		    xip[7] = cmap[greymap[(y0+y2)/2]];
		    yp += 4;
		    xip += 8;
		}
	    }
	    yp += imagewidth-width;
	    xip += ximagewidth*3-width*2;
	}
    } else {
	yp = &vidPtr->image->y_data[y*imagewidth+x];
	xip = &((uint32 *)vidPtr->ximage->image->data)
		  [(y >> scalebits)*ximagewidth + (x >> scalebits)];
	while (height > 0) {
	    for (w=width; w > 0; w -= 8*scaler) {
		xip[0] = cmap[greymap[yp[0]]];
		xip[1] = cmap[greymap[yp[scaler]]];
		yp += 2*scaler;
		xip[2] = cmap[greymap[yp[0]]];
		xip[3] = cmap[greymap[yp[scaler]]];
		yp += 2*scaler;
		xip[4] = cmap[greymap[yp[0]]];
		xip[5] = cmap[greymap[yp[scaler]]];
		yp += 2*scaler;
		xip[6] = cmap[greymap[yp[0]]];
		xip[7] = cmap[greymap[yp[scaler]]];
		yp += 2*scaler;
		xip += 8;
	    }
	    yp += (imagewidth << scalebits)-width;
	    xip += ximagewidth-(width >> scalebits);

	    height -= scaler;
	}
    }
}
