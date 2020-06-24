/*
	Netvideo version 2.7
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video encoder and decoder definitions
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

#define NUM_VIDCODES		16

#define VIDCODE_VERSION		2

#define VIDCODE_HDR_LEN		4
#define VIDCODE_QTRBLOCK_LEN	(VIDCODE_HDR_LEN+4)
#define VIDCODE_HALFBLOCK_LEN	(VIDCODE_HDR_LEN+8)
#define VIDCODE_FULLBLOCK_LEN	(VIDCODE_HDR_LEN+32)
#define VIDCODE_FRAME_END_LEN	VIDCODE_HDR_LEN

#define VIDCODE_NAME		0
#define VIDCODE_QTRBLOCK	1
#define VIDCODE_HALFBLOCK	2
#define VIDCODE_FULLBLOCK	3
#define VIDCODE_FRAME_END	4

#define VIDCODE_NTSC		0
#define VIDCODE_PAL		1

typedef struct {
    unsigned	protvers:3;
    unsigned	fmt:1;
    unsigned	type:4;
    unsigned	x:8;
    unsigned	y:8;
    unsigned	initpix:8;
} vidcode_hdr_t;

typedef void videncode_proc_t(/* char *buf, int len */);

extern char vidcode_to_diff[NUM_VIDCODES], viddiff_to_code[GREY_LEVELS*2];

#define ADDPIX(pix, code) \
    ((pix += vidcode_to_diff[(code)]), (pix = pix & GREY_MASK))

#define PIXDIFF(pix, newpix, diff) \
    ((diff = (newpix)-(pix)+GREY_LEVELS), ADDPIX(pix, viddiff_to_code[diff]), \
     ((newpix) = (pix)), viddiff_to_code[diff])

extern void VidEncode_Init(/* int width, int height */);
extern void VidEncode_Reset();
extern void VidEncode(/* unsigned char *xmitted_image, unsigned char *image,
			 char *buf, int buflen, int max_bytes, int max_aged,
			 videncode_proc_t *callback */);

extern void VidDecode_Init();
extern int  VidDecode(/* vidimage_t *image, char *data, int len */);
