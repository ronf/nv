/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video utility definitions
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

/* Mildly gross but moderately portable test for littleendian machines */
#define LITTLEENDIAN (ntohl(0x12345678) != 0x12345678)

extern uint8 rgb2y[32768];
extern uint16 rgb2uv[32768];

typedef struct {
    XImage	*image;
    void	*shminfo;
} ximage_t;

extern void VidUtil_Init(Display *dpy);
extern ximage_t *VidUtil_AllocStdXImage(Display *dpy, Visual *vis, int depth,
					int width, int height);
extern ximage_t *VidUtil_AllocXImage(Display *dpy, Visual *vis, int depth,
				     int width, int height, int readonly);
extern void VidUtil_DestroyXImage(Display *dpy, ximage_t *ximage);
extern void VidUtil_GetXImage(Display *dpy, Window w, int x, int y,
			      ximage_t *ximage);
extern void VidUtil_PutXImage(Display *dpy, Window w, GC gc, int x, int y,
			      ximage_t *ximage);
