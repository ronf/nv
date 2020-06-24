#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/svideo.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <tk.h>
#include "vidgrab.h"

#define SWITCH_ERROR_HANDLER

#define RED(c)   ((c >> 10) & 0x1f)
#define GREEN(c) ((c >> 5) & 0x1f)
#define BLUE(c)  (c & 0x1f)

enum {false = 0, true};
enum {ALPHA_PLANE = 0x8000};

static unsigned char rgb2y[0x8000];
static Display *dpy = NULL;
static Window window;
static int width = NTSC_WIDTH;
static int height = NTSC_HEIGHT;
static XImage *image = NULL;
static XShmSegmentInfo info;

int (*GrabImage)(unsigned char *);

#ifdef SWITCH_ERROR_HANDLER
static int
error_handler(Display *dpy, XErrorEvent *err)
{
    return (0);
}
#endif

static XImage *
alloc_image_16(void)
{
    register int i;
    XImage *image;

    if ((image = XShmCreateImage(dpy, 0, 16, ZPixmap, 0, &info,
				 width, height)) == NULL)
	return (NULL);
    
    info.shmid = shmget(IPC_PRIVATE,
			image->bytes_per_line * image->height,
			IPC_CREAT|0777);
    
    if (info.shmid < 0){
	XDestroyImage(image);
	return (NULL);
    }
    
    info.shmaddr = (char *)shmat(info.shmid, 0, 0);
    
    if (info.shmaddr == ((char *) -1)) {
	XDestroyImage(image);
	return (NULL);
    }
    
    image->data = info.shmaddr;
    info.readOnly = False;
    
    XShmAttach(dpy, &info);
    
    XSync(dpy, False);
    if (shmctl(info.shmid, IPC_RMID, NULL) < 0)
	perror("shmctl: IPC_RMID");

    for (i = 0; i < sizeof(rgb2y); i++)
        rgb2y[i] = 2.392 * RED(i) + 4.696 * GREEN(i) + 0.912 * BLUE(i);
    return (image);
}

int
GrabImage_vmap(unsigned char *buf)
{
    register int y;
    register unsigned short *idx;
#ifdef SWITCH_ERROR_HANDLER
    XErrorHandler default_handler;
#endif

    if (image == NULL && (image = alloc_image_16()) == NULL)
	return (false);

#ifdef SWITCH_ERROR_HANDLER
    default_handler = XSetErrorHandler(error_handler);
#endif

    XShmGetImage(dpy, window, image, 0, 0, AllPlanes ^ ALPHA_PLANE);

#ifdef SWITCH_ERROR_HANDLER
    XSync(dpy, False);
    XSetErrorHandler(default_handler);
#endif

    idx = (unsigned short *)image->data;
    for (y = 0; y < height; y++) {
	register int x;

	for (x = 0; x < width; x++)
	    *buf++ = rgb2y[*idx++];
    }

    return (true);
}

int
GrabImage_Init(int framerate, int *widthp, int *heightp)
{
    extern Tk_Window tkMainWin;
    XSonyVideoAttr v_attr;
    XSizeHints	sizehints;
    XVisualInfo	vinfo;
    XSetWindowAttributes attr;

    dpy = Tk_Display(tkMainWin);

    if (XSonyQueryVideo(dpy, DefaultScreen(dpy), &v_attr) == NULL)
	return (false);

    if (v_attr.max_video == 0)
	return (false);

    if (!XMatchVisualInfo(dpy, DefaultScreen(dpy), 16, TrueColor, &vinfo)) {
        fprintf (stderr,"Can't find match visual: 16 plane TrueColor\n");
        exit(1);
    }
    attr.colormap = XCreateColormap(dpy, DefaultRootWindow(dpy),
				    vinfo.visual, AllocNone);
    attr.background_pixel = attr.border_pixel = 0;
    window = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, width, height,
			   0, vinfo.depth, InputOutput, vinfo.visual, 
	                   CWColormap | CWBackPixel | CWBorderPixel, &attr);

    XSonyRenderVideo(dpy, window, 0, 0, 0, width, height,
		     0, 0, width, height, 30);

    XStoreName(dpy, window, "video");

    /* Inhibit resizing */
    sizehints.max_width = sizehints.min_width = width;
    sizehints.max_height = sizehints.min_height = height;
    sizehints.flags = (PMaxSize|PMinSize);
    XSetNormalHints(dpy, window, &sizehints);

    XMapWindow(dpy, window);

    *widthp = width;
    *heightp = height;
    GrabImage = GrabImage_vmap;

    return (true);
}

void
GrabImage_Cleanup(void)
{
    if (image != NULL) {
	XShmDetach(dpy, &info);
	shmdt(info.shmaddr);
	XDestroyImage(image);
	image = NULL;
    }

    XUnmapWindow(dpy, window);
    XDestroyWindow(dpy, window);

    return;
}
