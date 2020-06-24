/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video coder definitions
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

#define NTSC_WIDTH		320
#define NTSC_HEIGHT		240

#define PAL_WIDTH		384
#define PAL_HEIGHT		288

#define VID_SMALL		0x01
#define VID_MEDIUM		0x02
#define VID_LARGE		0x04
#define VID_SIZEMASK		0x07

#define VID_GREYSCALE		0x08
#define VID_COLOR		0x10

#define VIDIMAGE_GREY		0
#define VIDIMAGE_YUYV		1
#define VIDIMAGE_UYVY		2
#define VIDIMAGE_CELLB		3

typedef void reconfigproc_t(void *enc_state, int width, int height);

typedef int grabproc_t(uint8 **datap, int *lenp);

typedef struct {
    char name[64];
    char keyword[64];
    int (*probe)(void);
    char *(*attach)(void);
    void (*detach)(void);
    grabproc_t *(*start)(int grabtype, int min_framespacing, int config,
			 reconfigproc_t *reconfig, void *enc_state);
    void (*stop)(void);
    int config_mask;
} grabber_t;

typedef struct {
    char name[64];
    char keyword[64];
    int rtp_pt;
    int (*probe)(grabber_t *grabber);
    void *(*start)(grabber_t *grabber, int max_bandwidth, int min_framespacing,
		   int config);
    void *(*restart)(void *enc_state, int max_bandwidth, int min_framespacing,
		     int config);
    int (*encode)(void *enc_state, vidimage_t *image, uint8 *buf, int *lenp,
		  int *markerp, uint32 *timestampp);
    void (*stop)(void *enc_state);
    int available;
} encoding_t;

typedef int decodeproc_t(vidimage_t *image, uint8 *data, int len);
