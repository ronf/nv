/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	PARC video frame grab routines
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

#ifdef PARCVID
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <tcl.h>
#include <parcvid_lib.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_code.h"
#include "parcvid_grab.h"

extern Tcl_Interp *interp;

static parcvid_dev_t *pv;
static int xmit_size, xmit_color, interlaced, port=PARCVID_COMP, width, height;
static uint8 *grabdata;
static reconfigproc_t *reconfig;
static void *enc_state;

static void PARCVid_Reconfig(void)
{
    int w, h;

    switch (parcvid_get_format(pv)) {
    default:
    case PARCVID_NOLOCK:
        fprintf(stderr, "Warning: no video signal found!\n");
	/* Fall through */
    case PARCVID_NTSC:
	w = NTSC_WIDTH;
	h = NTSC_HEIGHT;
	break;
    case PARCVID_PAL:
	w = PAL_WIDTH;
	h = PAL_HEIGHT;
	break;
    }

    switch (xmit_size) {
    case VID_SMALL:
	w /= 2;
	h /= 2;
	break;
    case VID_MEDIUM:
	break;
    case VID_LARGE:
	w *= 2;
	h *= 2;
	break;
    }

    if (xmit_size == VID_LARGE) {
	(void) parcvid_set_fields(pv, PARCVID_PAIRS);
	(void) parcvid_set_scaler(pv, -1, -1, -1, -1, w, h/2);
	interlaced = 1;
    } else {
	(void) parcvid_set_fields(pv, PARCVID_EVEN);
	(void) parcvid_set_scaler(pv, -1, -1, -1, -1, w, h);
	interlaced = 0;
    }

    if (grabdata != NULL) free(grabdata);
    grabdata = (uint8 *)malloc(xmit_color? w*h*2 : w*h);

    width = w;
    height = h;
    if (reconfig) (*reconfig)(enc_state, width, height);
}

static int PARCVid_GrabGrey(uint8 **datap, int *lenp)
{
    int field;

    if (interlaced) {
	field = parcvid_readgrey_interlaced(pv, grabdata, width, height);
    } else {
	field = parcvid_readgrey(pv, grabdata, width, height);
    }

    if (field >= 0) {
	*datap = grabdata;
	*lenp = width*height;
	return 1;
    } else {
	return 0;
    }
}

static int PARCVid_GrabYUYV(uint8 **datap, int *lenp)
{
    int field;

    if (interlaced) {
	field = parcvid_readyuyv_interlaced(pv, grabdata, width, height);
    } else {
	field = parcvid_readyuyv(pv, grabdata, width, height);
    }

    if (field >= 0) {
	*datap = grabdata;
	*lenp = width*height*2;
	return 1;
    } else {
	return 0;
    }
}

static int PARCVid_GrabUYVY(uint8 **datap, int *lenp)
{
    int field;

    if (interlaced) {
	field = parcvid_readuyvy_interlaced(pv, grabdata, width, height);
    } else {
	field = parcvid_readuyvy(pv, grabdata, width, height);
    }

    if (field >= 0) {
	*datap = grabdata;
	*lenp = width*height*2;
	return 1;
    } else {
	return 0;
    }
}

/*ARGSUSED*/
static char *PARCVid_TracePort(ClientData clientData, Tcl_Interp *interp,
			       char *name1, char *name2, int flags)
{
    port = atoi(Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY));
    if (pv != NULL) {
	(void) parcvid_set_port(pv, port);
	PARCVid_Reconfig();
    }
 
    return NULL;
}
 
int PARCVid_Probe(void)
{
    struct stat vfcstat;
 
    Tcl_TraceVar(interp, "pvidPort", TCL_TRACE_WRITES, PARCVid_TracePort, NULL);

    return (stat("/dev/parcvid0", &vfcstat) < 0) ?
		0 : VID_GREYSCALE|VID_COLOR|VID_SMALL|VID_MEDIUM|VID_LARGE;
}

char *PARCVid_Attach(void)
{
    return ".grabControls.parcvid";
}

void PARCVid_Detach(void)
{
}

/*ARGSUSED*/
grabproc_t *PARCVid_Start(int grabtype, int min_framespacing, int config,
			  reconfigproc_t *r, void *e)
{
    grabproc_t *grab;

    if (pv == NULL) {
	if ((pv = parcvid_open("/dev/parcvid0")) == NULL) return NULL;
    }

    xmit_size = (config & VID_SIZEMASK);
    reconfig = r;
    enc_state = e;

    (void) parcvid_set_port(pv, port);

    switch (grabtype) {
    case VIDIMAGE_GREY:
	(void) parcvid_set_mode(pv, PARCVID_GREY);
	grab = PARCVid_GrabGrey;
	xmit_color = 0;
	break;
    case VIDIMAGE_YUYV:
	(void) parcvid_set_mode(pv, PARCVID_YUV);
	grab = PARCVid_GrabYUYV;
	xmit_color = 1;
	break;
    case VIDIMAGE_UYVY:
	(void) parcvid_set_mode(pv, PARCVID_YUV);
	grab = PARCVid_GrabUYVY;
	xmit_color = 1;
	break;
    default:
	grab = NULL;
	break;
    }

    if (grab != NULL) PARCVid_Reconfig();
    return grab;
}

void PARCVid_Stop(void)
{
    if (grabdata != NULL) free(grabdata);
    grabdata = NULL;

    parcvid_destroy(pv);
    pv = NULL;

    reconfig = NULL;
    enc_state = NULL;
}
#endif /* PARCVID */
