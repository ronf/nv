/*
        Netvideo version 3.3
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
#include <netinet/in.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <tk.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_util.h"
#include "vid_widget.h"

#define GREY_BITS	5
#define GREY_LEVELS	(1 << (GREY_BITS))

uint32 bg_color, bg_fill, black_pix, white_pix;
uint8 y_dither8[256*16], yuv_dither8[65536*16];
uint32 y_cmap[256], yuv_cmap[65536];

static int screenDepth, private_map=0;
static Visual *visual;
static Colormap colmap;

static uint8 grey_dither[16] =
    { 6, 2, 7, 3,
      1, 5, 0, 4,
      7, 3, 6, 2,
      0, 4, 1, 5 };

static uint32 color_dither[16] =
  { 0x180a00, 0x080010, 0x1c0804, 0x0c0214,
    0x060618, 0x160c08, 0x02041c, 0x120e0c,
    0x1e0906, 0x0e0316, 0x1a0b02, 0x0a0112,
    0x00051e, 0x100f0e, 0x04071a, 0x140d0a };

static Colormap VidWidget_AllocPrivateMap(Display *dpy, Colormap oldmap)
{
    int i;
    Colormap map;
    XColor reserved[32];

    if (private_map) {
	fprintf(stderr, "Can't allocate color cube!");
	exit(1);
	/*NOTREACHED*/
    } else {
	private_map = 1;
	map = XCopyColormapAndFree(dpy, oldmap);
	for (i=0; i<32; i++) reserved[i].pixel = i;
	XQueryColors(dpy, oldmap, reserved, 32);
	for (i=0; i<32; i++) XAllocColor(dpy, map, &reserved[i]);
	return map;
    }
}

static void VidWidget_SetColormap(Tk_Window w)
{
    Tk_Window parent;

    parent = w;
    while (!Tk_IsTopLevel(parent)) parent = Tk_Parent(parent);

    if ((Tk_Colormap(parent) != Tk_Colormap(w)) &&
	(Tk_Visual(parent) == Tk_Visual(w)) &&
	(Tk_Depth(parent) == Tk_Depth(w)))
	Tk_SetWindowColormap(parent, Tk_Colormap(w));
}

static void VidWidget_AutoScale(vidwidget_t *vidPtr)
{
    int width=vidPtr->image->width, height=vidPtr->image->height, scalebits=0;
    int maxwidth=vidPtr->maxwidth, maxheight=vidPtr->maxheight;

    if ((maxwidth != 0) && (maxheight != 0)) {
	while ((width > maxwidth) || (height > maxheight)) {
	    scalebits++;
	    width /= 2;
	    height /= 2;
	}
	vidPtr->scalebits = scalebits;
    }
}

static void VidWidget_SetUpdateRoutine(vidwidget_t *vidPtr)
{
    vidimage_t *image=vidPtr->image;
    int color = (image->flags & VIDIMAGE_ISCOLOR) &&
		(image->flags & VIDIMAGE_WANTCOLOR);

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
    Display *dpy=Tk_Display(vidPtr->tkwin);
    vidimage_t *image=vidPtr->image;
    int scalebits=vidPtr->scalebits, width, height;

    if (scalebits == -1) {
	width = image->width*2;
	height = image->height*2;
    } else {
	width = image->width >> scalebits;
	height = image->height >> scalebits;
    }

    Tk_GeometryRequest(vidPtr->tkwin, width, height);
    vidPtr->ximage = VidUtil_AllocXImage(dpy, visual, screenDepth, width,
					 height, True);
    VidWidget_UpdateRect(vidPtr, 0, 0, image->width, image->height);
}

static void VidWidget_DestroyXImage(vidwidget_t *vidPtr)
{
    VidUtil_DestroyXImage(Tk_Display(vidPtr->tkwin), vidPtr->ximage);
    vidPtr->ximage = NULL;
}

static int VidWidget_Copy(Tcl_Interp *interp, vidwidget_t *vidPtr, int argc,
			  char **argv)
{
    Display *dpy=Tk_Display(vidPtr->tkwin);
    XImage *image=vidPtr->ximage->image;
    Tk_Window destwin;
    Pixmap pixmap;

    if (argc != 1) {
	Tcl_AppendResult(interp, "wrong # args", NULL);
	return TCL_ERROR;
    } else if (!(destwin = Tk_NameToWindow(interp, argv[0], vidPtr->tkwin))) {
	Tcl_AppendResult(interp, "invalid window name", NULL);
	return TCL_ERROR;
    }

    pixmap = XCreatePixmap(dpy, Tk_WindowId(vidPtr->tkwin), image->width,
			   image->height, screenDepth);
    XPutImage(dpy, pixmap, vidPtr->gc, image, 0, 0, 0, 0, image->width,
	      image->height);
    Tk_GeometryRequest(destwin, image->width, image->height);
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

static int VidWidget_InstanceCmd(ClientData vidwidget, Tcl_Interp *interp,
				 int argc, char **argv)
{
    vidwidget_t *vidPtr = (vidwidget_t *) vidwidget;
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
    vidwidget_t *vidPtr = (vidwidget_t *) vidwidget;
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

	Tk_EventuallyFree((ClientData) vidPtr, VidWidget_Destroy);
    }
}

#define YUVLIM(x) (((x) < 0) ? 0 : ((x) > 127) ? 127 : (x))

int VidWidget_Init(Display *dpy)
{
    int i, j, screen, r, g, b, r0, g0, b0, y, u, v, u0, v0;
    uint8 *gd=grey_dither;
    uint32 basergb, rgb, *cd=color_dither;
    XVisualInfo visinfo;
    XColor color;
    static u_long small_lut[128], red_lut[128], green_lut[128], blue_lut[128];
    static uint8 grey_lut[2*GREY_LEVELS], color_lut[4096];

    screen = DefaultScreen(dpy);
    if ((DefaultDepth(dpy, screen) == 24)||(DefaultDepth(dpy, screen) == 32)) {
	screenDepth = DefaultDepth(dpy, screen);
	visual = DefaultVisual(dpy, screen);
	(void) XMatchVisualInfo(dpy, screen, screenDepth, visual->class,
	    &visinfo);
	colmap = XDefaultColormap(dpy, screen);
    } else if (XMatchVisualInfo(dpy, screen, 32, TrueColor, &visinfo)) {
	screenDepth = 32;
	visual = visinfo.visual;
	colmap = XCreateColormap(dpy, RootWindow(dpy, screen), visual,
	    AllocNone);
    } else if (XMatchVisualInfo(dpy, screen, 32, DirectColor, &visinfo)) {
	screenDepth = 32;
	visual = visinfo.visual;
	colmap = XCreateColormap(dpy, RootWindow(dpy, screen), visual,
	    AllocNone);
    } else if (XMatchVisualInfo(dpy, screen, 24, TrueColor, &visinfo)) {
	screenDepth = 24;
	visual = visinfo.visual;
	colmap = XCreateColormap(dpy, RootWindow(dpy, screen), visual,
	    AllocNone);
    } else if (XMatchVisualInfo(dpy, screen, 24, DirectColor, &visinfo)) {
	screenDepth = 24;
	visual = visinfo.visual;
	colmap = XCreateColormap(dpy, RootWindow(dpy, screen), visual,
	    AllocNone);
    } else {
	screenDepth = DefaultDepth(dpy, screen);
	visual = DefaultVisual(dpy, screen);
	(void) XMatchVisualInfo(dpy, screen, screenDepth, visual->class,
	    &visinfo);
	colmap = XDefaultColormap(dpy, screen);
    }

    switch (screenDepth) {
    case 1:
	black_pix = bg_color = BlackPixel(dpy, screen);
	white_pix = WhitePixel(dpy, screen);
	bg_fill = black_pix * 0xff;
	if (LITTLEENDIAN) {
	    black_pix <<= 7;
	    white_pix <<= 7;
	}
	return 0;
    case 8:
	for (i=1; i<=GREY_LEVELS; i++) {
	    color.red = color.green = color.blue = i * 65536/GREY_LEVELS - 1;
	    color.flags = DoRed | DoGreen | DoBlue;
	    if (XAllocColor(dpy, colmap, &color) == 0) {
		colmap = VidWidget_AllocPrivateMap(dpy, colmap);
		i--;
	    }
	    grey_lut[i] = color.pixel;
	}
	grey_lut[0] = grey_lut[1];
	for (i=GREY_LEVELS+1; i<2*GREY_LEVELS; i++)
	    grey_lut[i] = grey_lut[GREY_LEVELS];
     
	bg_color = bg_fill = grey_lut[GREY_LEVELS/2];

	i = 0;
	for (y=0; y<256; y++)
	    for (j=0; j<16; j++)
		y_dither8[i++] = grey_lut[(y+gd[j]) >> (8-GREY_BITS)];

	i = 0;
	for (b=0; b<4; b++) {
	    for (g=0; g<8; g++) {
		for (r=0; r<4; r++) {
		    color.red = r * 16384 + 16383;
		    color.green = g * 8192 + 8191;
		    color.blue = b * 16384 + 16383;
		    color.flags = DoRed | DoGreen | DoBlue;
		    if (XAllocColor(dpy, colmap, &color) == 0) {
			colmap = VidWidget_AllocPrivateMap(dpy, colmap);
			r--;
		    } else {
			small_lut[i++] = color.pixel;
		    }
		}
	    }
	}

	i = 0;
	for (b=0; b<16; b++) {
	    b0 = (b < 2) ? 0 : (b < 8) ? b/2-1 : 3;
	    for (g=0; g<16; g++) {
		g0 = (g < 1) ? 0 : (g < 8) ? g-1 : 7;
		for (r=0; r<16; r++) {
		    r0 = (r < 2) ? 0 : (r < 8) ? r/2-1 : 3;
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
		    basergb = (YUVLIM(b)<<16) + (YUVLIM(g)<<8) + YUVLIM(r);
		    for (j=0; j<16; j++) {
			rgb = basergb+cd[j];
			rgb = ((rgb>>12) & 0xf00) + ((rgb>>8) & 0xf0) +
			      ((rgb>>4) & 0xf);
			yuv_dither8[i++] = color_lut[rgb];
		    }
		}
	    }
	}
	return 1;
    case 16:
    case 24:
    case 32:
	for (i=0; i<128; i++) {
	    color.red = color.green = color.blue = i*512 + 256;
	    color.flags = DoRed | DoGreen | DoBlue;
	    if (XAllocColor(dpy, colmap, &color) == 0) {
		colmap = VidWidget_AllocPrivateMap(dpy, colmap);
		i--;
	    }
	    red_lut[i] = color.pixel & visinfo.red_mask;
	    green_lut[i] = color.pixel & visinfo.green_mask;
	    blue_lut[i] = color.pixel & visinfo.blue_mask;
	    y_cmap[2*i] = y_cmap[2*i+1] = color.pixel;
	}

	bg_color = bg_fill = y_cmap[128];

	i = 0;
	for (u=0; u<128; u+=4) {
	    for (v=0; v<128; v+=4) {
		for (y=0; y<128; y+=2) {
		    u0 = (u < 64) ? u : (u-128);
		    v0 = (v < 64) ? v : (v-128);
		    r = y + 11*v0/8;
		    g = y - 11*u0/32 - 45*v0/64;
		    b = y + 111*u0/64;
		    yuv_cmap[i++] = red_lut[YUVLIM(r)] +
				    green_lut[YUVLIM(g)] +
				    blue_lut[YUVLIM(b)];
		}
	    }
	}
	return 1;
    default:
	fprintf(stderr, "Can't handle this type of display.\n");
	exit(1);
	/*NOTREACHED*/
    }
}

void *VidWidget_Create(void *interp, char *name, int toplevel, void *parent,
		       vidimage_t *image, int maxwidth, int maxheight)
{
    vidwidget_t *vidPtr;
    Tk_Window newwin;
    Tk_Uid screenUid;
    XGCValues gcValues;

    if (image->num_widgets == MAX_WIDGETS) return NULL;

    if (toplevel) {
	screenUid = Tk_GetUid("");
    } else {
	screenUid = NULL;
    }

    newwin = Tk_CreateWindowFromPath(interp, (Tk_Window) parent, name,
				     screenUid);
    if (newwin == NULL) return NULL;

    Tk_SetClass(newwin, "VideoWidget");
    Tk_SetWindowVisual(newwin, visual, screenDepth, colmap);
    VidWidget_SetColormap(newwin);

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

    vidPtr->maxwidth = maxwidth;
    vidPtr->maxheight = maxheight;
    VidWidget_AutoScale(vidPtr);

    VidWidget_AllocXImage(vidPtr);
    VidWidget_SetUpdateRoutine(vidPtr);
    VidWidget_UpdateRect(vidPtr, 0, 0, image->width, image->height);
    VidWidget_Redraw(vidPtr);

    image->widgetlist[image->num_widgets++] = vidPtr;
    return vidPtr;
}

void VidWidget_Redraw(void *vidwidget)
{
    vidwidget_t *vidPtr = vidwidget;
    Tk_Window tkwin=vidPtr->tkwin;

    if ((tkwin != NULL) && Tk_IsMapped(tkwin))
	VidUtil_PutXImage(Tk_Display(tkwin), Tk_WindowId(tkwin), vidPtr->gc,
			  0, 0, vidPtr->ximage);
}

void VidWidget_Resize(void *vidwidget)
{
    vidwidget_t *vidPtr = vidwidget;
    vidimage_t *image=vidPtr->image;

    VidWidget_AutoScale(vidPtr);
    VidWidget_DestroyXImage(vidPtr);
    VidWidget_AllocXImage(vidPtr);
    VidWidget_UpdateRect(vidPtr, 0, 0, image->width, image->height);
    VidWidget_Redraw(vidPtr);
}

void VidWidget_SetColor(void *vidwidget)
{
    vidwidget_t *vidPtr = vidwidget;
    vidimage_t *image=vidPtr->image;

    VidWidget_SetUpdateRoutine(vidPtr);
    VidWidget_UpdateRect(vidPtr, 0, 0, image->width, image->height);
    VidWidget_Redraw(vidPtr);
}

void VidWidget_SetMaxSize(void *vidwidget, int maxwidth, int maxheight)
{
    vidwidget_t *vidPtr = vidwidget;

    vidPtr->maxwidth = maxwidth;
    vidPtr->maxheight = maxheight;
    VidWidget_Resize(vidPtr);
}

void VidWidget_UpdateRect(void *vidwidget, int x, int y, int width, int height)
{
    vidwidget_t *vidPtr = vidwidget;
    int scalebits=vidPtr->scalebits, scaler=1<<scalebits, alignmask;

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
