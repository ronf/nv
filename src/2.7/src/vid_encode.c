/*
	Netvideo version 2.7
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video encode routines
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
#include "video.h"
#include "vidcode.h"
#include "vidgrab.h"

#define VIDENCODE_THRESHOLD	48

typedef struct {
    unsigned short loc, pri;
} encodeheap_t;

/* HACK: This is exported to allow the name sending code to match the old
	 NV packet format. When the format changes, this will go away. */
int vidcode_fmt;

static int width, height, block_width, block_height;
static unsigned char *last_image;
static short *age;

static char *videncode_buf;
static int videncode_len;
static videncode_proc_t *videncode_callback;

static encodeheap_t *encodeheap;
static unsigned short *encodeheaploc, encodeheaplen;

char vidcode_to_diff[NUM_VIDCODES] = 
    { 0, 1, 2, 3, 4, 8, 16, 32, 64, 96, 112, 120, 124, 125, 126, 127 };
char viddiff_to_code[GREY_LEVELS*2];

static void EncodeHeap_FixUp(i)
    int i;
{
    register int l=2*i, r=2*i+1, largest;
    encodeheap_t temp;

    largest = ((l <= encodeheaplen) &&
	(encodeheap[l].pri > encodeheap[i].pri)) ? l : i;
    if ((r <= encodeheaplen) && (encodeheap[r].pri > encodeheap[largest].pri))
	largest = r;

    if (largest != i) {
	temp = encodeheap[i];
	encodeheap[i] = encodeheap[largest];
	encodeheap[largest] = temp;
	encodeheaploc[encodeheap[i].loc] = i;
	encodeheaploc[encodeheap[largest].loc] = largest;
	EncodeHeap_FixUp(largest);
    }
}

static unsigned short EncodeHeap_DeleteMax()
{
    int loc = encodeheap[1].loc;

    encodeheaploc[loc] = 0;
    encodeheap[1] = encodeheap[encodeheaplen--];
    if (encodeheaplen > 0) {
	encodeheaploc[encodeheap[1].loc] = 1;
	EncodeHeap_FixUp(1);
    }
    return loc;
}

static void EncodeHeap_Insert(loc, pri)
    unsigned short loc, pri;
{
    register int i;

    if ((i = encodeheaploc[loc]) != 0) {
	if (encodeheap[i].pri > pri) return;
	encodeheap[i] = encodeheap[encodeheaplen--];
	encodeheaploc[encodeheap[i].loc] = i;
	EncodeHeap_FixUp(i);
    }

    i = ++encodeheaplen;
    while ((i > 1) && (encodeheap[i/2].pri < pri)) {
	encodeheap[i] = encodeheap[i/2];
	encodeheaploc[encodeheap[i].loc] = i;
	i /= 2;
    }

    encodeheap[i].loc = loc;
    encodeheap[i].pri = pri;
    encodeheaploc[loc] = i;
}

static void EncodeHeap_Empty()
{
    register int i;

    for (i=1; i<=encodeheaplen; i++) encodeheaploc[encodeheap[i].loc] = 0;
    encodeheaplen = 0;
}

static VidEncode_PutBlock(type, x, y, initpix, data, len)
    int type;
    char *data;
    int len;
{
    static char *bufp=NULL;
    register vidcode_hdr_t *hdr;
    register int total_len=VIDCODE_HDR_LEN+len;

    if (bufp == NULL) bufp = videncode_buf;

    if (bufp-videncode_buf > videncode_len-total_len) {
	videncode_callback(videncode_buf, bufp-videncode_buf);
	bufp = videncode_buf;
    }

    hdr = (vidcode_hdr_t *) bufp;
    hdr->protvers = VIDCODE_VERSION;
    hdr->fmt = vidcode_fmt;
    hdr->type = type;
    hdr->x = x;
    hdr->y = y;
    hdr->initpix = initpix;
    bufp += VIDCODE_HDR_LEN;

    if (len > 0) {
	bcopy(data, bufp, len);
	bufp += len;
    }

    if (type == VIDCODE_FRAME_END) {
	videncode_callback(videncode_buf, bufp-videncode_buf);
	bufp = NULL;
    }

    return total_len;
}

static VidEncode_ImageDiff(block1, block2)
    unsigned char block1[8], block2[8];
{
    register int total;

#define	BLOCKDIFF(x)	(block1[x]-block2[x])
    total = BLOCKDIFF(0) + BLOCKDIFF(1) + BLOCKDIFF(2) + BLOCKDIFF(3)
	  + BLOCKDIFF(4) + BLOCKDIFF(5) + BLOCKDIFF(6) + BLOCKDIFF(7);
#undef	BLOCKDIFF

    return (total >= 0) ? total : -total;
}

static VidEncode_FindDiffs(old, new, xmit_threshold, do_aging)
    unsigned char *old, *new;
    int xmit_threshold, do_aging;
{
    static int scan_cycle=0;
    register int h=block_height-1, w=block_width-1;
    register int x, y, loc=0, offset, pri;
    register unsigned char *oldp, *newp;

    EncodeHeap_Empty();

    offset = scan_cycle*width;
    oldp = &old[offset];
    newp = &new[offset];
    for (y=0; y<=h; y++) {
	for (x=0; x<=w; x++) {
	    pri = VidEncode_ImageDiff(oldp, newp);
	    if (do_aging) pri += age[loc]++/2;

	    if (pri >= xmit_threshold) {
		if (y != 0) {
		    if (x != 0) EncodeHeap_Insert(loc-block_width-1, pri-8);
		    EncodeHeap_Insert(loc-block_width, pri-4);
		    if (x != w) EncodeHeap_Insert(loc-block_width+1, pri-8);
		}

		if (x != 0) EncodeHeap_Insert(loc-1, pri-4);
		EncodeHeap_Insert(loc, pri);
		if (x != w) EncodeHeap_Insert(loc+1, pri-4);

		if (y != h) {
		    if (x != 0) EncodeHeap_Insert(loc+block_width-1, pri-8);
		    EncodeHeap_Insert(loc+block_width, pri-4);
		    if (x != w) EncodeHeap_Insert(loc+block_width+1, pri-8);
		}
	    }

	    loc++;
	    oldp += 8;
	    newp += 8;
	}
	oldp += 8*(width-block_width);
	newp += 8*(width-block_width);
    }
    scan_cycle = (scan_cycle+1) % 8;
}

static VidEncode_CopyBlock(block1, block2)
    unsigned long *block1, *block2;
{
    register int i, lineskip=width/4;

    for (i=0; i<8; i++) {
	block2[0] = block1[0];
	block2[1] = block1[1];
	block1 += lineskip;
	block2 += lineskip;
    }
}

static VidEncode_PutQtrBlock(image, x0, y0)
    unsigned char *image;
    int x0, y0;
{
    register int x=x0*8, y=y0*8;
    register unsigned char initpix, pix0, pix, diff, *ip;
    static unsigned long out;

    VidEncode_CopyBlock((unsigned long *)&image[y*width+x],
	(unsigned long *)&last_image[y*width+x]);

    ip = &image[y*width+x];
    initpix = pix0 = pix = ip[0];
    out = PIXDIFF(pix, ip[3], diff);
    out = (out<<4) + PIXDIFF(pix, ip[7], diff);
    pix = pix0;
    ip += 3*width;
    out = (out<<4) + PIXDIFF(pix, ip[0], diff);
    pix0 = pix;
    out = (out<<4) + PIXDIFF(pix, ip[3], diff);
    out = (out<<4) + PIXDIFF(pix, ip[7], diff);
    pix = pix0;
    ip += 4*width;
    out = (out<<4) + PIXDIFF(pix, ip[0], diff);
    out = (out<<4) + PIXDIFF(pix, ip[3], diff);
    out = (out<<4) + PIXDIFF(pix, ip[7], diff);

    return VidEncode_PutBlock(VIDCODE_QTRBLOCK, x0, y0, initpix,
			      (char *)&out, sizeof(out));
}

static short VidEncode_DoHalfRow(ip, pixp)
    register unsigned char *ip;
    register unsigned char *pixp;
{
    register unsigned char pix = *pixp, diff;
    register short out;

    out = PIXDIFF(pix, *ip, diff);
    *pixp = pix;
    out = (out<<4) + PIXDIFF(pix, *(ip+2), diff);
    out = (out<<4) + PIXDIFF(pix, *(ip+5), diff);
    out = (out<<4) + PIXDIFF(pix, *(ip+7), diff);

    return out;
}

static VidEncode_PutHalfBlock(image, x0, y0)
    unsigned char *image;
    int x0, y0;
{
    register int x=x0*8, y=y0*8;
    unsigned char initpix, pix, *ip;
    static unsigned short out[4];

    VidEncode_CopyBlock((unsigned long *)&image[y*width+x],
	(unsigned long *)&last_image[y*width+x]);

    ip = &image[y*width+x];
    initpix = pix = ip[0];
    out[0] = VidEncode_DoHalfRow(ip, &pix);
    ip += 2*width;
    out[1] = VidEncode_DoHalfRow(ip, &pix);
    ip += 3*width;
    out[2] = VidEncode_DoHalfRow(ip, &pix);
    ip += 2*width;
    out[3] = VidEncode_DoHalfRow(ip, &pix);

    return VidEncode_PutBlock(VIDCODE_HALFBLOCK, x0, y0, initpix,
			      (char *)out, sizeof(out));
}

static VidEncode_PutFullBlock(image, x0, y0)
    unsigned char *image;
    int x0, y0;
{
    static unsigned long out[8];
    register int i, j, x=x0*8, y=y0*8;
    register unsigned char initpix, pix, diff, *ip;
    register unsigned long outrow;

    VidEncode_CopyBlock((unsigned long *)&image[y*width+x],
	(unsigned long *)&last_image[y*width+x]);

    ip = &image[y*width+x];
    initpix = pix = ip[0];
    for (i=0; i<8; i++) {
	outrow = 0;
	for (j=0; j<8; j++)
	    outrow = (outrow<<4) + PIXDIFF(pix, ip[j], diff);
	out[i] = outrow;
	pix = ip[0];
	ip += width;
    }

    return VidEncode_PutBlock(VIDCODE_FULLBLOCK, x0, y0, initpix,
			      (char *)out, sizeof(out));
}

static VidEncode_PutFrameEnd()
{
    return VidEncode_PutBlock(VIDCODE_FRAME_END, 0, 0, 0, NULL, 0);
}

void VidDiff_Init()
{
    int i, j=0;
    static int viddiff_initted=0;

    if (viddiff_initted) return;

    j = 1;
    for (i = -GREY_LEVELS; i<0; i++) {
	viddiff_to_code[i+GREY_LEVELS] = j;
	if (i == vidcode_to_diff[j]-128) j++;
    }

    j = NUM_VIDCODES-1;
    for (i=GREY_LEVELS; i>=0; i--) {
	viddiff_to_code[i+GREY_LEVELS] = j;
	if (i == vidcode_to_diff[j]) j--;
    }

    viddiff_initted = 1;
}

void VidEncode_Init(w, h)
    int w, h;
{
    VidDiff_Init();

    width = w;
    height = h;
    block_width = (width+7)/8;
    block_height = (height+7)/8;
    vidcode_fmt = (width == NTSC_WIDTH) ? VIDCODE_NTSC : VIDCODE_PAL;

    if (last_image != NULL) free(last_image);
    last_image = (unsigned char *) malloc(height*width);
    memset(last_image, GREY_LEVELS/2, height*width);

    if (age != NULL) free(age);
    age = (short *) malloc(block_height*block_width*sizeof(short));
    memset(age, 0, block_height*block_width*sizeof(short));

    if (encodeheap != NULL) free(encodeheap);
    encodeheap = (encodeheap_t *)
	malloc((block_height*block_width+1)*sizeof(encodeheap_t));
    encodeheaplen = 0;

    if (encodeheaploc != NULL) free(encodeheaploc);
    encodeheaploc = (unsigned short *)
	malloc(block_height*block_width*sizeof(unsigned short));
    memset(encodeheaploc, 0, block_height*block_width*sizeof(unsigned short));
}

void VidEncode_Reset()
{
    memset(last_image, GREY_LEVELS/2, height*width);
}

void VidEncode(xmitted_image, image, buf, buflen, max_bytes, max_aged, callback)
    unsigned char *xmitted_image, *image;
    char *buf;
    int buflen, max_bytes;
    videncode_proc_t *callback;
{
    static int loc, frame_count=0, heaplen;
    register int i, qtr_blocks=0, half_blocks=0, full_blocks=0;
    register int bytes=0, bytes_left=0;

    videncode_buf = buf;
    videncode_len = buflen;
    videncode_callback = callback;

    max_bytes -= VIDCODE_FRAME_END_LEN;

    VidEncode_FindDiffs(last_image, image, VIDENCODE_THRESHOLD, 0);
    if (encodeheaplen*VIDCODE_QTRBLOCK_LEN > max_bytes) {
	qtr_blocks = max_bytes/VIDCODE_QTRBLOCK_LEN;
    } else if (encodeheaplen*VIDCODE_HALFBLOCK_LEN > max_bytes) {
	half_blocks = (max_bytes-VIDCODE_QTRBLOCK_LEN*encodeheaplen) /
	    (VIDCODE_HALFBLOCK_LEN-VIDCODE_QTRBLOCK_LEN);
	qtr_blocks = encodeheaplen - half_blocks;
    } else if (encodeheaplen*VIDCODE_FULLBLOCK_LEN > max_bytes) {
	full_blocks = (max_bytes-VIDCODE_HALFBLOCK_LEN*encodeheaplen) /
	    (VIDCODE_FULLBLOCK_LEN-VIDCODE_HALFBLOCK_LEN);
	half_blocks = encodeheaplen - full_blocks;
    } else {
	full_blocks = encodeheaplen;
	bytes_left = max_bytes-VIDCODE_FULLBLOCK_LEN*encodeheaplen;
    }

    for (i=0; i<qtr_blocks; i++) {
	loc = EncodeHeap_DeleteMax();
	age[loc] = (random() % 32)+64;
	bytes +=
	    VidEncode_PutQtrBlock(image, loc%block_width, loc/block_width);
    }

    for (i=0; i<half_blocks; i++) {
	loc = EncodeHeap_DeleteMax();
	age[loc] = (random() % 32)+64;
	bytes +=
	    VidEncode_PutHalfBlock(image, loc%block_width, loc/block_width);
    }

    for (i=0; i<full_blocks; i++) {
	loc = EncodeHeap_DeleteMax();
	age[loc] = (random() % 32)-16;
	bytes +=
	    VidEncode_PutFullBlock(image, loc%block_width, loc/block_width);
    }

    if (bytes_left > max_aged) bytes_left = max_aged;
    if ((xmitted_image != NULL) && (bytes_left > 0)) {
	VidEncode_FindDiffs(xmitted_image, image, VIDENCODE_THRESHOLD, 1);
	full_blocks = bytes_left/VIDCODE_FULLBLOCK_LEN;
	if (full_blocks > encodeheaplen) full_blocks = encodeheaplen;
	for (i=0; i<full_blocks; i++) {
	    loc = EncodeHeap_DeleteMax();
	    age[loc] = (random() % 32)-16;
	    bytes +=
		VidEncode_PutFullBlock(image, loc%block_width, loc/block_width);
	}
    }

    (void) VidEncode_PutFrameEnd();
}
