/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	DECstation PIP frame grab routines

	Derived from code written by Steve McCanne <mccanne@ee.lbl.gov> in
	March 1993, which was based on earlier work done by Eric Anderson,
	Jon Kay, Vach Kompella, and Kevin Fall, UCSD Computer Systems Lab.
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

#ifdef PIP
#include <stdio.h>
#include <signal.h>
#include <sys/param.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xvlib.h>
#include <tk.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_util.h"
#include "vid_code.h"
#include "pip_grab.h"

extern Tk_Window tkMainWin;

static Display *dpy;
static XVisualInfo vinfo;
static Window root, w;
static GC gc;
static Colormap cmap;
static int full_width, full_height;
static int xmit_size, xmit_color, width, height;
static reconfigproc_t *reconfig;
static XvPortID grabID;
static ximage_t *ximage;

/*ARGSUSED*/
static int PIP_ErrHandler(ClientData clientData, XErrorEvent *errevp)
{
    int *errp=(int *)clientData;

    *errp = 1;
    return 0;
}

static void PIP_GetImage(void)
{
    int oldmask;

    /*
     * The brain dead video hardware requires that the window
     * be unobscured in order to capture the video.  So we grab
     * the server and raise the window.  This won't work if
     * the window isn't mapped.  Also, we block signals while
     * the server is grabbed.  Otherwise, the process could be
     * stopped while the display is locked out.
     */
    oldmask = sigsetmask(~0);
    XGrabServer(dpy);
    XRaiseWindow(dpy, w);
    XSync(dpy, 0);
    XvPutStill(dpy, grabID, w, gc, 0, 0, full_width, full_height,
	       0, 0, width, height);
    XSync(dpy, False);
    XUngrabServer(dpy);
    (void) sigsetmask(oldmask);
}

static void PIP_Reconfig(void)
{
    /*XXX*/
    full_width = NTSC_WIDTH*2;
    full_height = NTSC_HEIGHT*2;

    switch (xmit_size) {
    case VID_SMALL:
	width = full_width/4;
	height = full_height/4;
	break;
    case VID_MEDIUM:
	width = full_width/2;
	height = full_height/2;
	break;
    case VID_LARGE:
	width = full_width;
	height = full_height;
	break;
    }

    if (w != None) XResizeWindow(dpy, w, width, height);

    if (ximage != NULL) VidUtil_DestroyXImage(dpy, ximage);
    ximage = VidUtil_AllocXImage(dpy, vinfo.visual, 24, width, height,
				 False);

    if (reconfig) (*reconfig)(xmit_color, width, height);
}

static void PIP_GrabColor(uint8 *y_data, int8 *uv_data)
{
    int x, y;
    uint8 *yp=y_data, *ry=rgb2y;
    uint16 *uvp=(uint16 *)uv_data, *ruv=rgb2uv;
    uint32 p0, p1, *data=(uint32 *)ximage->image->data;

    for (y=0; y<height; y++) {
	for (x=0; x<width; x+=2) {
	    p0 = data[0];
	    p1 = data[1];
	    data += 2;

	    yp[0] = ry[((p0<<7) & 0x7c00)+((p0>>6) & 0x3e0)+((p0>>19) & 0x1f)];
	    yp[1] = ry[((p1<<7) & 0x7c00)+((p1>>6) & 0x3e0)+((p1>>19) & 0x1f)];
	    yp += 2;

	    p0 &= 0xfefeff;
	    p1 &= 0xfefeff;
	    p0 += p1;
            *uvp++ = ruv[((p0<<6) & 0x7c00) +
			 ((p0>>7) & 0x3e0) +
			 ((p0>>20) & 0x1f)];
	}
    }
}

static void PIP_GrabGrey(uint8 *y_data)
{
    int x, y;
    uint8 *yp=y_data, *ry=rgb2y;
    uint32 p0, p1, *data=(uint32 *)ximage->image->data;

    for (y=0; y<height; y++) {
	for (x=0; x<width; x+=2) {
	    p0 = data[0];
	    p1 = data[1];
	    data += 2;

	    yp[0] = ry[((p0<<7) & 0x7c00)+((p0>>6) & 0x3e0)+((p0>>19) & 0x1f)];
	    yp[1] = ry[((p1<<7) & 0x7c00)+((p1>>6) & 0x3e0)+((p1>>19) & 0x1f)];
	    yp += 2;
	}
    }
}

static int PIP_GrabFrame(uint8 *y_data, int8 *uv_data)
{
    int err=0;
    Tk_ErrorHandler handler;

    PIP_GetImage();

    handler = Tk_CreateErrorHandler(dpy, -1, -1, -1, PIP_ErrHandler, &err);
    VidUtil_GetXImage(dpy, w, 0, 0, ximage);
    Tk_DeleteErrorHandler(handler);
    if (err) return 0;

    if (xmit_color)
	PIP_GrabColor(y_data, uv_data);
    else
	PIP_GrabGrey(y_data);

    return 1;
}

int PIP_Probe(void)
{
    int i, ngrabbers, majop, eventbase, errbase;
    XvAdaptorInfo *grabber, *g;

    dpy = Tk_Display(tkMainWin);
    root = DefaultRootWindow(dpy);

    if (XQueryExtension(dpy, "XVideo", &majop, &eventbase, &errbase) == False)
	return 0;

    XvQueryAdaptors(dpy, root, &ngrabbers, &grabber);

    for (i=0; i<ngrabbers; i++) if (grabber[i].type & XvInputMask) break;
    if (i == ngrabbers) return 0;

    grabID = grabber[i].base_id;
    return VID_SMALL|VID_MEDIUM|VID_LARGE|VID_GREYSCALE|VID_COLOR;
}

char *PIP_Attach(void)
{
    int screen=Tk_ScreenNumber(tkMainWin), mask;
    XSetWindowAttributes attr;
    XColor sc, ec;

    if (XMatchVisualInfo(dpy, screen, 24, TrueColor, &vinfo) == 0) {
	fprintf(stderr, "Can't find a TrueColor visual with depth 24.\n");
	w = None;
	gc = None;
	cmap = None;
    } else {
	attr.colormap = cmap = XCreateColormap(dpy, root, vinfo.visual,
					       AllocNone);
	XAllocNamedColor(dpy, cmap, "black", &sc, &ec);
	attr.background_pixel = attr.border_pixel = sc.pixel;
	mask = CWBackPixel | CWBorderPixel | CWColormap;
	w = XCreateWindow(dpy, root, 0, 0, NTSC_WIDTH, NTSC_HEIGHT, 1, 24,
			  InputOutput, vinfo.visual, mask, &attr);
	XSetStandardProperties(dpy, w, "Local camera", "Camera", 0, 0, 0, 0);
	gc = XCreateGC(dpy, w, 0, 0);
    }

    return NULL;
}

void PIP_Detach(void)
{
    if (w != None) {
	XDestroyWindow(dpy, w);
	XFreeGC(dpy, gc);
	XFreeColormap(dpy, cmap);
	w = None;
	gc = None;
	cmap = None;
    }
}

/*ARGSUSED*/
grabproc_t *PIP_Start(int max_framerate, int config, reconfigproc_t *r)
{
    reconfig = r;
    xmit_size = (config & VID_SIZEMASK);
    xmit_color = (config & VID_COLOR);

    if (w != None) {
	PIP_Reconfig();
	XMapWindow(dpy, w);
	return PIP_GrabFrame;
    } else {
	return NULL;
    }
}

void PIP_Stop(void)
{
    VidUtil_DestroyXImage(dpy, ximage);
    XUnmapWindow(dpy, w);
    ximage = NULL;
    reconfig = NULL;
}
#endif /* PIP */
