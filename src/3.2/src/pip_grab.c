/*
	Netvideo version 3.1
	Written by Ron Frederick <frederick@parc.xerox.com>

	pip frame grabber for decstation

	Derived from code written by Eric Anderson, UCSD Computer Systems Lab,
	4/92 and hacked on by Jon Kay, Vach Kompella, Kevin Fall.
	Adapted for nv and hacked beyond recognition by
	Steven McCanne (mccanne@ee.lbl.gov) March 1993.
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

#ifdef PIP_GRABBER

#include <stdio.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>
#include <tk.h>

int (*GrabImage)(u_char *y_data, signed char *uv_data);

static Display *disp;

XvPortID grabID;

struct wininfo {
	Window win;
	Visual	*visual;/* pointer to visual */
	XVisualInfo	 vinfo; /* info on visual (or template) */
	GC		 gc;	/* graphics context */
	Colormap	 cmap;	/* colormap */
	int		 x,y;	/* origin relative to parent */
};

struct wininfo caminfo;

Window
createXwindow(int w, int  h, char *wname, char *iname, struct wininfo *info)
{
	XVisualInfo *vList;
	int visualsMatched;
	XSizeHints size_hints;
	int screen;
	u_int border_width = 2;
	Window win, root;
	XSetWindowAttributes attr;
  
	/* Get screen size from display structure macro */
	info->vinfo.screen = screen = DefaultScreen(disp);
	root = DefaultRootWindow(disp);

	if (info->visual == 0) {
		vList = XGetVisualInfo(disp, VisualScreenMask|VisualDepthMask 
				       | VisualClassMask, 
				       &info->vinfo, &visualsMatched);
		if (visualsMatched == 0) {
			fprintf(stderr, "cannot find suitable visual\n");
			exit(-1);
		}
		info->visual = vList[0].visual;
		/* store info about Visual */
		info->vinfo = vList[0];

		/* each visual requires a different colormap */
		info->cmap = XCreateColormap(disp, root, info->visual,
					     AllocNone);
	}
	attr.colormap = info->cmap;
	attr.event_mask = VisibilityChangeMask;
	attr.backing_store = Always;
	attr.border_pixel = BlackPixel(disp, screen);
	attr.background_pixel = BlackPixel(disp, screen);
	attr.override_redirect = True;

	win = XCreateWindow(disp, root, info->x, info->y, w, h,
			    border_width, info->vinfo.depth, InputOutput, 
			    info->visual,
			    CWColormap | CWBorderPixel | CWBackPixel |
			    CWEventMask | CWBackingStore | CWOverrideRedirect,
			    &attr);

	/* Initialize size hint property for window manager */
	size_hints.flags  = PSize;
	size_hints.width  = w;
	size_hints.height = h;
	if (info->x >= 0) 
		size_hints.x = info->x;
	if (info->y >= 0) 
		size_hints.y = info->y;
	size_hints.flags |= USPosition;

	/* Set properties for window manager (always before mapping) */
	XSetStandardProperties(disp, win, wname, iname, None, 
			       (char **) 0, 0, &size_hints);

	if (info->gc == 0)
		info->gc = XCreateGC(disp, win, (u_long)0, (XGCValues *)0);

	XMapWindow(disp, win);
	/*
	 * override window manager if it doesn't follow hints 
	 * the standard says we're not supposed to do this!
	 */
	XResizeWindow(disp, win, w, h);
	XFlush(disp);
	info->win = win;
	for (;;) {
		XEvent e;
		XNextEvent(disp, &e);
		if (e.type == VisibilityNotify)
			break;
	} 
	return (win);
}

int
have_shmem()
{
	int op, event, err;

	if (XQueryExtension(disp, "MIT-SHM", &op, &event, &err) == 0) {
		fprintf(stderr,
			"nv: (pip grabber) warning: no shared memory\n");
		return (0);
	}
	return (1);
}

XImage *
create_sharedImage()
{
	static XShmSegmentInfo shmi;
	XImage *p;

	if (!have_shmem())
		return (0);

	p = XShmCreateImage(disp, caminfo.visual, 24, ZPixmap, NULL,
			    &shmi, 320, 240);

	shmi.shmid = shmget(IPC_PRIVATE, p->bytes_per_line * p->height,
			    IPC_CREAT | 0777);
	if (shmi.shmid == -1) {
		perror("shmget");
		exit(1);
	}
	shmi.shmaddr = p->data = shmat(shmi.shmid, 0, 0);
	if ((int)p->data == -1) {
		perror("shmat");
		exit(1);
	}
	shmi.readOnly = False;
	if (XShmAttach(disp, &shmi) == 0) {
		perror("XShmAttach");
		exit(1);
	}
	return (p);
}

static XImage *shcamera;

int
catchframe(u_char *yp, signed char *uvp)
{
	u_long *sp;
	u_char *ep;
	int oldmask;
	XImage *camera;
	int sigsetmask();
	/*
	 * The brain dead video hardware requires that the window
	 * be unobscured in order to capture the video.  So we grab
	 * the server and raise the window.  This won't work if
	 * the window isn't mapped.  Also, we block signals while
	 * the server is grabbed.  Otherwise, the process could be
	 * stopped while the display is locked out.
	 */
	oldmask = sigsetmask(~0);
	XGrabServer(disp);
	XRaiseWindow(disp, caminfo.win);
	XSync(disp, 0);
	XvPutStill(disp, grabID, caminfo.win, caminfo.gc,
		   0, 0, 640, 480,
		   0, 0, 320, 240);
	XSync(disp, False);
	XUngrabServer(disp);
	(void)sigsetmask(oldmask);
	camera = shcamera;
	if (camera != 0)
		XShmGetImage(disp, caminfo.win, camera, 0, 0, AllPlanes);
	else
		camera = XGetImage(disp, caminfo.win, 0, 0,
				   320, 240,
				   AllPlanes, ZPixmap);

	/* XXX bold assumptions here */
	sp = (u_long *)camera->data;
	ep = yp + 320*240;
	while (yp < ep) {
		register int y0, y1;
		register int p0 = *sp++;
		register int p1 = *sp++;

		y0 = 5 * (p0 & 0xff);
		y0 += 9 * ((p0 >> 8) & 0xff);
		y0 += 2 * ((p0 >> 16) & 0xff);
		y0 >>= 4;
		*yp++ = y0;
		
		y1 = 5 * (p1 & 0xff);
		y1 += 9 * ((p1 >> 8) & 0xff);
		y1 += 2 * ((p1 >> 16) & 0xff);
		y1 >>= 4;
		*yp++ = y1;

		if (uvp != 0) {
			y0 += y1;
			y0 >>= 1;

			p0 &=~ 0x01010100;
			p1 &=~ 0x01010100;
			p0 = (p0 + p1) >> 1;

			p1 = (126 * (((p0 >> 16) & 0xff) - y0)) / 256;
			if (p1 > 127)
				p1 = 127;
			else if (p1 < -128)
				p1 = -128;
			*uvp++ = p1;
			
			p1 = (160 * ((p0 & 0xff) - y0)) / 256;
			if (p1 > 127)
				p1 = 127;
			else if (p1 < -128)
				p1 = -128;
			*uvp++ = p1;
		}
	}
	if (shcamera == 0)
		XDestroyImage(camera);

	return (1);
}

int
getgrabber()
{
	XvAdaptorInfo *grabbers;
	int ngrabbers, majop, eventbase, errbase;
	Window root = DefaultRootWindow(disp);
  
	if (XQueryExtension(disp, "XVideo", &majop,
			    &eventbase, &errbase) == False)
		return (-1);
	XvQueryAdaptors(disp, root, &ngrabbers, &grabbers);
	if (ngrabbers == 0)
		return (-1);
	if (ngrabbers > 1)
		fprintf(stderr, "warning: more than one frame grabber\n");
		
	grabID = grabbers->base_id;
	return (0);
}

int
GrabImage_Init(int framerate, int *widthp, int *heightp)
{
	extern Tk_Window tkMainWin;
	disp = Tk_Display(tkMainWin);

	if (getgrabber() < 0)
		return (0);

	*widthp = 320;
	*heightp = 240;

	caminfo.vinfo.class = TrueColor;
	caminfo.vinfo.depth = 24;
	caminfo.x = 0;
	caminfo.y = 0;
	caminfo.win = createXwindow(320, 240,
				    "camera", "camera",
				    &caminfo);
	shcamera = create_sharedImage();
	GrabImage = catchframe;

	return (1);
}

void
GrabImage_Cleanup(void)
{
	/*XXX*/
}
#else
int (*GrabImage)(u_char *y_data, signed char *uv_data);

int
GrabImage_Init(int framerate, int *widthp, int *heightp)
{
    return (0);
}

void
GrabImage_Cleanup(void)
{
}
#endif
