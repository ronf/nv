/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video widget definitions
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
    ximage_t		*ximage;
    vidupdate_proc_t	*update_rect;
    int16		maxwidth, maxheight;	/* Nonzero if auto-scaling */
    int			scalebits;		/* -1=double size */
    uint32		flags;
};

extern vidupdate_proc_t VidGrey_MSB1bit,  VidGrey_LSB1bit;
extern vidupdate_proc_t VidGrey_MSB8bit,  VidGrey_LSB8bit;
extern vidupdate_proc_t VidGrey_MSB16bit, VidGrey_LSB16bit;
extern vidupdate_proc_t VidGrey_24bit;

extern vidupdate_proc_t VidColor_MSB8bit,  VidColor_LSB8bit;
extern vidupdate_proc_t VidColor_MSB16bit, VidColor_LSB16bit;
extern vidupdate_proc_t VidColor_MSB24bit, VidColor_LSB24bit;

extern int VidWidget_Init(Display *dpy);
extern void *VidWidget_Create(void *interp, char *name, int toplevel,
			      void *parent, vidimage_t *image, int maxwidth,
			      int maxheight);
extern void VidWidget_Redraw(void *vidwidget);
extern void VidWidget_Resize(void *vidwidget);
extern void VidWidget_SetColor(void *vidwidget);
extern void VidWidget_SetMaxSize(void *vidwidget, int maxwidth, int maxheight);
extern void VidWidget_UpdateRect(void *vidwidget, int x, int y, int width,
				 int height);
