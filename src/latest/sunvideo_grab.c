/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>

	XIL Frame grab routines for SunVideo (a.k.a., rtvc) Sbus card
	running on Solaris 2.3 or better, hacked by Michael Speer
	(speer@eng.sun.com).

	XIL CellB grab routines by Michael Speer <speer@eng.sun.com>.

	To compile this you'll need the XIL header files (from the SUNWxilh
	package in the developers kit), not just the XIL runtime packages
	that are bundled with Solaris 2.x releases.
*/

/*
 * Copyright (c) Sun Microsystems, Inc.  1992, 1993. All rights reserved.
 *
 * License is granted to copy, to use, and to make and to use derivative
 * works for research and evaluation purposes, provided that Sun Microsystems is
 * acknowledged in all documentation pertaining to any such copy or derivative
 * work. Sun Microsystems grants no other licenses expressed or implied. The
 * Sun Microsystems  trade name should not be used in any advertising without
 * its written permission.
 *
 * SUN MICROSYSTEMS MERCHANTABILITY OF THIS SOFTWARE OR THE SUITABILITY OF
 * THIS SOFTWARE FOR ANY PARTICULAR PURPOSE.  The software is provided "as is"
 * without express or implied warranty of any kind.
 *
 * These notices must be retained in any copies of any part of this software.
 */

#ifdef SUNVIDEO
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <xil/xil.h>
#include <synch.h>
#include <thread.h>
#include <tcl.h>
#include "sized_types.h"
#include "rtp.h"
#include "vid_image.h"
#include "vid_code.h"
#include "sunvideo_grab.h"
#include "cellb.h"

/*
 * Define some local structures.
 */
typedef enum { SV_CellB=1, SV_Raw } sv_encoding_t;

#define SUNVIDEO_CMD_PORT	0x01
#define SUNVIDEO_CMD_SKIP	0x02
#define SUNVIDEO_CMD_RECONFIG	0x04
#define SUNVIDEO_CMD_START	0x08
#define SUNVIDEO_CMD_STOP	0x10
#define SUNVIDEO_CMD_ENCODING	0x20
#define SUNVIDEO_CMD_EXIT	0x40

#define SUNVIDEO_SUCCESS 0x00
#define SUNVIDEO_FAILURE 0x01

typedef struct {
    unsigned		ctype;
    mutex_t		lock;
    cond_t		cv;
    union {
	int		port;
	int		skip;
	unsigned int	encoding;
    } cmd;
    int			work_to_do;
    int			go;
    int			inited;
    int			status;
} sunvideo_cmd_t;

typedef struct {
    XilImage		si;
    XilDataType		dtype;
    unsigned int	width, height, nbands;
    int			scale;
    uint8		*data;
    int			length;
} sv_image_t;

typedef struct {
    mutex_t		lock;
    cond_t		cv;
    unsigned int	encoding;
    XilSystemState	xilstate;
    XilCis		xilcis;
#ifdef XIL_API_MAJOR_VERSION
    XilDevice		device;
#endif
    sv_image_t		small, medium, large;
    sv_image_t		*current;
    int			frame_avail;
    int			xmit_size;
    int			xmit_color;
    XilImage		ri;	
    reconfigproc_t	*reconfig;
    void		*enc_state;
    thread_t		sunvideo_thread_id;
    unsigned int	bandwidth;
} sv_state_t;

/*
 * External Variables.
 */
extern Tcl_Interp *interp;

/*
 * Declare local variables.
 */
static sv_state_t	*svp = NULL; 
static sunvideo_cmd_t	*svcp = NULL;

static char *devs[] = { "/dev/rtvc0", "/dev/rtvc1", "/dev/rtvc2", NULL };
static char *devname = NULL;

/*
 * In order to allow this source to work on machine which do not
 * have the Solaris 2.3 Edition 4 or Solaris 2.4 or later versions
 * of the software, we do not assume UYVY support.  Instead, we test
 * for it and use it if it is there (cis != NULL)
 */
static Xil_boolean SunVideo_UYVY_ErrHandler(XilError error)
{
    /*
     * If UYVY is not installed, we got the following four errors
     */
    if (xil_error_get_primary(error)) {
	if (xil_error_get_category(error) == XIL_ERROR_CONFIGURATION)
	    return TRUE;
    } else {
	char *id = xil_error_get_id(error);

	if (!strcmp(id, "di-158") || !strcmp(id, "di-125") ||
	    !strcmp(id, "di-282")) return TRUE;
    }

    return xil_call_next_error_handler(error);
}

static void SunVideo_UYVY_Init(sv_state_t *svp)
{
    if (!svp->xilstate) return;

    /* In case we already have one, destroy it. */ 
    if (svp->xilcis != NULL) {
	xil_cis_destroy(svp->xilcis);
	svp->xilcis = NULL;
    }

    xil_install_error_handler(svp->xilstate, SunVideo_UYVY_ErrHandler);
    svp->xilcis = xil_cis_create(svp->xilstate, "UYVY");
    xil_remove_error_handler(svp->xilstate, SunVideo_UYVY_ErrHandler);

    if (svp->xilcis != NULL) {
	xil_cis_set_max_frames(svp->xilcis, 1);
	xil_cis_set_keep_frames(svp->xilcis, 1);
    }
}

static void SunVideo_Reconfig(sv_state_t *svp)
{
    switch (svp->xmit_size) {
    case VID_SMALL:
	svp->current = &svp->small;
	break;
    case VID_MEDIUM:
	svp->current = &svp->medium;
	break;
    case VID_LARGE:
	svp->current = &svp->large;
	break;
    }

    if (svp->reconfig) {
	(*svp->reconfig)(svp->enc_state, svp->current->width,
			 svp->current->height);
    }
}

static void SunVideo_Image_Create(sv_state_t *svp, int imagesize) 
{
    unsigned int scale, width, height, nbands;
    XilDataType datatype;
    sv_image_t *svip;

    if (!svp->ri) {
	fprintf(stderr, "nv: Sunvideo Raw Image non-existent\n");
	exit(1);
    }

    xil_get_info(svp->ri, &width, &height, &nbands, &datatype);

    switch (imagesize) {
    case VID_SMALL:
	scale = 4;
	svip = &svp->small;
	break;
    case VID_MEDIUM:
	scale = 2;
	svip = &svp->medium;
	break;
    case VID_LARGE:
	scale = 1;
	svip = &svp->large;
	break;
    }

    svip->width = width / scale;
    svip->height = height / scale;
    svip->nbands = nbands;
    svip->dtype = datatype;
    svip->scale = scale;

    if ((svip->si = xil_create(svp->xilstate, svip->width,
			       svip->height, nbands, datatype)) == NULL) {
	fprintf(stderr, "nv: SunVideo Scaled Image create failed\n");
	exit(1);
    }

    svip->data = NULL;
    svip->length = 0;
}

static void SunVideo_GrabYUV(sv_state_t *svp)
{
#if 0
    int x, y;
    XilDataType datatype;
    XilMemoryStorage layout;
    unsigned long scanline_stride;
    unsigned int pixel_stride;
    Xil_unsigned8 *xil_data;
    uint32 *yp, *uvp, *w;
    sv_image_t *svip = svp->current;

    xil_scale(svp->ri, svip->si, "nearest", 1.0/(float)svip->scale,
	      1.0/(float)svip->scale);
    xil_export(svip->si);
    xil_get_memory_storage(svip->si, &layout);

    xil_data = layout.byte.data;
    scanline_stride = layout.byte.scanline_stride;
    pixel_stride = layout.byte.pixel_stride;

    mutex_lock(&svp->lock);

    if (svp->xmit_color && (pixel_stride == 3) &&
	(((uint32) xil_data) % sizeof(uint32) == 0) &&
	(scanline_stride % sizeof(uint32) == 0)) {

	yp = (uint32 *) y_data;
	uvp = (uint32 *) uv_data;

	/*
	 * Grab four pixels at a time, produce four bytes of Y, Y, Y, Y
	 *   and four bytes of U, V, U, V.
	 * Note: the framegrabber gives us UV values for every Y value,
	 *   whereas nv (sensibly) only wants UV values per two Y
	 *   values.  For better or worse, we simply ignore one set of
	 *   UV values rather than, say, averaging them.  We'd have to
	 *   shift the UV values in pixels 0 and 2, whereas the UV
	 *   values in pixels 1 and 3 are already aligned the way we
	 *   want them, so we use the latter.
	 */
	for (y=0; y<svip->height; y++, xil_data += scanline_stride) {
	    for (x=0, w=(uint32 *)xil_data; x<svip->width; x += 4) {
		uint32 w0, w1, w2;

		w0 = w[0];	/* y0 u0 v0 y1 */
		w1 = w[1];	/* u1 v1 y2 u2 */
		w2 = w[2];	/* v2 y3 u3 v3 */
		w += 3;

		/* ====> Big-endian assumptions here... */
		*yp++ = (w0 & 0xff000000) | ((w0 << 16) & 0xff0000) |
			(w1 & 0xff00) | ((w2 >> 16) & 0xff);

		*uvp++ = ((w1 & 0xffff0000) | (w2 & 0xffff)) ^ 0x80808080;
	    }
	}
    } else {
	/*
	 * Do everything a byte at a time instead of a word at a time;
	 * more general but painfully slow.  If anyone cares about
	 * greyscale performance, we should tune this.
	 */
	for (y=0; y<svip->height; y++, xil_data += scanline_stride) {
	    Xil_unsigned8 *p;

	    for (x=0, p=xil_data; x<svip->width; x += 2) {
		/*
		 * Note: the framegrabber gives us UV values for
		 * every Y value, whereas nv (sensibly) only wants
		 * UV values per two Y values.  For better or worse,
		 * we simply ignore one set of UV values rather
		 * than, say, averaging them.
		 */
		*y_data++ = p[0];

		if (svp->xmit_color) {
		    *uv_data++ = p[1] - 0x80;
		    *uv_data++ = p[2] - 0x80;
		}

		p += pixel_stride;

		*y_data++ = p[0];
		p += pixel_stride;
	    }
	}
    }

    if (!svp->frame_avail) {
	svp->frame_avail = 1;
	cond_broadcast(&svp->cv);
    }

    mutex_unlock(&svp->lock);

    xil_import(svip->si, FALSE);
    xil_toss(svip->si);
#endif
}

static int SunVideo_Create_Cis(sv_state_t *svp, unsigned int encoding)
{
    svp->encoding = encoding;

    switch (encoding) {
    case SV_CellB:
	svp->xilcis = xil_cis_create(svp->xilstate, "CellB");
	if (svp->xilcis != NULL) {
	    xil_cis_set_max_frames(svp->xilcis, 1);
	    xil_cis_set_keep_frames(svp->xilcis, 1);
	} else {
	    return 1;
	}
	break;
    case SV_Raw:
	SunVideo_UYVY_Init(svp);
	break;
    default:
	return 1;
	break;
    }

    return 0;
}

static void *SunVideo_GrabThread(void *arg)
{
    int port, nframes, max_buffers=1;
    char *devtype="SUNWrtvc";
    sv_image_t *svip;

    mutex_lock(&svcp->lock);
    mutex_lock(&svp->lock);

    /* Open the xil library. */
    if ((svp->xilstate = xil_open()) == NULL) {
	fprintf(stderr, "nv: Unable to initialize XIL\n");
	exit(1);
    }

    /*
     * Should install an error handler to reformat the XIL error messages
     * for consumption by humans, but for now...
     */
#ifdef XIL_API_MAJOR_VERSION
    if ((svp->device = xil_device_create(svp->xilstate, "SUNWrtvc")) == NULL) {
	fprintf(stderr, "nv: Unable to initialize XIL\n");
	exit(1);
    }

    xil_device_set_value(svp->device, "DEVICE_NAME", (void *)NULL);

    if ((svp->ri = xil_create_from_device(svp->xilstate, "SUNWrtvc",
					  svp->device)) == NULL) {
	fprintf(stderr, "nv: Unable to initialize XIL\n");
	exit(1);
    }

    xil_device_destroy(svp->device);
#else
    svp->ri = xil_create_from_device(svp->xilstate, devtype, devname);
    if (svp->ri == NULL) {
	fprintf(stderr, "nv: Unable to initialize XIL\n");
	exit(1);
    }
#endif

    xil_set_device_attribute(svp->ri, "SYNC", (void *)0);
    xil_set_device_attribute(svp->ri, "IMAGE_SKIP", (void *)0);
    xil_set_device_attribute(svp->ri, "MAX_BUFFERS", (void *)max_buffers);
    xil_set_device_attribute(svp->ri, "PORT_V", (void *) 1);

    SunVideo_Image_Create(svp, VID_SMALL);
    SunVideo_Image_Create(svp, VID_MEDIUM);
    SunVideo_Image_Create(svp, VID_LARGE);

    mutex_unlock(&svp->lock);

    /*
     * We are ready to go now.
     */
    svcp->inited++;
    cond_broadcast(&svcp->cv);
    mutex_unlock(&svcp->lock);

    for (;;) {
	mutex_lock(&svcp->lock);

	if (!svcp->go || svcp->work_to_do) {
	    while (!svcp->go && !svcp->work_to_do)
		cond_wait(&svcp->cv, &svcp->lock);
	    
	    switch (svcp->ctype) {
	    case SUNVIDEO_CMD_EXIT:
		break;
	    case SUNVIDEO_CMD_START:
		svcp->go++;
		svcp->status = SUNVIDEO_SUCCESS;
		break;
	    case SUNVIDEO_CMD_STOP:
		svcp->go = 0;
		svcp->status = SUNVIDEO_SUCCESS;
		break;
	    case SUNVIDEO_CMD_RECONFIG:
		if (svcp->go) {
		    svcp->status = SUNVIDEO_FAILURE;
		} else {
		    SunVideo_Reconfig(svp);
		    svcp->status = SUNVIDEO_SUCCESS;
		}
		break;
	    case SUNVIDEO_CMD_ENCODING:
		svcp->status = SUNVIDEO_SUCCESS;
		if ((svp->encoding == svcp->cmd.encoding) &&
		    (svp->xilcis != NULL)) {
		    xil_cis_reset(svp->xilcis);
		    xil_cis_set_max_frames(svp->xilcis, 1);
		    xil_cis_set_keep_frames(svp->xilcis, 1);
		} else {
		    if (svp->xilcis != NULL) {
			xil_cis_destroy(svp->xilcis);
			svp->xilcis = NULL;
		    }
		    
		    if (SunVideo_Create_Cis(svp, svcp->cmd.encoding))
			svcp->status = SUNVIDEO_FAILURE;
		}

		if (svp->frame_avail) {
		    mutex_lock(&svp->lock);
		    svp->frame_avail = 0;
		    mutex_unlock(&svp->lock);
		}
		break;
	    case SUNVIDEO_CMD_PORT:
		svcp->status = SUNVIDEO_SUCCESS;
		xil_set_device_attribute(svp->ri, "PORT_V",
					 (void *)svcp->cmd.port);
		break;
	    case SUNVIDEO_CMD_SKIP:
		svcp->status = SUNVIDEO_SUCCESS;
		xil_set_device_attribute(svp->ri, "IMAGE_SKIP",
					 (void *)svcp->cmd.skip);
		break;
	    default:
		fprintf(stderr, "nv: invalid sunvideo cmd\n");
	    }

	    svcp->work_to_do = 0;
	    cond_broadcast(&svcp->cv);
	} 

	mutex_unlock(&svcp->lock);

	if (svcp->go) {
	    svip = svp->current;
	    if (svp->xilcis != NULL) {
		mutex_lock(&svp->lock);
		while (svp->frame_avail && !svcp->work_to_do)
		    cond_wait(&svp->cv, &svp->lock);
		mutex_unlock(&svp->lock);
		if (svp->frame_avail) break;

		xil_scale(svp->ri, svip->si, "nearest",
			  1.0/(float)svip->scale, 1.0/(float)svip->scale);
		xil_compress(svip->si, svp->xilcis);
		xil_cis_sync(svp->xilcis);
		xil_toss(svip->si);
		
		if (xil_cis_has_frame(svp->xilcis)) {
		    mutex_lock(&svp->lock);

		    svip->data = xil_cis_get_bits_ptr(svp->xilcis,
						      &svip->length,
						      &nframes);
		    svp->frame_avail = 1;
		    cond_broadcast(&svp->cv);

		    mutex_unlock(&svp->lock);
		}
	    } else {
		SunVideo_GrabYUV(svp);
	    }
	}
    }
}

static int do_sunvideo_cmd(sunvideo_cmd_t *svcp, char *cmdstring, void *arg)
{
    int result;

    mutex_lock(&svcp->lock);

    if (!strcmp("PORT_V", cmdstring)) {
	svcp->ctype = SUNVIDEO_CMD_PORT;
	svcp->cmd.port = (int)arg;
    } else if (!strcmp("IMAGE_SKIP", cmdstring)) {
	svcp->ctype = SUNVIDEO_CMD_SKIP;
	svcp->cmd.skip = (int)arg;
    } else if (!strcmp("RECONFIG", cmdstring)) {
	svcp->ctype = SUNVIDEO_CMD_RECONFIG;
    } else if (!strcmp("START", cmdstring)) {
	svcp->ctype = SUNVIDEO_CMD_START;
    } else if (!strcmp("STOP", cmdstring)) {
	svcp->ctype = SUNVIDEO_CMD_STOP;
    } else if (!strcmp("EXIT", cmdstring)) {
	svcp->ctype = SUNVIDEO_CMD_EXIT;
	svcp->work_to_do++;
    } else if (!strcmp("ENCODING", cmdstring)) {
	svcp->ctype = SUNVIDEO_CMD_ENCODING;
	svcp->cmd.skip = (unsigned)arg;
    }

    svcp->work_to_do++;
    if (svcp->go)
	cond_broadcast(&svp->cv);
    else
	cond_broadcast(&svcp->cv);
    cond_wait(&svcp->cv, &svcp->lock);
    result = (svcp->status == SUNVIDEO_FAILURE) ? 1 : 0;

    mutex_unlock(&svcp->lock);

    return result;
}

static int XilSetDeviceAttribute(ClientData clientData, Tcl_Interp *interp,
				 int argc, char *argv[], void *third_arg)
{
    XilImage image;
    int status;

    if (argc != 4) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			 "image attribute value", NULL);
	return TCL_ERROR;
    }

    if (strcmp(argv[1], "rtvc0") != 0) {
	Tcl_AppendResult(interp, "first argument to ", argv[0],
			 "must be \"rtvc0\" at present", NULL);
	return TCL_ERROR;
    }

    if ((image = svp->ri) == NULL) {
	/*
	 * Silently ignore the command;  this is what the code in
	 * videopix_grab.c does, but is it right?
	 */
	return TCL_OK;
    }

    if (do_sunvideo_cmd(svcp, argv[2], third_arg) != 0) {
	Tcl_AppendResult(interp, "XIL attempt to set ", argv[2],
			 " attribute on ", argv[1], " failed", NULL);
	return TCL_ERROR;
    }

    return TCL_OK;
}

static int
XilSetDeviceAttributeInt(ClientData clientData, Tcl_Interp *interp,
	int argc, char *argv[])
{
    int val;
    char *endp;

    if (argc >= 4) {
	val = strtol(argv[3], &endp, 10);
	if (*endp != '\0') {
	    Tcl_AppendResult(interp, "third (last) argument to ",
			     argv[0], " should be an integer", NULL);
	    return TCL_ERROR;
	}
    }

    /*
     * Strange but true:  xil_set_device_attribute() really does pass
     * integers in the (void *) pointer rather than in the memory pointed
     * to by the (void *) pointer.
     */
    return XilSetDeviceAttribute(clientData, interp, argc, argv, (void *)val);
}

/* Unused at present */
static int XilSetDeviceAttributeString(ClientData clientData,
				       Tcl_Interp *interp, int argc,
				       char *argv[])
{
    return XilSetDeviceAttribute(clientData, interp, argc, argv,
				 (argc >= 4) ? argv[3] : 0);
}

static int SunVideo_GrabFrame(uint8 **datap, int *lenp)
{
    int nframes, newskip;
    static int skip=0, avgpos=0, avglen[8], totlen=0;

    mutex_lock(&svp->lock);

    while (!svp->frame_avail)
	cond_wait(&svp->cv, &svp->lock);

    *datap = svp->current->data;
    *lenp = svp->current->length;

    svp->frame_avail = 0;
    cond_broadcast(&svp->cv);

    mutex_unlock(&svp->lock);

    totlen += *lenp-avglen[avgpos];
    avglen[avgpos] = *lenp;
    avgpos = (avgpos+1) % 8;

    if (avgpos == 0) {
	newskip = 30*totlen/8/(125*svp->bandwidth);
	if (newskip < 0) newskip = 0;

	if (skip != newskip) {
	    skip = newskip;
	    (void) do_sunvideo_cmd(svcp, "IMAGE_SKIP", (void *)skip);
	}
    }

    return 1;
}

int SunVideo_Probe(void)
{
    char **dp;
    int result;
    int thrpri;

    for (dp=devs; *dp != NULL; dp++) {
	if (access(*dp, R_OK) == 0) {
	    devname = *dp;
	    break;
	}
    }

    if (devname == NULL) return 0;

    if ((svp = (sv_state_t *)malloc(sizeof(sv_state_t))) == NULL) return 0;

    if ((svcp = (sunvideo_cmd_t *)malloc(sizeof(sunvideo_cmd_t))) == NULL)
	return 0;
    
    memset((void *)svp, 0, sizeof(sv_state_t));
    memset((void *)svcp, 0, sizeof(sunvideo_cmd_t));

    mutex_init(&svcp->lock, USYNC_PROCESS, (void *)NULL);
    cond_init(&svcp->cv, USYNC_PROCESS, (void *)NULL);
    mutex_init(&svp->lock, USYNC_PROCESS, (void *)NULL);
    cond_init(&svp->cv, USYNC_PROCESS, (void *)NULL);

    thr_getprio(thr_self(), &thrpri);
    thrpri += 1;
    thr_setprio(thr_self(), thrpri);

    if (thr_create(NULL, 0, SunVideo_GrabThread, NULL, THR_NEW_LWP|THR_BOUND,
		   &svp->sunvideo_thread_id)) return 0;

    mutex_lock(&svcp->lock);
    while (!svcp->inited && (svcp->status != SUNVIDEO_FAILURE))
	cond_wait(&svcp->cv, &svcp->lock);
    mutex_unlock(&svcp->lock);

    if (svcp->status == SUNVIDEO_FAILURE) return 0;

    return VID_SMALL|VID_MEDIUM|VID_LARGE|VID_GREYSCALE|VID_COLOR;
}

char *SunVideo_Attach(void)
{
    Tcl_CreateCommand(interp, "xilSetDevAttrInt",
		      XilSetDeviceAttributeInt, 0, NULL);
    Tcl_CreateCommand(interp, "xilSetDevAttrString",
		      XilSetDeviceAttributeString, 0, NULL);

    return ".grabControls.sunvideo";
}

void SunVideo_Detach(void)
{
    Tcl_DeleteCommand(interp, "xilSetDevAttrInt");
    Tcl_DeleteCommand(interp, "xilSetDevAttrString");
}


/*ARGSUSED*/
grabproc_t *SunVideo_Start(int grabtype, int min_framespacing, int config,
			   reconfigproc_t *r, void *e)
{
    int xmit_color;
    sv_encoding_t encoding;

    switch (grabtype) {
    case VIDIMAGE_GREY:
	xmit_color = 0;
	encoding = SV_Raw;
	break;
    case VIDIMAGE_UYVY:
	xmit_color = 1;
	encoding = SV_Raw;
	break;
    case VIDIMAGE_CELLB:
	xmit_color = 1;
	encoding = SV_CellB;
	break;
    default:
	return NULL;
    }

    mutex_lock(&svp->lock);
    svp->xmit_size = (config & VID_SIZEMASK);
    svp->xmit_color = xmit_color;
    svp->reconfig = r;
    svp->enc_state = e;
    mutex_unlock(&svp->lock);

    if (do_sunvideo_cmd(svcp, "ENCODING", (void *)encoding) != 0) return NULL;
    if (do_sunvideo_cmd(svcp, "IMAGE_SKIP", (void *)0) != 0) return NULL;
    if (do_sunvideo_cmd(svcp, "RECONFIG", (void *)0) != 0) return NULL;
    if (do_sunvideo_cmd(svcp, "START", (void *)0) != 0) return NULL;

    return SunVideo_GrabFrame;
}

void
SunVideo_Stop(void)
{
    (void) do_sunvideo_cmd(svcp, "STOP", (void *)0);

    mutex_lock(&svp->lock);
    svp->reconfig = NULL;
    svp->enc_state = NULL;
    mutex_unlock(&svp->lock);
}
#endif /* SUNVIDEO */
