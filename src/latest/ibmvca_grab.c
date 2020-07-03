/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>

	IBM VCA frame grab routines

        IBM Support added by Philip Papadopoulos <phil@msr.epm.ornl.gov> and
                             Al Geist <geist@msr.epm.ornl.gov>
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

#ifdef IBMVCA 
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/in.h>
#ifdef AIX
#include <net/nh.h>
#endif
#include <sys/ioctl.h>
#include <sys/vca.h>
#include <X11/Xlib.h>
#include <tcl.h>
#include "sized_types.h"
#include "vid_util.h"
#include "vid_image.h"
#include "vid_code.h"
#include "ibmvca_grab.h"


extern Tcl_Interp *interp;

#define NTSCDEV			"/dev/vca0"
#define PALDEV			"/dev/vcapal0"
#define VCA_XRES		640
#define VCA_YRES		480

static int vcafd = -1;
static int width, height, xmit_color, port;
static unsigned short *image;

/*----------------------------------------------------------------------*/
/* Image data for NV is stored in the YUV format.  Two arrays, rgb2y and
 * rgb2uv, are initialized in the file vid_util.h.  These arrays
 * indexed by an rgb555 (5 bits each for red green and blue) values and
 * and map the rgb index to the corresponding y and uv values.  
 * rgb2y has 8 bit elements while rgb2uv has 16 bit elements.  
 * This gives a strange storage pattern in that
 * one 16-bit uv field corresponds to two 8-but y-fields.  Visually,
 *         ---------  ---------
 *         | Y(p0) |  | Y(p1) |
 *         ---------  ---------
 *         --------------------
 *         |   UV (p0 & p1)   |
 *         -------------------- 
 *
 * The IBM VCA frame grabber returns 16 bits for each pixel. The
 * format is programmable and an rgb565 format was chosen. Notice 
 * that each pass through the inner loop of the grab routines processes
 * two pixels and writes out 2 Y-values and 1 UV-value at each iteration.
 * The grey-scale capture does not write an UV-values.
 */

/* -----   Color Grabber ----- */
static int IBMVCA_GrabColor(uint8 *y_data, uint8 *uv_data)
{
    int x, y;
    uint8 *yp=y_data; 
    uint16 *uvp=(uint16 *)uv_data;
    uint16 *pixel;
    uint16 rgb0, rgb1;
 
    /* now need to convert image into y uv format */
    pixel = image;
    for (y=0; y<height; y++) {
        for (x=0; x<width; x += 2) {
             rgb0 = *pixel++; 
             rgb1 = *pixel++;

             /* byte reverse rgbdata  to get the rgb565  */
             rgb0 = ((rgb0 << 8) & 0xff00) | ((rgb0 >> 8) & 0x00ff);
             rgb1 = ((rgb1 << 8) & 0xff00) | ((rgb1 >> 8) & 0x00ff); 

             /* convert rgb565 to rgb555 */
             rgb0 = ((rgb0 >> 1 ) & 0x7fe0) | (rgb0 & 0x1f); 
             rgb1 = ((rgb1 >> 1 ) & 0x7fe0) | (rgb1 & 0x1f); 

             *yp++ = rgb2y[(int)rgb0]; 
             *yp++ = rgb2y[(int)rgb1];
             *uvp++ = rgb2uv[(int)rgb0]; 
        }
    }

    return 1;
}

/* -----   Grey-Scale Grabber ----- */
static int IBMVCA_GrabGrey(uint8 *y_data)
{
    int x, y;
    uint8 *yp=y_data;
    uint16 *pixel;
    uint16 rgb0, rgb1;

    /* now need to convert image into y uv format */
    pixel = image;
    for (y=0; y<height; y++) {
        for (x=0; x<width; x += 2) {
             rgb0 = *pixel++;
             rgb1 = *pixel++;

             /* byte reverse rgbdata  to get the rgb565  */
             rgb0 = ((rgb0 << 8) & 0xff00) | ((rgb0 >> 8) & 0x00ff);
             rgb1 = ((rgb1 << 8) & 0xff00) | ((rgb1 >> 8) & 0x00ff);

             /* convert rgb565 to rgb555 */
             rgb0 = ((rgb0 >> 1 ) & 0x7fe0) | (rgb0 & 0x1f);
             rgb1 = ((rgb1 >> 1 ) & 0x7fe0) | (rgb1 & 0x1f);

             *yp++ = rgb2y[(int)rgb0];
             *yp++ = rgb2y[(int)rgb1];
        }
    }

    return 1;
}

/* -----  Grabber ----- */
static int IBMVCA_GrabImage(uint8 *y_data, uint8 *uv_data)
{
    /* capture the image */
    ioctl(vcafd, VCA_IOC_CAPTURE, NULL); 
    lseek(vcafd, 0, SEEK_SET);
    read(vcafd, (char *)image, width*height*2);

     if (xmit_color)
         return IBMVCA_GrabColor(y_data, uv_data);
     else
         return IBMVCA_GrabGrey(y_data);
}


/* This routine switches the input port between Composite, RGB and YC */ 
static void IBMVCA_SetPort(int port)
{
    struct vca_controls controls;

    if (vcafd < 0) {
       if ((vcafd = open(NTSCDEV, O_RDWR)) < 0) return;
    }

    controls.format = VCA_IGNORE;
    controls.sync_on_green = VCA_IGNORE;
    controls.display_enable = VCA_IGNORE;
    controls.gen_lock = VCA_IGNORE;
    controls.sync_lock = VCA_IGNORE;
    controls.capture_position = VCA_IGNORE;
    controls.plane = VCA_IGNORE;

    switch (port) {
    case 0: /* COMPOSITE */
	controls.source = VCA_COMPOSITE;
	controls.gen_lock = VCA_ON;
	controls.sync_lock = 12;
	controls.capture_position = VCA_INTERLACED_BOTH;
	break;
    case 1: /* YC (S-VIDEO) */
	controls.source = VCA_YC;
	controls.sync_on_green = VCA_ON_IN_OUT;
	controls.gen_lock = VCA_OFF;
	break;
    case 2: /* RGB */
	controls.source = VCA_RGB;
	controls.sync_on_green = VCA_ON_IN_OUT;
	controls.gen_lock = VCA_OFF;
	break;
    default:  
	fprintf(stderr, "IBMVCA_TracePort: invalid input port.\n");         
    }

    ioctl(vcafd, VCA_IOC_SET_CONTROLS, &controls); 
}

static int IBMVCA_TracePort(ClientData clientData, Tcl_Interp *interp,
			    char *name1, char *name2, int flags)
{
    port = atoi(Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY));
    IBMVCA_SetPort(port);
    return TCL_OK;
}

int IBMVCA_Probe(void)
{
    struct stat vcastat;

    Tcl_TraceVar(interp, "ibmvcaPort", TCL_TRACE_WRITES, IBMVCA_TracePort,
		 NULL);

    if (stat(NTSCDEV, &vcastat) < 0)
	return 0;
    else
	return VID_MEDIUM|VID_GREYSCALE|VID_COLOR;
}

char *IBMVCA_Attach(void)
{
    return ".grabControls.ibmvca";
}

void IBMVCA_Detach(void)
{
}


grabproc_t *IBMVCA_Start(int min_framespacing, int config,
			 reconfigproc_t *reconfig, void *enc_state)
{
    int xoffset, yoffset;
    struct vca_init vinit;
    struct vca_controls controls;
    struct vca_mode mode;
    struct vca_window window;

    xmit_color = (config & VID_COLOR);

    /* Open the video device if not already opened */
    if (vcafd < 0) {
       if ((vcafd = open(NTSCDEV, O_RDWR)) < 0) return NULL;
    }

    width  = VCA_XRES / 2;
    height = VCA_YRES / 2;
    xoffset = (VCA_XRES-width) / 2;
    yoffset = (VCA_YRES-height) / 2;
    if (image != NULL) free(image);
    image = (unsigned short *) malloc(width*height*2);

    /* Set the capture mode for the Video Adapter.  This sets up composite 
       Video with RGB565 as the format.  */
    mode.mode = VCA_MODE_CAPTURE;
    ioctl(vcafd, VCA_IOC_SET_MODE, &mode);

    window.window_enable = VCA_ON; 
    window.window_x = xoffset;
    window.window_y = yoffset;
    window.window_width = width;
    window.window_height = height;
    ioctl(vcafd, VCA_IOC_SET_WINDOW, &window);

    IBMVCA_SetPort(port);

    (*reconfig)(enc_state, xmit_color, width, height);
    return IBMVCA_GrabImage; 
}

void IBMVCA_Stop(void)
{
    free(image);
    close(vcafd);
    image = NULL;
    vcafd = -1;
}
#endif /*IBMVCA*/
