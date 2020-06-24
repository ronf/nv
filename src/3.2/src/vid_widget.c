/*
        Netvideo version 3.2
        Written by Ron Frederick <frederick@parc.xerox.com>
 
        Video TK widget routines
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <default.h>
#include <tk.h>
#include "video.h"
#include "vidwidget.h"

#define LITTLEENDIAN (ntohl(0x12345678) != 0x12345678)

extern Tk_Window tkMainWin;
extern int screenDepth, color_ok;
extern Visual *visual;
extern Colormap colmap;

u_long bg_color, bg_fill, black_pix, white_pix;
u_long yuv2rgb[65536], grey_lut[256], color_lut[32768];

static int completion, shm_available=1;

/*ARGSUSED*/
static int ErrHandler(Display *dpy, XErrorEvent *errevp)
{
    shm_available = 0;
    return 0;
}

/*ARGSUSED*/
static Bool Completion(Display *dpy, XEvent *eventp, char *arg)
{
    return eventp->xany.type == completion;
}

static void VidWidget_SetUpdateRoutine(vidwidget_t *vidPtr)
{
    Display *dpy=Tk_Display(tkMainWin);
    register vidimage_t *image=vidPtr->image;
    int color = (image->flags & VIDIMAGE_ISCOLOR) &&
		(image->flags & VIDIMAGE_WANTCOLOR) && color_ok;

    switch (screenDepth) {
    case 1:
	if (LITTLEENDIAN) {
	    vidPtr->update_rect = VidGrey_LSB1bit;
	} else {
	    vidPtr->update_rect = VidGrey_MSB1bit;
	}
	break;
    case 8:
	if (LITTLEENDIAN) {
	    vidPtr->update_rect = color ? VidColor_LSB8bit : VidGrey_LSB8bit;
	} else {
	    vidPtr->update_rect = color ? VidColor_MSB8bit : VidGrey_MSB8bit;
	}
	break;
    case 16:
	if (LITTLEENDIAN) {
	    vidPtr->update_rect = color ? VidColor_LSB16bit : VidGrey_LSB16bit;
	} else {
	    vidPtr->update_rect = color ? VidColor_MSB16bit : VidGrey_MSB16bit;
	}
	break;
    case 24:
    case 32:
	if (LITTLEENDIAN) {
	    vidPtr->update_rect = color ? VidColor_LSB24bit : VidGrey_24bit;
	} else {
	    vidPtr->update_rect = color ? VidColor_MSB24bit : VidGrey_24bit;
	}
	break;
    }
}

static void VidWidget_AllocXImage(vidwidget_t *vidPtr)
{
    Display *dpy=Tk_Display(tkMainWin);
    register vidimage_t *image=vidPtr->image;
    register int scalebits=vidPtr->scalebits, width, height, ximage_size;

    if (scalebits == -1) {
	width = image->width*2;
	height = image->height*2;
    } else {
	width = image->width >> scalebits;
	height = image->height >> scalebits;
    }

    Tk_GeometryRequest(vidPtr->tkwin, width, height);

    if (shm_available) {
	vidPtr->ximage = XShmCreateImage(dpy, visual, screenDepth, ZPixmap, 0,
	    &vidPtr->shminfo, width, height);
	ximage_size = vidPtr->ximage->bytes_per_line * vidPtr->ximage->height;

	vidPtr->shminfo.shmid =
	    shmget(IPC_PRIVATE, ximage_size, IPC_CREAT|0777);
	shm_available = (vidPtr->shminfo.shmid >= 0);
	if (shm_available) {
	    vidPtr->shminfo.shmaddr = vidPtr->ximage->data =
		(char *) shmat(vidPtr->shminfo.shmid, 0, 0);
	    vidPtr->shminfo.readOnly = True;

	    XSetErrorHandler(ErrHandler);
	    XShmAttach(dpy, &vidPtr->shminfo);
	    XSync(dpy, False);
	    XSetErrorHandler(NULL);

	    if (!shm_available) {
		shmdt(vidPtr->shminfo.shmaddr);
		shmctl(vidPtr->shminfo.shmid, IPC_RMID, 0);
		XDestroyImage(vidPtr->ximage);
	    }
	} else {
	    XDestroyImage(vidPtr->ximage);
	}
    }

    if (!shm_available) {
	vidPtr->ximage = XCreateImage(dpy, visual, screenDepth, ZPixmap, 0,
	    NULL, width, height, (screenDepth == 24) ? 32 : screenDepth, 0);
	ximage_size = vidPtr->ximage->bytes_per_line * vidPtr->ximage->height;
	vidPtr->ximage->data = (char *) malloc(ximage_size);
    }

    VidWidget_UpdateRect(vidPtr, 0, 0, image->width, image->height);
}

static void VidWidget_DestroyXImage(vidwidget_t *vidPtr)
{
    if (shm_available) {
	XShmDetach(Tk_Display(vidPtr->tkwin), &vidPtr->shminfo);
	shmdt(vidPtr->shminfo.shmaddr);
	shmctl(vidPtr->shminfo.shmid, IPC_RMID, 0);
    } else {
	free(vidPtr->ximage->data);
    }

    XDestroyImage(vidPtr->ximage);
}

static int VidWidget_Copy(Tcl_Interp *interp, vidwidget_t *vidPtr, int argc,
			  char **argv)
{
    Display *dpy=Tk_Display(vidPtr->tkwin);
    Tk_Window destwin;
    Pixmap pixmap;

    if (argc != 1) {
	Tcl_AppendResult(interp, "wrong # args", NULL);
	return TCL_ERROR;
    } else if (!(destwin = Tk_NameToWindow(interp, argv[0], vidPtr->tkwin))) {
	Tcl_AppendResult(interp, "invalid window name", NULL);
	return TCL_ERROR;
    }

    pixmap = XCreatePixmap(dpy, Tk_WindowId(vidPtr->tkwin),
	vidPtr->ximage->width, vidPtr->ximage->height, screenDepth);
    XPutImage(dpy, pixmap, vidPtr->gc, vidPtr->ximage, 0, 0, 0, 0,
	vidPtr->ximage->width, vidPtr->ximage->height);
    Tk_GeometryRequest(destwin, vidPtr->ximage->width, vidPtr->ximage->height);
    Tk_SetWindowVisual(destwin, visual, screenDepth, colmap);
    Tk_SetWindowBackgroundPixmap(destwin, pixmap);
    Tk_MakeWindowExist(destwin);
    XFreePixmap(dpy, pixmap);

    return TCL_OK;
}

static void VidWidget_Destroy(ClientData vidwidget)
{
    vidwidget_t *vidPtr = (vidwidget_t *) vidwidget;

    ckfree((char *) vidPtr);
}

static void VidWidget_Display(ClientData vidwidget)
{
    register vidwidget_t *vidPtr = (vidwidget_t *) vidwidget;
    XEvent event;

    vidPtr->flags &= ~REDRAW_PENDING;
    if ((vidPtr->tkwin != NULL) && Tk_IsMapped(vidPtr->tkwin)) {
	Display *dpy = Tk_Display(vidPtr->tkwin);
	Window w = Tk_WindowId(vidPtr->tkwin);

	if (shm_available) {
	    XShmPutImage(dpy, w, vidPtr->gc, vidPtr->ximage, 0, 0, 0, 0,
		vidPtr->ximage->width, vidPtr->ximage->height, True);
	    XIfEvent(dpy, &event, Completion, NULL);
	} else {
	    XPutImage(dpy, w, vidPtr->gc, vidPtr->ximage, 0, 0, 0, 0,
		vidPtr->ximage->width, vidPtr->ximage->height);
	}
    }
}

static int VidWidget_InstanceCmd(ClientData vidwidget, Tcl_Interp *interp,
				 int argc, char **argv)
{
    register vidwidget_t *vidPtr = (vidwidget_t *) vidwidget;
    int result = TCL_OK;
    int length;
    char c;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " option ?arg arg ...?\"", NULL);
	return TCL_ERROR;
    }

    Tk_Preserve((ClientData) vidPtr);
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'c') && (strncmp(argv[1], "color", length) == 0)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " color <0|1>\"", NULL);
	    result = TCL_ERROR;
	} else {
	    vidimage_t *image=vidPtr->image;
	    VidImage_SetColor(image, image->flags & VIDIMAGE_ISCOLOR,
			      atoi(argv[2]));
	    result = TCL_OK;
	}
    } else if ((c == 'c') && (strncmp(argv[1], "copy", length) == 0)) {
	result = VidWidget_Copy(interp, vidPtr, argc-2, argv+2);
    } else if ((c == 'r') && (strncmp(argv[1], "redraw", length) == 0)) {
	VidWidget_Redraw(vidPtr);
	result = TCL_OK;
    } else if ((c == 's') && (strncmp(argv[1], "scale", length) == 0)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " scale <scalebits>\"", NULL);
	    result = TCL_ERROR;
	} else {
	    if (vidPtr->scalebits != atoi(argv[2])) {
		VidWidget_DestroyXImage(vidPtr);
		vidPtr->scalebits = atoi(argv[2]);
		VidWidget_AllocXImage(vidPtr);
		VidWidget_SetUpdateRoutine(vidPtr);
		VidWidget_UpdateRect(vidPtr, 0, 0, vidPtr->image->width,
				     vidPtr->image->height);
		VidWidget_Redraw(vidPtr);
	    }
	    result = TCL_OK;
	}
    } else {
	Tcl_AppendResult(interp, "unknown option ", argv[1], " for ", argv[0],
		NULL);
	result = TCL_ERROR;
    }
    Tk_Release((ClientData) vidPtr);
    return result;
}

static void VidWidget_EventProc(ClientData vidwidget, XEvent *eventPtr)
{
    register vidwidget_t *vidPtr = (vidwidget_t *) vidwidget;
    int i;

    if ((eventPtr->type == Expose) && (eventPtr->xexpose.count == 0)) {
	VidWidget_UpdateRect(vidPtr, 0, 0, vidPtr->image->width,
	    vidPtr->image->height);
	VidWidget_Redraw(vidPtr);
    } else if (eventPtr->type == DestroyNotify) {
	VidWidget_DestroyXImage(vidPtr);
	Tcl_DeleteCommand(vidPtr->interp, Tk_PathName(vidPtr->tkwin));
	vidPtr->tkwin = NULL;

	for (i=0; i<vidPtr->image->num_widgets; i++) {
	    if (vidPtr->image->widgetlist[i] == vidPtr) {
		vidPtr->image->widgetlist[i] =
		    vidPtr->image->widgetlist[--vidPtr->image->num_widgets];
		break;
	    }
	}

	if (vidPtr->flags & REDRAW_PENDING)
	    Tk_CancelIdleCall(VidWidget_Display, (ClientData) vidPtr);
	Tk_EventuallyFree((ClientData) vidPtr, VidWidget_Destroy);
    }
}

#define YUVLIM(x) (((x) < 0) ? 0 : ((x) > 127) ? 127 : (x))

static int VidWidget_FindShift(u_long mask)
{
    int shift=0;

    if (mask == 0) return 0;

    while ((mask & 1) == 0) {
	mask >>= 1;
	shift++;
    }

    return shift;
}

void VidWidget_Init(void)
{
    int i, screen, r_shift, g_shift, b_shift;
    int r, g, b, r0, g0, b0, y, u, v, u0, v0;
    Display *dpy=Tk_Display(tkMainWin);
    XVisualInfo visinfo;
    XColor color;
    static u_long small_lut[128], red_lut[128], green_lut[128], blue_lut[128];

    if ((shm_available = XShmQueryExtension(dpy)) != 0)
	completion = XShmGetEventBase(dpy) + ShmCompletion;

    screen = DefaultScreen(dpy);
    if (DefaultDepth(dpy, screen) == 24) {
	screenDepth = 24;
	visual = DefaultVisual(dpy, screen);
	(void) XMatchVisualInfo(dpy, screen, 24, visual->class, &visinfo);
	colmap = XDefaultColormap(dpy, screen);
    } else if (XMatchVisualInfo(dpy, screen, 24, TrueColor, &visinfo)) {
	Tk_MakeWindowExist(tkMainWin);
	screenDepth = 24;
	visual = visinfo.visual;
	colmap = XCreateColormap(dpy, Tk_WindowId(tkMainWin), visual,
	    AllocNone);
    } else if (XMatchVisualInfo(dpy, screen, 24, DirectColor, &visinfo)) {
	Tk_MakeWindowExist(tkMainWin);
	screenDepth = 24;
	visual = visinfo.visual;
	colmap = XCreateColormap(dpy, Tk_WindowId(tkMainWin), visual,
	    AllocNone);
    } else {
	screenDepth = DefaultDepth(dpy, screen);
	visual = DefaultVisual(dpy, screen);
	colmap = XDefaultColormap(dpy, screen);
    }

    if (screenDepth == 1) {
	black_pix = bg_color = BlackPixel(dpy, screen);
	white_pix = WhitePixel(dpy, screen);
	bg_fill = black_pix * 0xff;
	color_ok = 0;
	if (LITTLEENDIAN) {
	    black_pix <<= 7;
	    white_pix <<= 7;
	}
    } else if ((screenDepth == 8)  || (screenDepth == 16)) {
	for (i=1; i<=GREY_LEVELS; i++) {
	    color.red = color.green = color.blue =
		i * 65536/GREY_LEVELS - 1;
	    color.flags = DoRed | DoGreen | DoBlue;
	    if (XAllocColor(dpy, colmap, &color) == 0) {
		fprintf(stderr, "Can't allocate enough greyscales!\n");
		exit(1);
	    }
	    if (LITTLEENDIAN && (screenDepth <= 16))
		color.pixel <<= (32-screenDepth);
	    grey_lut[i] = color.pixel;
	}
	grey_lut[0] = grey_lut[1];
	for (i=GREY_LEVELS+1; i<2*GREY_LEVELS; i++)
	    grey_lut[i] = grey_lut[GREY_LEVELS];
     
	bg_color = bg_fill = grey_lut[GREY_LEVELS/2];

	i = 0;
	for (b=0; b<4; b++) {
	    for (g=0; g<8; g++) {
		for (r=0; r<4; r++) {
		    color.red = r * 16384 + 16383;
		    color.green = g * 8192 + 8191;
		    color.blue = b * 16384 + 16383;
		    color.flags = DoRed | DoGreen | DoBlue;
		    if (XAllocColor(dpy, colmap, &color) == 0) {
			if (screenDepth < 24)
			    XFreeColors(dpy, colmap, small_lut, i, 0);
			fprintf(stderr, "Can't allocate color cube!\n");
			color_ok = 0;
			return;
		    } else {
			if (LITTLEENDIAN && (screenDepth <= 16))
			    color.pixel <<= (32-screenDepth);
			small_lut[i++] = color.pixel;
		    }
		}
	    }
	}

	i = 0;
	for (b=0; b<32; b++) {
	    b0 = (b < 4) ? 0 : (b < 16) ? b/4-1 : 3;
	    for (g=0; g<32; g++) {
		g0 = (g < 2) ? 0 : (g < 16) ? g/2-1 : 7;
		for (r=0; r<32; r++) {
		    r0 = (r < 4) ? 0 : (r < 16) ? r/4-1 : 3;
		    color_lut[i++] = small_lut[b0*32+g0*4+r0];
		}
	    }
	}

	i = 0;
	for (u=0; u<128; u+=4) {
	    for (v=0; v<128; v+=4) {
		for (y=0; y<128; y+=2) {
		    u0 = (u < 64) ? u : (u-128);
		    v0 = (v < 64) ? v : (v-128);
		    r = y + 11*v0/8;
		    g = y - 11*u0/32 -45*v0/64;
		    b = y + 111*u0/64;
		    yuv2rgb[i++] = (YUVLIM(b)<<16) + (YUVLIM(g)<<8) + YUVLIM(r);
		}
	    }
	}
	color_ok = 1;
    } else if ((screenDepth == 24) || (screenDepth == 32)) {
	for (i=0; i<128; i++) {
	    color.red = color.green = color.blue = i*512 + 256;
	    color.flags = DoRed | DoGreen | DoBlue;
	    if (XAllocColor(dpy, colmap, &color) == 0) {
		fprintf(stderr, "Can't allocate colors!\n");
		exit(1);
	    }
	    grey_lut[2*i] = grey_lut[2*i+1] = color.pixel;
	    red_lut[i] = color.pixel & visinfo.red_mask;
	    green_lut[i] = color.pixel & visinfo.green_mask;
	    blue_lut[i] = color.pixel & visinfo.blue_mask;
	}

	bg_color = bg_fill = grey_lut[128];

	i = 0;
	for (u=2; u<128; u+=4) {
	    for (v=2; v<128; v+=4) {
		for (y=1; y<128; y+=2) {
		    u0 = (u < 64) ? u : (u-128);
		    v0 = (v < 64) ? v : (v-128);
		    r = y + 11*v0/8;
		    g = y - 11*u0/32 -45*v0/64;
		    b = y + 111*u0/64;
		    yuv2rgb[i++] = red_lut[YUVLIM(r)] +
				   green_lut[YUVLIM(g)] +
				   blue_lut[YUVLIM(b)];
		}
	    }
	}
	color_ok = 1;
    } else {
	fprintf(stderr, "Can't handle this type of display.\n");
	exit(1);
    }
}

void *VidWidget_Create(void *interp, char *name, int toplevel,
		       vidimage_t *image)
{
    Tk_Window newwin;
    register vidwidget_t *vidPtr;
    Tk_Uid screenUid;
    XGCValues gcValues;

    if (image->num_widgets == MAX_WIDGETS) return NULL;

    if (toplevel) {
	screenUid = Tk_GetUid("");
    } else {
	screenUid = NULL;
    }

    newwin = Tk_CreateWindowFromPath(interp, tkMainWin, name, screenUid);
    if (newwin == NULL) return NULL;

    Tk_SetClass(newwin, "VideoWidget");
    Tk_SetWindowVisual(newwin, visual, screenDepth, colmap);

    vidPtr = (vidwidget_t *) ckalloc(sizeof(vidwidget_t));
    memset((char *)vidPtr, 0, sizeof(vidwidget_t));
    vidPtr->image = image;
    vidPtr->interp = interp;
    vidPtr->screenName = screenUid;
    vidPtr->tkwin = newwin;

    Tk_CreateEventHandler(vidPtr->tkwin, ExposureMask|StructureNotifyMask,
	    VidWidget_EventProc, (ClientData) vidPtr);
    Tcl_CreateCommand(interp, Tk_PathName(vidPtr->tkwin),
	    VidWidget_InstanceCmd, (ClientData) vidPtr, (void (*)()) NULL);

    Tk_SetWindowBackground(vidPtr->tkwin, bg_color);
    vidPtr->gc = Tk_GetGC(vidPtr->tkwin, 0, &gcValues);

    vidPtr->scalebits = 0;
    VidWidget_AllocXImage(vidPtr);
    VidWidget_SetUpdateRoutine(vidPtr);
    VidWidget_UpdateRect(vidPtr, 0, 0, image->width, image->height);
    VidWidget_Redraw(vidPtr);

    image->widgetlist[image->num_widgets++] = vidPtr;
    return vidPtr;
}

void VidWidget_Redraw(void *vidwidget)
{
    register vidwidget_t *vidPtr = vidwidget;

    if ((vidPtr->tkwin != NULL) && !(vidPtr->flags & REDRAW_PENDING)) {
	Tk_DoWhenIdle(VidWidget_Display, (ClientData) vidPtr);
	vidPtr->flags |= REDRAW_PENDING;
    }
}

void VidWidget_SetColor(void *vidwidget)
{
    register vidwidget_t *vidPtr = vidwidget;
    register vidimage_t *image=vidPtr->image;

    VidWidget_SetUpdateRoutine(vidPtr);
    VidWidget_UpdateRect(vidPtr, 0, 0, image->width, image->height);
    VidWidget_Redraw(vidPtr);
}

void VidWidget_SetSize(void *vidwidget)
{
    register vidwidget_t *vidPtr = vidwidget;
    register vidimage_t *image=vidPtr->image;

    VidWidget_DestroyXImage(vidPtr);
    VidWidget_AllocXImage(vidPtr);
    VidWidget_UpdateRect(vidPtr, 0, 0, image->width, image->height);
    VidWidget_Redraw(vidPtr);
}

void VidWidget_UpdateRect(void *vidwidget, int x, int y, int width, int height)
{
    register vidwidget_t *vidPtr = vidwidget;
    register int scalebits=vidPtr->scalebits, scaler=1<<scalebits, alignmask;

    if ((vidPtr->tkwin == NULL) || !Tk_IsMapped(vidPtr->tkwin)) return;

    if (scalebits == -1) {
	alignmask = 3;
    } else {
	alignmask = 8*scaler-1;
    }

    width += x & alignmask;
    x -= x & alignmask;

    if (width & alignmask) width += alignmask+1 - (width & alignmask);

    if (scalebits > 0) {
	alignmask = scaler-1;
	height += y & alignmask;
	y -= y & alignmask;
	if (height & alignmask) height += alignmask+1 - (height & alignmask);
    }

    vidPtr->update_rect(vidPtr, x, y, width, height);
}
