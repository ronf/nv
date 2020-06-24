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

extern void NV_FwdTransform(uint8 *yp, int8 *uvp, int width, uint32 *out);
extern void NV_RevTransform(uint32 *inp, uint8 *yp, int8 *uvp, int width);

extern int NV_Encode_Probe(grabber_t *grabber);
extern encodeproc_t *NV_Encode_Start(grabber_t *grabber, int max_bandwidth,
				     int max_framerate, int config);
extern void NV_Encode_Stop(grabber_t *grabber);
 
extern int NV_Decode(vidimage_t *image, uint8 *data, int len);
