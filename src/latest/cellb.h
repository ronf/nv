/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>

	CellB coder definitions
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

extern int CellB_Encode_Probe(grabber_t *grabber);
extern void *CellB_Encode_Start(grabber_t *grabber, int max_bandwidth,
				int min_framespacing, int config);
extern void *CellB_Encode_Restart(void *enc_state, int max_bandwidth,
				  int min_framespacing, int config);
extern int CellB_Encode(void *enc_state, vidimage_t *image, uint8 *buf,
			int *lenp, int *markerp, uint32 *timestampp);
extern void CellB_Encode_Stop(void *enc_state);

extern int CellB_Decode(vidimage_t *image, uint8 *data, int len);
