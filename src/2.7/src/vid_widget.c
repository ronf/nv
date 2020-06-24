/*
        Netvideo version 2.7
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
#include <X11/extensions/XShm.h>
#include <default.h>
#include <tk.h>
#include "video.h"

typedef struct {
    vidimage_t		*image;
    Tcl_Interp		*interp;
    Tk_Uid		screenName;
    Tk_Window		tkwin;
    GC			gc;
    XImage		*ximage;
    XShmSegmentInfo	shminfo;
    int			scaler;		/* 0=double size, else 1/scaler */
    int			flags;
} VideoWidget;

#define REDRAW_PENDING		1

int screenDepth;
Visual *screenVisual;
int bg_color, bg_fill, black_pix, white_pix, color_lut[GREY_LEVELS];

static int completion, shm_available=1;
static Colormap colmap;
static Tk_Window tkMainWin;

static char dither4[4][4] =
    { {  56,  24, 120,  88 },
      {  96,  32,  64,  16 },
      {  72, 112,   8,  40 },
      {   0,  80,  48, 104 } };

#define DITHER(c, x, y) \
    (((c) > dither4[(y)][(x)]) ? white_pix : black_pix)

static char dither8[8][8] =
    { {   2, 118,  30, 110,   4, 112,  24, 104 },
      {  66,  34,  94,  62,  68,  36,  88,  56 },
      {  18,  98,  10, 126,  20, 100,  12, 120 },
      {  82,  50,  74,  42,  84,  52,  76,  44 },
      {   6, 114,  26, 106,   0, 116,  28, 108 },
      {  70,  38,  90,  58,  64,  32,  92,  60 },
      {  22, 102,  14, 122,  16,  96,   8, 124 },
      {  86,  54,  78,  46,  80,  48,  72,  40 } };
 
#define DBLDITHER(c, x, y) \
    (((c) > dither8[(y)][(x)]) ? white_pix : black_pix)

static int ErrHandler(dpy, errevp)
    Display *dpy;
    XErrorEvent *errevp;
{
    shm_available = 0;
    return 0;
}

static Bool Completion(dpy, eventp, arg)
    Display *dpy;
    XEvent *eventp;
    void *arg;
{
    return eventp->xany.type == completion;
}

static void VidWidget_AllocXImage(vidPtr)
    VideoWidget *vidPtr;
{    
    Display *dpy=Tk_Display(tkMainWin);
    register vidimage_t *image=vidPtr->image;
    register int scaler=vidPtr->scaler, width, height, ximage_size;

    if (scaler == 0) {
	width = image->width*2;
	height = image->height*2;
    } else {
	width = image->width/scaler;
	height = image->height/scaler;
    }

    Tk_GeometryRequest(vidPtr->tkwin, width, height);

    if (shm_available) {
	vidPtr->ximage = XShmCreateImage(dpy, 0, screenDepth, ZPixmap, 0,
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
	    XSync(dpy);
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
	vidPtr->ximage = XCreateImage(dpy, 0, screenDepth, ZPixmap, 0, NULL,
	    width, height, screenDepth, width);
	ximage_size = vidPtr->ximage->bytes_per_line * vidPtr->ximage->height;
	vidPtr->ximage->data = (char *) malloc(ximage_size);
    }

    VidWidget_UpdateRect((ClientData)vidPtr, 0, 0, image->width, image->height);
}

VidWidget_DestroyXImage(vidPtr)
    VideoWidget *vidPtr;
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

static int VidWidget_Copy(interp, vidPtr, argc, argv)
    Tcl_Interp *interp;
    register VideoWidget *vidPtr;
    int argc;
    char **argv;
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
    Tk_SetWindowBackgroundPixmap(destwin, pixmap);
    Tk_MakeWindowExist(destwin);
    XFreePixmap(dpy, pixmap);

    return TCL_OK;
}

static void VidWidget_UpdateMono(vidPtr, x, y, width, height)
    VideoWidget *vidPtr;
    register int x, y, width, height;
{
    register int w, z, scaler=vidPtr->scaler, imagewidth=vidPtr->image->width;
    register int ximagewidth=vidPtr->ximage->bytes_per_line;
    register unsigned char *ip, *greymap=vidPtr->image->greymap;
    register char *xip, row;
 
    if (vidPtr->scaler == 0) {
	ip = &vidPtr->image->data[y*imagewidth+x];
	xip = &vidPtr->ximage->data[y*2*ximagewidth+x/4];
	y %= 8;
	while (height--) {
	    z = (y % 4)*2;
	    for (w=width; w > 0; w -= 4) {
		row = DBLDITHER(greymap[*ip], 0, z);
		row = (row<<1) + DBLDITHER(greymap[*ip++], 1, z);
		row = (row<<1) + DBLDITHER(greymap[*ip], 2, z);
		row = (row<<1) + DBLDITHER(greymap[*ip++], 3, z);
		row = (row<<1) + DBLDITHER(greymap[*ip], 4, z);
		row = (row<<1) + DBLDITHER(greymap[*ip++], 5, z);
		row = (row<<1) + DBLDITHER(greymap[*ip], 6, z);
		*xip++ = (row<<1) + DBLDITHER(greymap[*ip++], 7, z);
	    }
	    ip -= width;
	    xip += ximagewidth-width/4;

	    z++;
	    for (w=width; w > 0; w -= 4) {
		row = DBLDITHER(greymap[*ip], 0, z);
		row = (row<<1) + DBLDITHER(greymap[*ip++], 1, z);
		row = (row<<1) + DBLDITHER(greymap[*ip], 2, z);
		row = (row<<1) + DBLDITHER(greymap[*ip++], 3, z);
		row = (row<<1) + DBLDITHER(greymap[*ip], 4, z);
		row = (row<<1) + DBLDITHER(greymap[*ip++], 5, z);
		row = (row<<1) + DBLDITHER(greymap[*ip], 6, z);
		*xip++ = (row<<1) + DBLDITHER(greymap[*ip++], 7, z);
	    }
	    y = (y + 1) % 8;
	    ip += imagewidth-width;
	    xip += ximagewidth-width/4;
	}
    } else {
	ip = &vidPtr->image->data[y*imagewidth+x];
	xip = &vidPtr->ximage->data[y/scaler*ximagewidth+x/scaler/8];
	y %= 8;
	while (height) {
	    z = y % 4;
	    for (w=width; w > 0; w -= 8*scaler) {
		row = DITHER(greymap[*ip], 0, z);
		ip += scaler;
		row = (row<<1) + DITHER(greymap[*ip], 1, z);
		ip += scaler;
		row = (row<<1) + DITHER(greymap[*ip], 2, z);
		ip += scaler;
		row = (row<<1) + DITHER(greymap[*ip], 3, z);
		ip += scaler;
		row = (row<<1) + DITHER(greymap[*ip], 0, z);
		ip += scaler;
		row = (row<<1) + DITHER(greymap[*ip], 1, z);
		ip += scaler;
		row = (row<<1) + DITHER(greymap[*ip], 2, z);
		ip += scaler;
		*xip++ = (row<<1) + DITHER(greymap[*ip], 3, z);
		ip += scaler;
	    }
	    y = (y + 1) % 8;
	    ip += imagewidth*scaler-width;
	    xip += ximagewidth-width/8/scaler;

	    height -= scaler;
	}
    }
}

static void VidWidget_UpdateGrey(vidPtr, x, y, width, height)
    VideoWidget *vidPtr;
    register int x, y, width, height;
{
    register int w, y1, scaler=vidPtr->scaler, imagewidth=vidPtr->image->width;
    register int ximagewidth=vidPtr->ximage->bytes_per_line;
    register unsigned char *ip, *greymap=vidPtr->image->greymap;
    register char *xip;
 
    if (vidPtr->scaler == 0) {
	ip = &vidPtr->image->data[y*imagewidth+x];
	xip = &vidPtr->ximage->data[(y*2+1)*ximagewidth+x*2];
	for (y1=y; y1<y+height; y1++) {
	    for (w=width; w > 0; w -= 4) {
		*xip++ = color_lut[greymap[(*(ip-1) + *ip)/2]];
		*xip++ = color_lut[greymap[*ip++]];
		*xip++ = color_lut[greymap[(*(ip-1) + *ip)/2]];
		*xip++ = color_lut[greymap[*ip++]];
		*xip++ = color_lut[greymap[(*(ip-1) + *ip)/2]];
		*xip++ = color_lut[greymap[*ip++]];
		*xip++ = color_lut[greymap[(*(ip-1) + *ip)/2]];
		*xip++ = color_lut[greymap[*ip++]];
	    }

	    if (x == 0) *(xip-width*2) = *(xip-width*2+1);
	    if (x+width < imagewidth)
                *xip = color_lut[greymap[(*(ip-1) + *ip)/2]];

	    ip -= width;
	    xip -= (ximagewidth+width*2);

	    if (y1 == 0) {
		memcpy(xip, xip+ximagewidth, width*2);
		ip += imagewidth;
		xip += ximagewidth*3;
	    } else {
		for (w=width; w > 0; w -= 4) {
		    *xip++ = color_lut[greymap[(*(ip-imagewidth-1) + *ip)/2]];
		    *xip++ = color_lut[greymap[(*(ip-imagewidth) + *ip)/2]];
		    ip++;
		    *xip++ = color_lut[greymap[(*(ip-imagewidth-1) + *ip)/2]];
		    *xip++ = color_lut[greymap[(*(ip-imagewidth) + *ip)/2]];
		    ip++;
		    *xip++ = color_lut[greymap[(*(ip-imagewidth-1) + *ip)/2]];
		    *xip++ = color_lut[greymap[(*(ip-imagewidth) + *ip)/2]];
		    ip++;
		    *xip++ = color_lut[greymap[(*(ip-imagewidth-1) + *ip)/2]];
		    *xip++ = color_lut[greymap[(*(ip-imagewidth) + *ip)/2]];
		    ip++;
		}
		if (x == 0) *(xip-width*2) = *(xip-width*2+1);
		if (x+width < imagewidth)
		    *xip = color_lut[greymap[(*(ip-imagewidth-1) + *ip)/2]];

		ip += imagewidth-width;
		xip += ximagewidth*3-width*2;
	    }
	}

	if (y1 < vidPtr->image->height) {
	    xip -= ximagewidth;
	    for (w=width; w > 0; w -= 4) {
		*xip++ = color_lut[greymap[(*(ip-imagewidth-1) + *ip)/2]];
		*xip++ = color_lut[greymap[(*(ip-imagewidth) + *ip)/2]];
		ip++;
		*xip++ = color_lut[greymap[(*(ip-imagewidth-1) + *ip)/2]];
		*xip++ = color_lut[greymap[(*(ip-imagewidth) + *ip)/2]];
		ip++;
		*xip++ = color_lut[greymap[(*(ip-imagewidth-1) + *ip)/2]];
		*xip++ = color_lut[greymap[(*(ip-imagewidth) + *ip)/2]];
		ip++;
		*xip++ = color_lut[greymap[(*(ip-imagewidth-1) + *ip)/2]];
		*xip++ = color_lut[greymap[(*(ip-imagewidth) + *ip)/2]];
		ip++;
	    }
	    if (x == 0) *(xip-width*2) = *(xip-width*2+1);
	    if (x+width < imagewidth)
		*xip = color_lut[greymap[(*(ip-imagewidth-1) + *ip)/2]];
	}
    } else {
	ip = &vidPtr->image->data[y*imagewidth+x];
	xip = &vidPtr->ximage->data[y/scaler*ximagewidth+x/scaler];
	while (height) {
	    for (w=width; w > 0; w -= 8*scaler) {
		*xip++ = color_lut[greymap[*ip]];
		ip += scaler;
		*xip++ = color_lut[greymap[*ip]];
		ip += scaler;
		*xip++ = color_lut[greymap[*ip]];
		ip += scaler;
		*xip++ = color_lut[greymap[*ip]];
		ip += scaler;
		*xip++ = color_lut[greymap[*ip]];
		ip += scaler;
		*xip++ = color_lut[greymap[*ip]];
		ip += scaler;
		*xip++ = color_lut[greymap[*ip]];
		ip += scaler;
		*xip++ = color_lut[greymap[*ip]];
		ip += scaler;
	    }
	    ip += imagewidth*scaler-width;
	    xip += ximagewidth-width/scaler;

	    height -= scaler;
	}
    }
}

static void VidWidget_Destroy(vidwidget)
    ClientData vidwidget;
{
    VideoWidget *vidPtr = (VideoWidget *) vidwidget;

    ckfree((char *) vidPtr);
}

static void VidWidget_Display(vidwidget)
    ClientData vidwidget;
{
    register VideoWidget *vidPtr = (VideoWidget *) vidwidget;
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

static int VidWidget_InstanceCmd(vidwidget, interp, argc, argv)
    ClientData vidwidget;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    register VideoWidget *vidPtr = (VideoWidget *) vidwidget;
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
    if ((c == 'c') && (strncmp(argv[1], "copy", length) == 0)) {
	result = VidWidget_Copy(interp, vidPtr, argc-2, argv+2);
    } else if ((c == 'r') && (strncmp(argv[1], "redraw", length) == 0)) {
	VidWidget_Redraw(vidPtr);
	result = TCL_OK;
    } else if ((c == 's') && (strncmp(argv[1], "scale", length) == 0)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " scale <scaler>\"", NULL);
	    return TCL_ERROR;
	} else {
	    if (vidPtr->scaler != atoi(argv[2])) {
		VidWidget_DestroyXImage(vidPtr);
		vidPtr->scaler = atoi(argv[2]);
		VidWidget_AllocXImage(vidPtr);
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

static void VidWidget_EventProc(vidwidget, eventPtr)
    ClientData vidwidget;
    register XEvent *eventPtr;
{
    register VideoWidget *vidPtr = (VideoWidget *) vidwidget;
    int i;

    if ((eventPtr->type == Expose) && (eventPtr->xexpose.count == 0)) {
	VidWidget_Redraw(vidPtr);
    } else if (eventPtr->type == DestroyNotify) {
	VidWidget_DestroyXImage(vidPtr);
	Tcl_DeleteCommand(vidPtr->interp, Tk_PathName(vidPtr->tkwin));
	vidPtr->tkwin = NULL;

	for (i=0; i<vidPtr->image->num_widgets; i++) {
	    if (vidPtr->image->widgetlist[i] == vidwidget) {
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

void VidWidget_Init(mainWindow)
    Tk_Window mainWindow;
{
    int i, screen;
    unsigned long planes[GREY_BITS], pixels[1], mask;
    static XColor colors[GREY_LEVELS];
    XSetWindowAttributes attr;
    Display *dpy = Tk_Display(mainWindow);

    if ((shm_available = XShmQueryExtension(dpy)) != 0)
	completion = XShmGetEventBase(dpy) + ShmCompletion;

    tkMainWin = mainWindow;
    screen = DefaultScreen(dpy);
    screenDepth = DefaultDepth(dpy, screen);
    screenVisual = DefaultVisual(dpy, screen);
    if (screenDepth == 1) {
	black_pix = bg_color = BlackPixel(dpy, screen);
	white_pix = WhitePixel(dpy, screen);
	bg_fill = black_pix * 0xff;
	return;
    } else if ((screenVisual->class != PseudoColor) &&
	       (screenVisual->class != GrayScale)) {
	fprintf(stderr, "Can't handle this type of display.\n");
	exit(1);
    }

    colmap = XDefaultColormap(dpy, screen);
    for (i=0; i<GREY_LEVELS; i++) {
	colors[i].red = colors[i].green = colors[i].blue =
	    i * 0xffff/(GREY_LEVELS-1);
	colors[i].flags = DoRed | DoGreen | DoBlue;
	if (XAllocColor(dpy, colmap, &colors[i]) == 0) {
	    fprintf(stderr, "Can't allocate colors\n");
	    exit(1);
	}
	color_lut[i] = colors[i].pixel;
    }
 
    bg_color = bg_fill = color_lut[GREY_LEVELS/2];
}

void *VidWidget_Create(interp, name, toplevel, image)
    Tcl_Interp *interp;
    char *name;
    int toplevel;
    vidimage_t *image;
{
    Tk_Window newwin;
    register VideoWidget *vidPtr;
    Tk_Uid screenUid;
    Display *dpy=Tk_Display(tkMainWin);
    XSetWindowAttributes attr;
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
    attr.colormap = colmap;
    Tk_ChangeWindowAttributes(newwin, CWColormap, &attr);

    vidPtr = (VideoWidget *) ckalloc(sizeof(VideoWidget));
    bzero((char *)vidPtr, sizeof(VideoWidget));
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

    vidPtr->scaler = 1;
    VidWidget_AllocXImage(vidPtr);
    VidWidget_Redraw(vidPtr);

    image->widgetlist[image->num_widgets++] = (ClientData)vidPtr;
    return (ClientData)vidPtr;
}

void VidWidget_Redraw(vidwidget)
    void *vidwidget;
{
    register VideoWidget *vidPtr = (VideoWidget *) vidwidget;

    if ((vidPtr->tkwin != NULL) && !(vidPtr->flags & REDRAW_PENDING)) {
	Tk_DoWhenIdle(VidWidget_Display, (ClientData) vidPtr);
	vidPtr->flags |= REDRAW_PENDING;
    }
}

void VidWidget_UpdateRect(vidwidget, x, y, width, height)
    ClientData vidwidget;
    register int x, y, width, height;
{
    register VideoWidget *vidPtr = (VideoWidget *) vidwidget;
    register int scaler=vidPtr->scaler, alignmask;

    if (scaler == 0) {
	alignmask = 3;
    } else {
	alignmask = 8*scaler-1;
    }

    if (x & alignmask) {
	width += x & alignmask;
	x -= x & alignmask;
    }

    if (width & alignmask) width += alignmask+1 - (width & alignmask);

    if (screenDepth == 1)
	VidWidget_UpdateMono(vidPtr, x, y, width, height);
    else
	VidWidget_UpdateGrey(vidPtr, x, y, width, height);
}
