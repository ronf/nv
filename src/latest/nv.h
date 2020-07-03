/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	NV coder definitions
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

#define NV_MAX_WIDTH		2048
#define NV_MAX_HEIGHT		2048

#define NV_COLORFLAG		0x8000
#define NV_WIDTHMASK		0x0fff

#define NV_ENCODING_STD		0x0000
#define NV_ENCODING_DCT		0x1000
#define NV_ENCODINGMASK		0xf000
#define NV_HEIGHTMASK		0x0fff

typedef void nv_fwdtransform_t(int grabtype, uint8 *imgp, int width,
			       uint32 *out);
typedef void nv_revtransform_t(uint32 *inp, uint8 *yp, uint8 *uvp, int width);

extern nv_fwdtransform_t NV_FwdTransform, NVDCT_FwdTransform;
extern nv_revtransform_t NV_RevTransform, NVDCT_RevTransform;

extern int NV_Encode_Probe(grabber_t *grabber);
extern void *NV_Encode_Start(grabber_t *grabber, int max_bandwidth,
			     int min_framespacing, int config);
extern void *NV_Encode_Restart(void *enc_state, int max_bandwidth,
			       int min_framespacing, int config);
extern int NV_Encode(void *enc_state, vidimage_t *image, uint8 *buf,
		     int *lenp, int *markerp, uint32 *timestampp);
extern void NV_Encode_Stop(void *enc_state);
 
extern int NV_Decode(vidimage_t *image, uint8 *data, int len);

extern uint32 RTPTime(void);
