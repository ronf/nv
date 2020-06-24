/*
	Netvideo version 3.2
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

#define VIDCODE_MAX_WIDTH	2048
#define VIDCODE_MAX_HEIGHT	2048

#define VIDCODE_COLORFLAG	0x8000
#define VIDCODE_WIDTHMASK	0x0fff

#define VIDCODE_BGFLAG		0x80
#define VIDCODE_BGBLOCK		0xff

#define VIDCODE_MAX_RUNW	127

#ifndef ultrix
typedef signed char s_char;
#endif

extern void VidTransform_Fwd(u_char *yp, s_char *uvp, int width, u_long *out);
extern void VidTransform_Rev(u_long *inp, u_char *yp, s_char *uvp, int width);

typedef void videncode_proc_t(u_char *hdr, int hdrlen, u_char *data, int len,
			      int end);

extern void VidEncode_Init(int width, int height);
extern void VidEncode_Reset(void);
extern int  VidEncode(u_char *old_y_data, u_char *curr_y_data,
		      s_char *curr_uv_data, int max_packlen,
		      int aging_bytes, videncode_proc_t *callback);

extern void VidDecode(vidimage_t *image, u_char *data, int len);
