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
static const uint32 mask0=0xff000000, mask1=0xff0000, mask2=0xff;

extern Tcl_Interp *interp;

static void *SGIVL_GetVideoData(void)
{
    int wait;
    VLInfoPtr info;

    vlPutFree(vlServer, transferBuf);

    for (wait=0; wait < 10; wait++) {
	info = vlGetLatestValid(vlServer, transferBuf);
	if (info != NULL) {
	    return vlGetActiveRegion(vlServer, transferBuf, info);
	}
	sginap(1);
    }
    return NULL;
}

static int SGIVL_GrabFrame(uint8 **datap, int *lenp)
{
    if ((*datap = (uint8 *)SGIVL_GetVideoData()) == NULL)
	return 0;

    *lenp = xmit_color? width*height*2 : width*height;
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

/*ARGSUSED*/
grabproc_t *SGIVL_Start(int grabtype, int min_framespacing, int config,
			reconfigproc_t *reconfig, void *enc_state)
{
    VLControlValue val;

    if (vlServer != NULL) SGIVL_Stop();

    if (devType == VINO) {
	Tcl_VarEval(interp, "sgivlDisableControls", NULL);
    }

    xmit_size = (config & VID_SIZEMASK);

    if ((vlServer = vlOpenVideo("")) == NULL) {
	vlPerror("open video");
	return NULL;
    }

    src = vlGetNode(vlServer, VL_SRC, VL_VIDEO, inputNode);
    drn = vlGetNode(vlServer, VL_DRN, VL_MEM, VL_ANY);

    vlPath = vlCreatePath(vlServer, VL_ANY, src, drn);
    if (vlPath < 0) {
	vlPerror("create path");
	goto failed;
    }

    if (vlSetupPaths(vlServer, (VLPathList)&vlPath, 1, VL_SHARE,
		     VL_SHARE) < 0) {
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

    switch (grabtype) {
    /*case VIDIMAGE_GREY:
	val.intVal = VL_PACKING_Y_8_P;
	if (vlSetControl(vlServer, vlPath, drn, VL_PACKING, &val) < 0) {
	    vlPerror("set control packing");
	    goto failed;
	}
	xmit_color = 0;
	break;*/
    case VIDIMAGE_UYVY:
	val.intVal = VL_PACKING_YVYU_422_8;
	if (vlSetControl(vlServer, vlPath, drn, VL_PACKING, &val) < 0) {
	    vlPerror("set control packing");
	    goto failed;
	}
	xmit_color = 1;
	break;
    default:
	goto failed;
    }

    val.fractVal.numerator =  1;
    switch (xmit_size) {
    case VID_SMALL:
	val.fractVal.denominator = 4;
	break;
    case VID_MEDIUM:
	val.fractVal.denominator = 2;
	break;
    case VID_LARGE:
	val.fractVal.denominator = 1;
	break;
    }

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

done:
    (*reconfig)(enc_state, width, height);
    return SGIVL_GrabFrame;

failed:
    vlCloseVideo(vlServer);
    vlServer = NULL;
    return NULL;
}

void
SGIVL_Stop(void)
{
    if (devType == VINO) {
	Tcl_VarEval(interp, "sgivlEnableControls", NULL);
    }

    vlPutFree(vlServer, transferBuf);
    vlEndTransfer(vlServer, vlPath);
    vlDeregisterBuffer(vlServer, vlPath, drn, transferBuf);
    vlDestroyBuffer(vlServer, transferBuf);
    vlDestroyPath(vlServer,vlPath);
    vlCloseVideo(vlServer);
    vlServer = NULL;
}
#endif /* SGIVL */
