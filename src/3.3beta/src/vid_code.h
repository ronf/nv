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

typedef void reconfigproc_t(int color, int width, int height);

typedef int grabproc_t(uint8 *y_data, int8 *uv_data);

typedef struct {
    char name[64];
    char keyword[64];
    int (*probe)(void);
    char *(*attach)(void);
    void (*detach)(void);
    grabproc_t *(*start)(int max_framerate, int config, reconfigproc_t *resize);
    void (*stop)(void);
    int config_mask;
} grabber_t;

typedef void xmitproc_t(int sync, int content, int timestamp,
			uint8 *hdr, int hdrlen, uint8 *data, int datalen);

typedef int encodeproc_t(vidimage_t *image, xmitproc_t *callback);

typedef struct {
    char name[64];
    char keyword[64];
    int (*probe)(grabber_t *grabber);
    encodeproc_t *(*start)(grabber_t *grabber, int max_bandwidth,
			   int max_framerate, int config);
    void (*stop)(grabber_t *grabber);
    int available;
} encoding_t;

typedef int decodeproc_t(vidimage_t *image, uint8 *data, int len);
