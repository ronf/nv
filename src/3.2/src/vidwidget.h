/*
	Netvideo version 3.2
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video widget private definitions
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

typedef struct vidwidget vidwidget_t;

typedef void vidupdate_proc_t(vidwidget_t *image, int x, int y, int width,
			      int height);
 
struct vidwidget {
    vidimage_t		*image;
    Tcl_Interp		*interp;
    Tk_Uid		screenName;
    Tk_Window		tkwin;
    GC			gc;
    XImage		*ximage;
    XShmSegmentInfo	shminfo;
    vidupdate_proc_t	*update_rect;
    int			scalebits;		/* -1=double size */
    int			flags;
};

#define REDRAW_PENDING		1

extern vidupdate_proc_t VidGrey_MSB1bit,  VidGrey_LSB1bit;
extern vidupdate_proc_t VidGrey_MSB8bit,  VidGrey_LSB8bit;
extern vidupdate_proc_t VidGrey_MSB16bit, VidGrey_LSB16bit;
extern vidupdate_proc_t VidGrey_24bit;

extern vidupdate_proc_t VidColor_MSB8bit,  VidColor_LSB8bit;
extern vidupdate_proc_t VidColor_MSB16bit, VidColor_LSB16bit;
extern vidupdate_proc_t VidColor_MSB24bit, VidColor_LSB24bit;
