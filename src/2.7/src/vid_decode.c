/*
	Netvideo version 2.7
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

#include "video.h"
#include "vidcode.h"

static void VidDecode_DoQtrCol(ip, width, x)
    register unsigned char *ip;
    register int width, x;
{
    register int q0, q1, q2, diff;

    q0 = ip[x];
    q1 = ip[3*width+x];
    q2 = ip[7*width+x];
    diff = q1-q0;
    ip[width+x] = q0+diff/4;
    ip[2*width+x] = q1-diff/4;
    diff = q2-q1;
    ip[4*width+x] = q1+diff/4;
    ip[5*width+x] = q1+diff/2;
    ip[6*width+x] = q2-diff/4;
}

static void VidDecode_DoQtrBlock(ip, width, initpix, data)
    register unsigned char *ip;
    register int width, initpix;
    unsigned char *data;
{
    register int y, pix, q0, q1, q2, diff;
    register int lineskip=width-6;
    register unsigned long inp;

    inp = *(unsigned long *)data;

    q0 = ip[0] = pix = initpix;
    ip[3] = ADDPIX(pix, inp >> 28);
    ip[7] = ADDPIX(pix, (inp>>24) & 0xf);
    pix = q0;
    q1 = ip[3*width] = ADDPIX(pix, (inp>>20) & 0xf);
    ip[3*width+3] = ADDPIX(pix, (inp>>16) & 0xf);
    ip[3*width+7] = ADDPIX(pix, (inp>>12) & 0xf);
    pix = q1;
    q2 = ip[7*width] = ADDPIX(pix, (inp>>8) & 0xf);
    ip[7*width+3] = ADDPIX(pix, (inp>>4) & 0xf);
    ip[7*width+7] = ADDPIX(pix, inp & 0xf);

    VidDecode_DoQtrCol(ip, width, 0);
    VidDecode_DoQtrCol(ip, width, 3);
    VidDecode_DoQtrCol(ip, width, 7);

    for (y=0; y<8; y++) {
	q0 = ip[0];
	q1 = ip[3];
	q2 = ip[7];
	diff = q1-q0;
	*++ip = q0+diff/4;
	*++ip = q1-diff/4;
	ip++;
	diff = q2-q1;
	*++ip = q1+diff/4;
	*++ip = q1+diff/2;
	*++ip = q2-diff/4;
	ip += lineskip;
    }
}

static void VidDecode_DoHalfRow(ip, pixp, inp)
    register unsigned char *ip;
    register int *pixp;
    register unsigned short inp;
{
    register int pix;

    pix = *pixp;
    ip[0] = ADDPIX(pix, inp >> 12);
    *pixp = pix;
    ip[2] = ADDPIX(pix, (inp>>8) & 0xf);
    ip[5] = ADDPIX(pix, (inp>>4) & 0xf);
    ip[7] = ADDPIX(pix, inp & 0xf);
}

static void VidDecode_DoHalfCol(ip, width, x)
    register unsigned char *ip;
    register int width, x;
{
    register int h0, h1, h2, h3, diff;

    h0 = ip[x];
    h1 = ip[2*width+x];
    h2 = ip[5*width+x];
    h3 = ip[7*width+x];
    diff = h1-h0;
    ip[width+x] = h0+diff/2;
    diff = h2-h1;
    ip[3*width+x] = h1+diff/4;
    ip[4*width+x] = h2-diff/4;
    diff = h3-h2;
    ip[6*width+x] = h2+diff/2;
}

static void VidDecode_DoHalfBlock(ip, width, initpix, data)
    register unsigned char *ip;
    register int width;
    int initpix;
    register unsigned char *data;
{
    register int y, h0, h1, h2, h3, diff;
    register int lineskip=width-6;
    register unsigned short *inp=(unsigned short *)data;

    VidDecode_DoHalfRow(ip, &initpix, inp[0]);
    VidDecode_DoHalfRow(ip+2*width, &initpix, inp[1]);
    VidDecode_DoHalfRow(ip+5*width, &initpix, inp[2]);
    VidDecode_DoHalfRow(ip+7*width, &initpix, inp[3]);

    VidDecode_DoHalfCol(ip, width, 0);
    VidDecode_DoHalfCol(ip, width, 2);
    VidDecode_DoHalfCol(ip, width, 5);
    VidDecode_DoHalfCol(ip, width, 7);

    for (y=0; y<8; y++) {
	h0 = ip[0];
	h1 = ip[2];
	h2 = ip[5];
	h3 = ip[7];
	diff = h1-h0;
	*++ip = h0+diff/2;
	ip++;
	diff = h2-h1;
	*++ip = h1+diff/4;
	*++ip = h2-diff/4;
	ip++;
	diff = h3-h2;
	*++ip = h2+diff/2;
	ip += lineskip;
    }
}

static void VidDecode_DoFullBlock(ip, width, initpix, data)
    register unsigned char *ip;
    register int width, initpix;
    unsigned char *data;
{
    register int x, y, lineskip=width-8, pix;
    register unsigned long *inp=(unsigned long *)data, row;

    pix = initpix;
    for (y=0; y<8; y++) {
	row = inp[y];
	*ip++ = ADDPIX(pix, row >> 28);
	*ip++ = ADDPIX(pix, (row>>24) & 0xf);
	*ip++ = ADDPIX(pix, (row>>20) & 0xf);
	*ip++ = ADDPIX(pix, (row>>16) & 0xf);
	*ip++ = ADDPIX(pix, (row>>12) & 0xf);
	*ip++ = ADDPIX(pix, (row>>8) & 0xf);
	*ip++ = ADDPIX(pix, (row>>4) & 0xf);
	*ip++ = ADDPIX(pix, row & 0xf);
	pix = *(ip-8);
	ip += lineskip;
    }
}

static int VidDecode_DoBlock(image, data, maxlen, endp)
    vidimage_t *image;
    unsigned char *data;
    int maxlen;
    int *endp;
{
    int i, maxnamelen;
    vidcode_hdr_t *hdr=(vidcode_hdr_t *)data;
    register unsigned char *ip;

    data += VIDCODE_HDR_LEN;
    maxlen -= VIDCODE_HDR_LEN;

    if ((hdr->protvers != VIDCODE_VERSION) || (hdr->type > VIDCODE_FRAME_END) ||
	(hdr->x >= image->width/8) || (hdr->y >= image->height/8)) goto bogus;

    ip = &image->data[hdr->y*8*image->width+hdr->x*8];
    switch (hdr->type) {
    case VIDCODE_QTRBLOCK:
	if (maxlen < VIDCODE_QTRBLOCK_LEN-VIDCODE_HDR_LEN) goto bogus;

	VidDecode_DoQtrBlock(ip, image->width, hdr->initpix, data);
	VidImage_UpdateRect(image, hdr->x*8, hdr->y*8, 8, 8);
	return VIDCODE_QTRBLOCK_LEN;
    case VIDCODE_HALFBLOCK:
	if (maxlen < VIDCODE_HALFBLOCK_LEN-VIDCODE_HDR_LEN) goto bogus;

	VidDecode_DoHalfBlock(ip, image->width, hdr->initpix, data);
	VidImage_UpdateRect(image, hdr->x*8, hdr->y*8, 8, 8);
	return VIDCODE_HALFBLOCK_LEN;
    case VIDCODE_FULLBLOCK:
	if (maxlen < VIDCODE_FULLBLOCK_LEN-VIDCODE_HDR_LEN) goto bogus;

	VidDecode_DoFullBlock(ip, image->width, hdr->initpix, data);
	VidImage_UpdateRect(image, hdr->x*8, hdr->y*8, 8, 8);
	return VIDCODE_FULLBLOCK_LEN;
    case VIDCODE_FRAME_END:
	VidImage_Redraw(image);
	*endp = 1;
	return VIDCODE_FRAME_END_LEN;
    }

bogus:
    return -1;
}

void VidDecode_Init()
{
    VidDiff_Init();
}

int VidDecode(image, data, len)
    vidimage_t *image;
    unsigned char *data;
    int len;
{
    int blocklen=0, end=0;
    unsigned char *p;

    for (p=data; (p < data+len) && (blocklen != -1); p += blocklen)
	blocklen = VidDecode_DoBlock(image, p, data+len-p, &end);

    return end;
}
