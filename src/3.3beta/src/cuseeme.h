/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	CU-SeeMe coder definitions
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

#define CUSEEME_HALFSIZE	1
#define CUSEEME_FULLSIZE	2

#define CUSEEME_HALFWIDTH	160
#define CUSEEME_HALFHEIGHT	120

#define CUSEEME_FULLWIDTH	320
#define CUSEEME_FULLHEIGHT	240

extern int CUSeeMe_Encode_Probe(grabber_t *grabber);
extern encodeproc_t *CUSeeMe_Encode_Start(grabber_t *grabber, int max_bandwidth,
					  int max_framerate, int config);
extern void CUSeeMe_Encode_Stop(grabber_t *grabber);

extern int CUSeeMe_Decode(vidimage_t *image, uint8 *data, int len);
