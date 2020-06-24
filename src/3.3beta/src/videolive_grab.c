/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	HP VideoLive frame grab routines

	Derived from code provided by John Brezak <brezak@apollo.hp.com>, based
	on earlier work by Geir Pederen <Geir.Pedersen@usit.uio.no> and the
	PIP grabber written by Steve McCanne <mccanne@ee.lbl.gov>.
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

#ifdef VIDEOLIVE
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/param.h>
#ifdef VIDEOLIVE_DIRECT
#include <sys/fcntl.h>
#include <sys/iomap.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>
#include <tk.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_util.h"
#include "vid_code.h"
#include "videolive_grab.h"

extern Tk_Window tkMainWin;
extern Tcl_Interp *interp;

static Atom XAbrightness, XAcontrast, XAhue, XAsaturation, XAencoding;

static Display *dpy;
static Window root, w;
static GC gc;
static Colormap cmap;
static int xmit_size, xmit_color, full_width, full_height, width, height;
static reconfigproc_t *reconfig;
static XvPortID grabID;
static ximage_t *ximage;
static unsigned int nencodings;
static XvEncodingInfo *encoding;
static Tk_Window camera;

#ifdef VIDEOLIVE_DIRECT
#define ROPS_RAMDAC	0x400
#define ROPS_REGS	0x200
#define ROPS_VENUS	0x200

static struct video_rops {
    uint32 *fb_addr;
    uint32 *regs_addr;
    uint32 ramdac;
    uint32 regs;
    uint32 venus;
} video;
#endif

static XvEncodingInfo *LookupEncodingByName(char *name)
{
    int i;
    
    for (i=0; i<nencodings; i++) {
	if (strncmp(name, encoding[i].name, strlen(name)) == 0)
	    return &encoding[i];
    }

    return NULL;
}

static void VideoLive_Reconfig(int w, int h)
{
    full_width = w;
    full_height = h;

    switch (xmit_size) {
    case VID_SMALL:
	width = w/4;
	height = h/4;
	break;
    case VID_MEDIUM:
	width = w/2;
	height = h/2;
	break;
    case VID_LARGE:
	width = w;
	height = h;
	break;
    }

    if (reconfig) (*reconfig)(xmit_color, width, height);
}

/*ARGSUSED*/
static int PopCamera(ClientData clientData, XEvent *eventPtr)
{
    XVisibilityEvent *event = &eventPtr->xvisibility;
	
    if (eventPtr->type == VisibilityNotify && event->window == w) {
	if (event->state != VisibilityUnobscured) {
	    XRaiseWindow(event->display, event->window);
	    XFlush(event->display);
	    return True;
	}
    }

    return False;
}

static void VideoLive_GrabColor(uint8 *y_data, int8 *uv_data)
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
 
#ifdef __hpux
            yp[0] = ry[((p0>>9) & 0x7c00)+((p0>>6) & 0x3e0)+((p0>>3) & 0x1f)];
            yp[1] = ry[((p1>>9) & 0x7c00)+((p1>>6) & 0x3e0)+((p1>>3) & 0x1f)];
            yp += 2;
 
            p0 &= 0xfefeff;
            p1 &= 0xfefeff;
            p0 += p1;
            *uvp++ = ruv[((p0>>10) & 0x7c00) +
                         ((p0>>7) & 0x3e0) +
                         ((p0>>4) & 0x1f)];
#else
	    yp[0] = ry[((p0<<7) & 0x7c00)+((p0>>6) & 0x3e0)+((p0>>19) & 0x1f)];
	    yp[1] = ry[((p1<<7) & 0x7c00)+((p1>>6) & 0x3e0)+((p1>>19) & 0x1f)];
	    yp += 2;
 
	    p0 &= 0xfefeff;
	    p1 &= 0xfefeff;
	    p0 += p1;
	    *uvp++ = ruv[((p0<<6) & 0x7c00) +
			 ((p0>>7) & 0x3e0) +
			 ((p0>>20) & 0x1f)];
#endif
	}
    }
}

static void VideoLive_GrabGrey(uint8 *y_data)
{
    int x, y;
    uint8 *yp=y_data, *ry=rgb2y;
    uint32 p0, p1, *data=(uint32 *)ximage->image->data;

    for (y=0; y<height; y++) {
	for (x=0; x<width; x+=2) {
	    p0 = data[0];
	    p1 = data[1];
	    data += 2;

#ifdef __hpux
            yp[0] = ry[((p0>>9) & 0x7c00)+((p0>>6) & 0x3e0)+((p0>>3) & 0x1f)];
            yp[1] = ry[((p1>>9) & 0x7c00)+((p1>>6) & 0x3e0)+((p1>>3) & 0x1f)];
#else
	    yp[0] = ry[((p0<<7) & 0x7c00)+((p0>>6) & 0x3e0)+((p0>>19) & 0x1f)];
	    yp[1] = ry[((p1<<7) & 0x7c00)+((p1>>6) & 0x3e0)+((p1>>19) & 0x1f)];
#endif
	    yp += 2;
	}
    }
}

static int VideoLive_GrabFrame(uint8 *y_data, int8 *uv_data)
{
    if (xmit_color)
	VideoLive_GrabColor(y_data, uv_data);
    else
	VideoLive_GrabGrey(y_data);

    return 1;
}

#ifdef VIDEOLIVE_DIRECT
static void VideoLive_GrabColorDirect(uint8 *y_data, int8 *uv_data,
				      uint32 *data)
{
    int x, y;
    uint8 *yp=y_data, *ry=rgb2y;
    uint16 *uvp=(uint16 *)uv_data, *ruv=rgb2uv;
    register uint32 p0, p1;
 
    for (y=0; y<height; y++) {
	for (x=0; x<width; x+=2) {
	    p0 = data[0];
	    p1 = data[1];
	    data += 2;
 
	    yp[0] = ry[((p0>>9) & 0x7c00)+((p0>>6) & 0x3e0)+((p0>>3) & 0x1f)];
	    yp[1] = ry[((p1>>9) & 0x7c00)+((p1>>6) & 0x3e0)+((p1>>3) & 0x1f)];
	    yp += 2;
 
	    p0 &= 0xfefeff;
	    p1 &= 0xfefeff;
	    p0 += p1;
	    *uvp++ = ruv[((p0>>10) & 0x7c00) +
			 ((p0>>7) & 0x3e0) +
			 ((p0>>4) & 0x1f)];
	}
	data += 896 - width;	
    }
}

static void VideoLive_GrabGreyDirect(uint8 *y_data, uint32 *data)
{
    int x, y;
    uint8 *yp=y_data, *ry=rgb2y;
    register uint32 p0, p1;

    for (y=0; y<height; y++) {
	for (x=0; x<width; x+=2) {
	    p0 = data[0];
	    p1 = data[1];
	    data += 2;

	    yp[0] = ry[((p0>>9) & 0x7c00)+((p0>>6) & 0x3e0)+((p0>>3) & 0x1f)];
	    yp[1] = ry[((p1>>9) & 0x7c00)+((p1>>6) & 0x3e0)+((p1>>3) & 0x1f)];
	    yp += 2;
	}
	data += 896-width;	
    }
}

static int VideoLive_GrabFrameDirect(uint8 *y_data, int8 *uv_data)
{
    if (xmit_color)
	VideoLive_GrabColorDirect(y_data, uv_data, video.fb_addr+4);
    else
	VideoLive_GrabGreyDirect(y_data, video.fb_addr+4);

    return 1;
}
#endif

/*ARGSUSED*/
static char *VideoLive_SetAttr(ClientData clientData, Tcl_Interp *interp,
			       char *name1, char *name2, int flags)
{
    int value;
    
    value = atoi(Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY));
    XvSetPortAttribute(dpy, grabID, (Atom) clientData, value*10);
    return NULL;
}

/*ARGSUSED*/
static char *VideoLive_SetEncoding(ClientData clientData, Tcl_Interp *interp,
				   char *name1, char *name2, int flags)
{
    char *std, *input, encoding_name[256];
    XvEncodingInfo *encoding_info;
    
    std = Tcl_GetVar(interp, "vliveStd", TCL_GLOBAL_ONLY);
    input = Tcl_GetVar(interp, "vliveInput", TCL_GLOBAL_ONLY);

    sprintf(encoding_name, "%s-%s", std, input);
    encoding_info = LookupEncodingByName(encoding_name);
    if (encoding_info != NULL) {
	XvSetPortAttribute(dpy, grabID, XAencoding, encoding_info->encoding_id);
	VideoLive_Reconfig(encoding_info->width, encoding_info->height);
    }    

    return NULL;
}

static void VideoLive_SetControls(int enable)
{
    int brightness, contrast, hue, saturation, encoding_id;
    char cmd[256];

    if (enable) {
	Tcl_TraceVar(interp, "vliveBrightness", TCL_TRACE_WRITES,
		     VideoLive_SetAttr, (ClientData) XAbrightness);
	Tcl_TraceVar(interp, "vliveContrast", TCL_TRACE_WRITES,
		     VideoLive_SetAttr, (ClientData) XAcontrast);
	Tcl_TraceVar(interp, "vliveHue", TCL_TRACE_WRITES,
		     VideoLive_SetAttr, (ClientData) XAhue);
	Tcl_TraceVar(interp, "vliveSaturation", TCL_TRACE_WRITES,
		     VideoLive_SetAttr, (ClientData) XAsaturation);
	Tcl_TraceVar(interp, "vliveStd", TCL_TRACE_WRITES,
		     VideoLive_SetEncoding, NULL);
	Tcl_TraceVar(interp, "vliveInput", TCL_TRACE_WRITES,
		     VideoLive_SetEncoding, NULL);

	XvGetPortAttribute(dpy, grabID, XAbrightness, &brightness);
	XvGetPortAttribute(dpy, grabID, XAcontrast, &contrast);
	XvGetPortAttribute(dpy, grabID, XAhue, &hue);
	XvGetPortAttribute(dpy, grabID, XAsaturation, &saturation);
	XvGetPortAttribute(dpy, grabID, XAencoding, &encoding_id);

	sprintf(cmd, "set vliveBrightness %d", brightness/10);
	Tcl_Eval(interp, cmd);
	sprintf(cmd, "set vliveContrast %d", contrast/10);
	Tcl_Eval(interp, cmd);
	sprintf(cmd, "set vliveHue %d", hue/10);
	Tcl_Eval(interp, cmd);
	sprintf(cmd, "set vliveSaturation %d", saturation/10);
	Tcl_Eval(interp, cmd);

	if (Tcl_Eval(interp, "videoliveGrabEnableControls") != TCL_OK)
	    printf("videoliveGrabEnableControls failed: %s\n", interp->result);
    } else {
	(void) Tcl_Eval(interp, "videoliveGrabDisableControls");

	Tcl_UntraceVar(interp, "vliveBrightness", TCL_TRACE_WRITES,
		       VideoLive_SetAttr, (ClientData) XAbrightness);
	Tcl_UntraceVar(interp, "vliveContrast", TCL_TRACE_WRITES,
		       VideoLive_SetAttr, (ClientData) XAcontrast);
	Tcl_UntraceVar(interp, "vliveHue", TCL_TRACE_WRITES,
		       VideoLive_SetAttr, (ClientData) XAhue);
	Tcl_UntraceVar(interp, "vliveSaturation", TCL_TRACE_WRITES,
		       VideoLive_SetAttr, (ClientData) XAsaturation);
	Tcl_UntraceVar(interp, "vliveStd", TCL_TRACE_WRITES,
		       VideoLive_SetEncoding, NULL);
	Tcl_UntraceVar(interp, "vliveInput", TCL_TRACE_WRITES,
		       VideoLive_SetEncoding, NULL);
    }
}

#ifdef VIDEOLIVE_DIRECT
static int IsLocal(void)
{
    char *dp, *pp;
    char thishost[MAXHOSTNAMELEN];
    
    _XGetHostname(thishost, sizeof(thishost));

    dp = strdup(XDisplayName(dpy));
    pp = index(dp, ':');
    *pp = '\0';

    if (strlen(dp) == 0 || strcmp(dp, "unix") == 0 ||
	strcmp(dp, thishost) == 0) {
	free(dp);
	return 1;
    } else {
	free(dp);
	return 0;
    }
}

static grabproc_t *VideoLive_MapDirect(void)
{
    char video_dev[MAXPATHLEN];
    static char *video_dev_base = "/dev/hptv0";	/* XXX Get from /usr/local/rops/etc/tvtab */
    static int fd;

    /*
     * If not running on a local display, don't use the direct access method
     */
    if (!IsLocal()) return VideoLive_GrabFrame;

    /*
     * Only init once
     */
    if (fd) return VideoLive_GrabFrameDirect;

    /*
     * Map true-color frame buffer and registers
     */
    sprintf(video_dev, "%s_tc", video_dev_base);
    if ((fd = open(video_dev, O_RDWR)) < 0) {
	perror("nv: Unable to open video frame buffer");
	return VideoLive_GrabFrame;
    }

    if (ioctl(fd, IOMAPMAP, &video.fb_addr) < 0){
	perror("nv: ioctl: IOMAPMAP");
	return VideoLive_GrabFrame;
    }

    sprintf(video_dev, "%s_reg", video_dev_base);
    if ((fd = open(video_dev, O_RDWR)) < 0) {
	perror("nv: Unable to open video registers");
	return VideoLive_GrabFrame;
    }

    if (ioctl(fd, IOMAPMAP, &video.regs_addr) < 0){
	perror("nv: ioctl: IOMAPMAP");
	return VideoLive_GrabFrame;
    }

    video.ramdac = (u_int)video.regs_addr + ROPS_RAMDAC;
    video.regs   = (u_int)video.ramdac + ROPS_REGS;
    video.venus  = (u_int)video.regs + ROPS_VENUS;

    return VideoLive_GrabFrameDirect;
}
#endif

int VideoLive_Probe(void)
{
    unsigned int ngrabbers;
    int i, majop, eventbase, errbase;
    XvAdaptorInfo *grabber, *g;
    char cmd[256], *std, *fmt;

    dpy = Tk_Display(tkMainWin);
    root = DefaultRootWindow(dpy);

    if (XQueryExtension(dpy, "XVideo", &majop, &eventbase, &errbase) == False)
	return 0;

    XvQueryAdaptors(dpy, root, &ngrabbers, &grabber);

    for (i=0; i<ngrabbers; i++)
	if (grabber[i].type & XvInputMask) break;
    if (i == ngrabbers) return 0;

    grabID = grabber[i].base_id;

    /* If not RasterOps, fail */
    if (strncmp(grabber[i].name, "RasterOps", 9) != 0) {
	XvFreeAdaptorInfo(grabber);
	return 0;
    }

    XvFreeAdaptorInfo(grabber);
    
    XvQueryEncodings(dpy, grabID, &nencodings, &encoding);

    XAbrightness = XInternAtom(dpy, "XV_BRIGHTNESS", False);
    XAcontrast = XInternAtom(dpy, "XV_CONTRAST", False);
    XAhue = XInternAtom(dpy, "XV_HUE", False);
    XAsaturation = XInternAtom(dpy, "XV_SATURATION", False);
    XAencoding = XInternAtom(dpy, "XV_ENCODING", False);
    
    /* XXX: Default to ntsc-composite since encoding seems confused */
    sprintf(cmd, "set vliveStd ntsc");
    Tcl_Eval(interp, cmd);
    sprintf(cmd, "set vliveInput composite");
    Tcl_Eval(interp, cmd);

    return VID_SMALL|VID_MEDIUM|VID_LARGE|VID_GREYSCALE|VID_COLOR;
}

char *VideoLive_Attach(void)
{
    return ".grabControls.videolive";
}

void VideoLive_Detach(void)
{
}

grabproc_t *VideoLive_Start(int max_framerate, int config, reconfigproc_t *r)
{
    int screen=Tk_ScreenNumber(tkMainWin), mask;
    XVisualInfo vinfo;
    XSetWindowAttributes attr;
    XColor sc, ec;
    int depth=24;
    char *std, *input, encoding_name[256];
    XvEncodingInfo *encoding_info;

    if (XvGrabPort(dpy, grabID, CurrentTime) != Success) {
	Tcl_AppendResult(interp, "VideoLive board busy.\n", NULL);
	return NULL;
    }

    reconfig = r;
    xmit_size = (config & VID_SIZEMASK);
    xmit_color = (config & VID_COLOR);

    std = Tcl_GetVar(interp, "vliveStd", TCL_GLOBAL_ONLY);
    input = Tcl_GetVar(interp, "vliveInput", TCL_GLOBAL_ONLY);
    sprintf(encoding_name, "%s-%s", std, input);

    encoding_info = LookupEncodingByName(encoding_name);
    VideoLive_Reconfig(encoding_info->width, encoding_info->height);

    if (XMatchVisualInfo(dpy, screen, 24, TrueColor, &vinfo) == 0) {
#ifdef __hpux
	depth = 8;
	vinfo.visual = DefaultVisual(dpy, screen);
#else
	fprintf(stderr, "Can't find a TrueColor visual with depth 24.\n");
	return 0;
#endif
    }

    attr.colormap = cmap = XCreateColormap(dpy, root, vinfo.visual, AllocNone);
    XAllocNamedColor(dpy, cmap, "black", &sc, &ec);
    attr.background_pixel = attr.border_pixel = sc.pixel;
    
    mask = CWBackPixel | CWBorderPixel | CWColormap;

    w = XCreateWindow(dpy, root, 0, 0, width, height, 1, depth, InputOutput,
		      vinfo.visual, mask, &attr);
    XSetStandardProperties(dpy, w, "Local camera", "Camera", 0, 0, 0, 0);
    XSelectInput(dpy, w, VisibilityChangeMask);
    Tk_CreateGenericHandler(PopCamera, NULL);
    gc = XCreateGC(dpy, w, 0, 0);
    XFlush(dpy);
    XMapWindow(dpy, w);

    XvPutVideo(dpy, grabID, w, gc, 0, 0, full_width, full_height,
	       0, 0, width, height);

    VideoLive_SetControls(True);
    
#ifdef __hpux
    /*
     * The HP Xv extension patches XGetImage so that you can
     * use a 8-bit pseudo-color visual and get back a 24-bit
     * true-color image. Unfortunately they didn't patch
     * XshmGetImage(), so you must allocate this as a regular
     * image. Ugh!
     */
    ximage = VidUtil_AllocStdXImage(dpy, vinfo.visual, 24, width, height);
#else
    ximage = VidUtil_AllocXImage(dpy, vinfo.visual, 24, width, height, False);
#endif
    
#ifdef VIDEOLIVE_DIRECT
    return VideoLive_MapDirect();
#else
    return VideoLive_GrabFrame;
#endif
}

void VideoLive_Stop(void)
{
    VideoLive_SetControls(False);
    VidUtil_DestroyXImage(dpy, ximage);
    XvStopVideo(dpy, grabID, w);
    Tk_DeleteGenericHandler(PopCamera, NULL);
    XFreeGC(dpy, gc);
    XDestroyWindow(dpy, w);
    XFreeColormap(dpy, cmap);
    ximage = NULL;
    reconfig = NULL;
    XvUngrabPort(dpy, grabID, CurrentTime);
}
#endif /* VIDEOLIVE */
