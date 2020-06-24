/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	SGI Video Library (VL) frame grab routines for Indy's built-in
	VINO chip (IndyCam) and the optional EV1 (Galileo, IndyVideo and
	Indigo2Video) boards.

	Derived from code written by Andrew Cherenson <arc@sgi.com>.

	To do: support grabber controls for Galileo boards.
*/

#ifdef SGIVL
#include <stdlib.h>
#include <stdio.h>
#include <vl/vl.h>
#include <vl/dev_vino.h>
#include <sys/cachectl.h>
#include <dlfcn.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_code.h"
#include "sgivl_grab.h"
#include "tcl.h"

/* In IRIX 5.2, the returned height is incorrect on EV1 boards. */
#define EV1BUG

static int xmit_size, xmit_color, width, height;
static VLServer vlServer;
static VLPath vlPath;
static VLBuffer transferBuf;
static VLNode src, drn;
static int inputNode;
static int analogType;
static enum { EV1, VINO, OTHER } devType;
static const uint32 mask0=0xff000000, mask1=0xff0000, mask2=0xff,
		    uvflip=0x80808080;

extern Tcl_Interp *interp;

static void *SGIVL_GetVideoData(void)
{
    int wait;
    VLInfoPtr info;

    for (wait=0; wait < 10; wait++) {
	info = vlGetLatestValid(vlServer, transferBuf);
	if (info != NULL) {
	    return vlGetActiveRegion(vlServer, transferBuf, info);
	}
	sginap(1);
    }
    return NULL;
}

static int SGIVL_GrabField(uint8 *y_data, int8 *uv_data)
{
    int w, h;
    uint32 *dataptr, *yptr, *uvptr, data, y, uv;

    if ((dataptr = (uint32 *)SGIVL_GetVideoData()) == NULL)
	return 0;

    yptr = (uint32 *) y_data;
    uvptr = (uint32 *) uv_data;

    if (xmit_color) {
	for (h=0; h < height; h++) {
	    for (w=0; w < width; w += 4) {
		data = dataptr[0];
		y = (data & mask1) << 8;

		data = dataptr[1];
		y += data & mask1;
		uv = (data & mask0) + ((data << 8) & mask1);

		data = dataptr[2];
		y += (data & mask1) >> 8;

		data = dataptr[3];
		*yptr++ = y + ((data & mask1) >> 16);
		*uvptr++ = (uv + (((data & mask0) +
			           ((data << 8) & mask1)) >> 16)) ^ uvflip;

		dataptr += 4;
	    }
	}
    } else {
	for (h=0; h < height; h++) {
	    for (w=0; w < width; w += 4) {
		data = dataptr[0];
		y = (data & mask1) << 8;

		data = dataptr[1];
		y += data & mask1;

		data = dataptr[2];
		y += (data & mask1) >> 8;

		data = dataptr[3];
		*yptr++ = y + ((data & mask1) >> 16);

		dataptr += 4;
	    }
	}
    }

    vlPutFree(vlServer, transferBuf);
    return 1;
}

static int SGIVL_GrabFrame(uint8 *y_data, int8 *uv_data)
{
    int w, h;
    uint32 *dataptr, *yptr, *uvptr, data, y, uv;

    if ((dataptr = (uint32 *)SGIVL_GetVideoData()) == NULL) return 0;

    yptr = (uint32 *) y_data;
    uvptr = (uint32 *) uv_data;

    if (xmit_color) {
	for (h=0; h < height; h++) {
	    for (w=0; w < width; w += 4) {
		data = dataptr[0];
		y = ((data & mask1) << 8) + ((data & mask2) << 16);
		uv = (data & mask0) + ((data << 8) & mask1);

		data = dataptr[1];
		*yptr++ = y + ((data & mask1) >> 8) + (data & mask2);
		*uvptr++ = (uv + ((data & mask0) >> 16) +
			    ((data >> 8) & mask2)) ^ uvflip;

		dataptr += 2;
	    }
	}
    } else {
	for (h=0; h < height; h++) {
	    for (w=0; w < width; w += 4) {
		data = dataptr[0];
		y = ((data & mask1) << 8) + ((data & mask2) << 16);

		data = dataptr[1];
		*yptr++ = y + ((data & mask1) >> 8) + (data & mask2);

		dataptr += 2;
	    }
	}
    }

    vlPutFree(vlServer, transferBuf);
    return 1;
}

static char *TraceVinoInput(ClientData clientData, Tcl_Interp *interp,
			    char *name1, char *name2, int flags)
{
    char *value;

    value = Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY);
    switch (value[0]) {
	case 'c':
	    inputNode = VL_VINO_SRC_AV_IN;
	    analogType = VL_VINO_COMPOSITE;
	    break;
	case 'i':
	    inputNode = VL_VINO_SRC_DV_IN;
	    break;
	case 's':
	    inputNode = VL_VINO_SRC_AV_IN;
	    analogType = VL_VINO_SVIDEO;
	    break;
	default:	/* use current vpanel choice */
	    inputNode = VL_ANY;
	    break;
    }

    return NULL;
}

int SGIVL_Probe(void)
{
    VLDevList devlist;
    int i;

    if ((vlServer = vlOpenVideo("")) == NULL) return 0;

    vlGetDeviceList(vlServer, &devlist);
    for (i=0; i < devlist.numDevices; i++) {
	if (!strcmp(devlist.devices[i].name, "ev1")) {
	    devType = EV1;
	} else if (!strcmp(devlist.devices[i].name, "vino")) {
	    devType = VINO;
	    Tcl_TraceVar(interp, "vinoInput", TCL_TRACE_WRITES,	
		    TraceVinoInput, (ClientData)0);
	    break;
	}
    }

    vlCloseVideo(vlServer);
    vlServer = NULL;
    return VID_SMALL|VID_MEDIUM|VID_LARGE|VID_GREYSCALE|VID_COLOR;
}

char *SGIVL_Attach(void)
{
    return (devType == VINO) ? ".grabControls.sgiIndyVINO" : NULL;
}

void SGIVL_Detach(void)
{
}

grabproc_t *SGIVL_Start(int max_framerate, int config, reconfigproc_t *reconfig)
{
    grabproc_t *grab;
    VLControlValue val;

    if (devType == VINO) {
	Tcl_VarEval(interp, "sgivlDisableControls", NULL);
    }

    xmit_size = (config & VID_SIZEMASK);
    xmit_color = (config & VID_COLOR);

    if (!(vlServer = vlOpenVideo(""))) {
	vlPerror("open video");
	goto failed;
    }

    src = vlGetNode(vlServer, VL_SRC, VL_VIDEO, inputNode);
    drn = vlGetNode(vlServer, VL_DRN, VL_MEM, VL_ANY);

    vlPath = vlCreatePath(vlServer, VL_ANY, src, drn);
    if (vlPath < 0) {
	vlPerror("create path");
	goto failed;
    }

    if (vlSetupPaths(vlServer, (VLPathList)&vlPath, 1, VL_SHARE, VL_SHARE)<0) {
	vlPerror("set up paths");
	goto failed;
    }

    if (devType == VINO && inputNode == VL_VINO_SRC_AV_IN) {
	val.intVal = analogType;
	if (vlSetControl(vlServer, vlPath, src, VL_MUXSWITCH, &val) < 0) {
	    vlPerror("set control analog");
	    goto failed;
	}
    }

    val.intVal = VL_PACKING_YVYU_422_8;
    if (vlSetControl(vlServer, vlPath, drn, VL_PACKING, &val) < 0) {
	vlPerror("set control packing");
	goto failed;
    }

    if (max_framerate > 0) {
	val.fractVal.numerator =  max_framerate;
	val.fractVal.denominator = 1;
	if (vlSetControl(vlServer, vlPath, drn, VL_RATE, &val) < 0) {
	    vlPerror("set control rate");
	    goto failed;
	}
    }

    switch (xmit_size) {
    case VID_SMALL:
    case VID_MEDIUM:
	val.fractVal.numerator =  1;
	val.fractVal.denominator = (xmit_size == VID_SMALL) ? 2 : 1;
	if (vlSetControl(vlServer, vlPath, drn, VL_ZOOM, &val) < 0) {
	    vlPerror("set zoom");
	    goto failed;
	}

	val.intVal = VL_CAPTURE_EVEN_FIELDS;
	if (vlSetControl(vlServer, vlPath, drn, VL_CAP_TYPE, &val) < 0) {
	    vlPerror("set capture type");
	    goto failed;
	}

	vlGetControl(vlServer, vlPath, drn, VL_SIZE, &val);
	width = val.xyVal.x/2;
	height = val.xyVal.y;
#ifdef EV1BUG
	if (devType == EV1) height /= 2;
#endif
	grab = SGIVL_GrabField;
	break;
    case VID_LARGE:
	val.fractVal.numerator =  1;
	val.fractVal.denominator = 1;
	if (vlSetControl(vlServer, vlPath, drn, VL_ZOOM, &val) < 0) {
	    vlPerror("set zoom");
	    goto failed;
	}

	val.intVal = VL_CAPTURE_INTERLEAVED;
	if (vlSetControl(vlServer, vlPath, drn, VL_CAP_TYPE, &val) < 0) {
	    vlPerror("set capture type");
	    goto failed;
	}

	vlGetControl(vlServer, vlPath, drn, VL_SIZE, &val);
	width = val.xyVal.x;
	height = val.xyVal.y;
	grab = SGIVL_GrabFrame;
	break;
    }

    /* Create ring buffers to hold captured data of new size. */
    transferBuf = vlCreateBuffer(vlServer, vlPath, drn, 2);
    if (transferBuf == NULL) {
	vlPerror("create Buffer");
	goto failed;
    }

    /* Tell VL to use newly created ring buffer. */
    if (vlRegisterBuffer(vlServer, vlPath, drn, transferBuf) < 0) {
	vlPerror("Buffer Registration");
	goto failed;
    }

    if (vlBeginTransfer(vlServer, vlPath, 0, NULL) < 0) {
	vlPerror("begin transfer");
	goto failed;
    }

    (*reconfig)(xmit_color, width, height);
    return grab;

failed:
    if (vlServer != NULL) {
	vlCloseVideo(vlServer);
	vlServer = NULL;
    }

    return NULL;
}

void
SGIVL_Stop(void)
{
    if (devType == VINO) {
	Tcl_VarEval(interp, "sgivlEnableControls", NULL);
    }

    vlEndTransfer(vlServer, vlPath);
    vlDeregisterBuffer(vlServer, vlPath, drn, transferBuf);
    vlDestroyPath(vlServer,vlPath);
    vlDestroyBuffer(vlServer, transferBuf);
    vlCloseVideo(vlServer);
    vlServer = NULL;
}
#endif /* SGIVL */
