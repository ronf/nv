/*
 *	Netvideo version 3.3
 *	Written by Ron Frederick <frederick@parc.xerox.com>
 *
 *	Frame grabber for DEC Sound and Motion J300 TurboChannel card
 *	using either Multimedia Services for DEC OSF/1 AXP or 
 *	DEC Systems Research Center's jv2driver.
 *
 *	Written by Mark Prior		<mrp@itd.adelaide.edu.au>,
 *		   Lance Berc		<berc@src.dec.com>,
 *	       and Steve McCanne	<mccanne@ee.lbl.gov>
 */

#if defined(J300)

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef J300_MME
#include <mme/mme_api.h>
#endif
#include <tcl.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_code.h"
#include "j300_grab.h"
#include "jvs.h"

#ifdef MME_FRAME
#define	MME_BUFFERS	1
#else
#define	MME_BUFFERS	3
#endif

/* If the frame is more than 1/2 sec old, get an new one */
#define	OLDFRAME	500000

#define	FULL		1
#define	HALF		2
#define	QUARTER		4

extern Tcl_Interp *interp;
extern char *getenv();

typedef struct {
    int shmid;
    char *addr;
} jv2buf_t;

static int jv2Grabber=0, jvfd = -1, current = 0;
static int devices=0, xmit_size, width, height;
static struct timeval lastgrab;
static jv2buf_t grabbuf[2];

#ifndef J300_MME
static int videoPort=1, videoStandard=1;
#else
static DWORD videoPort = 1, videoStandard = VIDEO_STANDARD_NTSC;
static LPHVIDEO lphvideo = NULL;
static LPBITMAPINFOHEADER bmh = NULL;
static LPVIDEOHDR videoHdr = NULL;

static struct structHdrs {
    VIDEOHDR		videoHdr[MME_BUFFERS];
    HVIDEO		hvideo;
    BITMAPINFOHEADER	bmh;
} *structHdrsPtr = NULL;
#endif

/*ARGSUSED*/
static char *J300_TracePort(ClientData clientData, Tcl_Interp *interp,
			    char *name1, char *name2, int flags)
{
    int port=atoi(Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY));

#ifdef J300_MME
    if (port == 0 && videoPort != 0) { /* changing to S-Video */
	switch (videoStandard) {
	case VIDEO_STANDARD_NTSC:
	    videoStandard = VIDEO_STANDARD_SVIDEO525;
	    break;
	case VIDEO_STANDARD_SECAM:
	    Tcl_SetVar(interp, "j300Format", "2", TCL_GLOBAL_ONLY);
	    /*FALLTHROUGH*/
	case VIDEO_STANDARD_PAL:
	    videoStandard = VIDEO_STANDARD_SVIDEO625;
	    break;
	}

	Tcl_VarEval(interp, "j300FormatSVideo", NULL);
    } else if (port != 0 && videoPort == 0) { /* changing from S-Video */
	switch (videoStandard) {
	case VIDEO_STANDARD_SVIDEO525:
	    videoStandard = VIDEO_STANDARD_NTSC;
	    break;
	case VIDEO_STANDARD_SVIDEO625:
	    videoStandard = VIDEO_STANDARD_PAL;
	    break;
	}

	Tcl_VarEval(interp, "j300FormatComposite", NULL);
    }
#endif

    videoPort = port;
    return NULL;
}

/*ARGSUSED*/
static char *J300_TraceFormat(ClientData clientData, Tcl_Interp *interp,
			      char *name1, char *name2, int flags)
{
    int format=atoi(Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY));

#ifdef J300_MME
    switch (format) {
    case 0:
	videoStandard = 0;
	break;
    case 1:
	if (videoPort == 0)
	    videoStandard = VIDEO_STANDARD_SVIDEO525;
	else
	    videoStandard = VIDEO_STANDARD_NTSC;
	break;
    case 2:
	if (videoPort == 0)
	    videoStandard = VIDEO_STANDARD_SVIDEO625;
	else
	    videoStandard = VIDEO_STANDARD_PAL;
	break;
    case 3:
	videoStandard = VIDEO_STANDARD_SECAM;
	break;
    }
#endif

    return NULL;
}

#ifdef J300_MME
#ifndef MME_FRAME
static void J300_ReuseBuffer(HVIDEO hVideo, DWORD wMsg, DWORD dwInstance,
			     LPVIDEOHDR srvvideohdr, DWORD param2)
{
    switch (wMsg) {
    case MM_DRVM_OPEN:
	    break;
    case MM_DRVM_DATA:
	    break;
    case MM_DRVM_CLOSE:
	    break;
    }
}
#endif /*!MME_FRAME*/

static int MME_Grab(uint8 **datap, int *lenp)
{
    DWORD status;

    status = videoFrame(*lphvideo, videoHdr);
    if (videoHdr->dwBytesUsed == 0) return 0;

    *datap = (uint8 *)videoHdr->lpData;
    *lenp = width*height*2;
    return 1;
}
#endif /*J300_MME*/

static int JV2_AllocBuf(jv2buf_t *jv2buf)
{
    if (JvsAllocateBuf(jvfd, JVS_INPUT, JVS_YUV, width, height,
		       &jv2buf->shmid) < 0) return 0;

    if ((int)(jv2buf->addr = (char *)shmat(jv2buf->shmid, 0, 0)) < 0) {
	JvsDeallocBuf(jvfd, jv2buf->shmid);
	return 0;
    }

    return 1;
}

static void JV2_FreeBuf(jv2buf_t *jv2buf)
{
    shmdt(jv2buf->addr);
    JvsDeallocBuf(jvfd, jv2buf->shmid);
}

static int JV2_Grab(uint8 **datap, int *lenp)
{
    int returnShmid, returnLength, status;
    struct timeval now;
    uint64 lt, nt;

    status = JvsWaitComp(jvfd, &returnShmid, &returnLength);
    gettimeofday(&now, NULL);
	
    /* It's nice to have a 64bit machine */
    lt = (lastgrab.tv_sec * 1000000) + lastgrab.tv_usec;
    nt = (now.tv_sec * 1000000) + now.tv_usec;

    /* do we have a frame waiting and is it "new" enough? */
    if ((status < 0) ||  (nt - lt > OLDFRAME)) {
	do {
	    status = JvsStartComp(jvfd, grabbuf[current].shmid);
	    gettimeofday(&now, NULL);
	    if (status < 0)
		fprintf(stderr, "StartComp failure\n");
	    else {
		status = JvsWaitComp(jvfd, &returnShmid, &returnLength);
		if (status < 0) fprintf(stderr, "WaitComp failure\n");
	    }
	} while (status < 0);
    }

    lastgrab = now;

    *datap = (uint8 *)grabbuf[current].addr;
    *lenp = width*height*2;

    current = 1-current;
    if (JvsStartComp(jvfd, grabbuf[current].shmid) < 0)
	fprintf(stderr, "StartComp failure\n");

    return 1;
}

int J300_Probe(void)
{
    int port, config=0;
    char *ports;
    char buf[64];
    static int trace=0;
    JvsSetCompressReq p1;
    JvsSetCompressRep p2;

    if (!trace) {
	Tcl_TraceVar(interp, "j300Port", TCL_TRACE_WRITES, J300_TracePort,
		     NULL);
	Tcl_TraceVar(interp, "j300Format", TCL_TRACE_WRITES, J300_TraceFormat,
		     NULL);
	trace = 1;
    }

#ifdef J300_MME
    devices = videoGetNumDevs();
    if (devices != 0) {
	jv2Grabber = 0;
	config = VID_GREYSCALE|VID_COLOR|VID_SMALL|VID_MEDIUM|VID_LARGE;
    } else if (mmeServerFileDescriptor() != -1) {
	/* If MME is alive there is no need to check for JV2 */
	return 0;
    } else
#endif
    {
	/*
	 * There is no MME server running so maybe this
	 * machine has DEC SRC's JV2 driver instead
	 */
	port = JVS_SOCKET;
	if (ports = (char *) getenv("JVDRIVER_PORT")) {
	    if (sscanf(ports, "%d", &port) <= 0) port = JVS_SOCKET;
	}

	if ((jvfd = JvsOpen("", port)) < 0) /* nice try but ... */
	    return 0;

	p1.qfactor = 0;
	p1.frameskip = 0;
	p1.type = JVS_YUV;

	p1.xdec = 1;
	p1.ydec = 1;
	JvsSetCompRaw(jvfd, &p1, &p2);

	config = VID_GREYSCALE|VID_COLOR|VID_SMALL|VID_MEDIUM;

	/* The J300 doesn't support full-frame PAL input */
	if (p2.width*3 == p2.height*4) config |= VID_LARGE;

	close(jvfd);
	jv2Grabber = 1;
    }

    sprintf(buf, "%d", videoStandard);
    Tcl_SetVar(interp, "j300Format", buf, TCL_GLOBAL_ONLY);
    sprintf(buf, "%d", videoPort);
    Tcl_SetVar(interp, "j300Port", buf, TCL_GLOBAL_ONLY);
    return config;
}

char *J300_Attach(void)
{
    return ".grabControls.j300";
}

void J300_Detach(void)

{
}

#ifdef J300_MME
/*ARGSUSED*/
static grabproc_t *MME_Start(int min_framespacing, int config,
			     reconfigproc_t *reconfig, void *enc_state)
{
    int i, devices;
    grabproc_t *grab;
    MMRESULT status;

    if (structHdrsPtr != NULL) J300_Stop();

    xmit_size = (config & VID_SIZEMASK);

    if (grabtype != VIDIMAGE_YUYV) return NULL;

    if ((structHdrsPtr = (struct structHdrs *)
	    mmeAllocMem(sizeof(struct structHdrs))) == NULL) return NULL;

    lphvideo = &(structHdrsPtr->hvideo);
    devices = videoGetNumDevs();
    for (i=0; i<devices; i++)
	if (videoOpen(lphvideo, i, VIDEO_IN) == DV_ERR_OK) break;

    if (i == devices) goto failed;

    if (videoStandard == VIDEO_STANDARD_NTSC ||
	videoStandard == VIDEO_STANDARD_SVIDEO525) {
	width = NTSC_WIDTH;
	height = NTSC_HEIGHT;
    } else {
	width = PAL_WIDTH;
	height = PAL_HEIGHT;
    }

    switch (xmit_size) {
    case VID_SMALL:
	width /= 2;
	height /= 2;
	break;
    case VID_MEDIUM:
	break;
    case VID_LARGE:
	width *= 2;
	height *= 2;
	break;
    }

    videoSetPortNum(*lphvideo, videoPort);
    videoSetStandard(*lphvideo, videoStandard);
    bmh = &(structHdrsPtr->bmh);
    bzero(bmh, sizeof(BITMAPINFOHEADER));
    bmh->biSize = sizeof(BITMAPINFOHEADER);
    bmh->biWidth = width;
    bmh->biHeight = height;
    bmh->biPlanes = 1;
    bmh->biBitCount = 16;
    bmh->biCompression = BICOMP_DECYUVDIB;
    if (videoConfigure((HVIDEO)*lphvideo, DVM_FORMAT,
		       VIDEO_CONFIGURE_GET|VIDEO_CONFIGURE_MIN, 0, bmh,
		       bmh->biSize, 0, 0) != DV_ERR_OK) goto failed;

    if (videoConfigure((HVIDEO)*lphvideo, DVM_FORMAT, VIDEO_CONFIGURE_SET,
		       0, bmh, bmh->biSize, 0, 0) != DV_ERR_OK)  goto failed;

#ifndef MME_FRAME
    if ((status = videoStreamInit((HVIDEO)*lphvideo, 0, J300_ReuseBuffer, NULL,
				  CALLBACK_FUNCTION)) != DV_ERR_OK) goto failed;
#endif

    for (i=0; i<MME_BUFFERS; i++) {
	videoHdr = &structHdrsPtr->videoHdr[i];
	videoHdr->lpData = mmeAllocBuffer(bmh->biSizeImage);
	videoHdr->dwBufferLength = bmh->biSizeImage;
	videoHdr->dwBytesUsed = 0;
	videoHdr->dwTimeCaptured = 0;
	videoHdr->dwUser = 0;
	videoHdr->dwFlags = 0;
	videoHdr->dwReserved[0] = (DWORD)NULL;

#ifndef MME_FRAME
	if (videoStreamPrepareHeader((HVIDEO)*lphvideo, videoHdr,
				     sizeof(videoHdr)) != DV_ERR_OK)
	    goto failed;

	if ( videoStreamAddBuffer((HVIDEO)*lphvideo, videoHdr,
				  sizeof(videoHdr)) != DV_ERR_OK) goto failed;
#endif
    }

#ifndef MME_FRAME
    if ((status = videoStreamStart((HVIDEO)*lphvideo)) != DV_ERR_OK)
	goto failed;
#endif

    (*reconfig)(enc_state, width, height);
    return MME_Grab;

failed:
    mmeFreeMem(structHdrsPtr);
    lphvideo = NULL;
    structHdrsPtr = NULL;
    return NULL;

}
#endif /*J300_MME*/

/*ARGSUSED*/
static grabproc_t *JV2_Start(int grabtype, int min_framespacing, int config,
			     reconfigproc_t *reconfig, void *enc_state)
{
    JvsSetCompressReq p1;
    JvsSetCompressRep p2;
    int port, status;
    char *ports;

    if (jvfd >= 0) J300_Stop();

    xmit_size = (config & VID_SIZEMASK);

    port = JVS_SOCKET;
    if (ports = getenv("JVDRIVER_PORT")) {
	if (sscanf(ports, "%d", &port) <= 0) port = JVS_SOCKET;
    }

    if ((jvfd = JvsOpen("", port)) < 0) return NULL;

    if (grabtype != VIDIMAGE_YUYV) return NULL;

    p1.qfactor = 0;
    p1.frameskip = 0;
    p1.type = JVS_YUV;

    switch (xmit_size) {
    case VID_SMALL:
	p1.xdec = 4;
	p1.ydec = 4;
	break;
    case VID_MEDIUM:
	p1.xdec = 2;
	p1.ydec = 2;
	break;
    case VID_LARGE:
	p1.xdec = 1;
	p1.ydec = 1;
	break;
    default:
	p1.xdec = 2;
	p1.ydec = 2;
	break;
    }

    JvsSetCompRaw(jvfd, &p1, &p2);
    width = p2.width;
    height = p2.height;
    if ((p1.xdec == 1) && (p1.ydec == 1) && (width*3 != height*4)) {
	/* JV2 can't grab full size in PAL or SECAM mode */
	p1.xdec = 2;
	p1.ydec = 2;
	JvsSetCompRaw(jvfd, &p1, &p2);
	width = p2.width;
	height = p2.height;
    }

    if (!JV2_AllocBuf(&grabbuf[0])) {
	close(jvfd);
	jvfd = -1;
	return NULL;
    }

    if (!JV2_AllocBuf(&grabbuf[1])) {
	JV2_FreeBuf(&grabbuf[0]);
	close(jvfd);
	jvfd = -1;
	return NULL;
    }

    current = 0;
    gettimeofday(&lastgrab, NULL);
    (void) JvsStartComp(jvfd, grabbuf[current].shmid);
    
    (*reconfig)(enc_state, width, height);
    return JV2_Grab;
}

grabproc_t *J300_Start(int grabtype, int min_framespacing, int config,
		       reconfigproc_t *reconfig, void *enc_state)
{
    Tcl_VarEval(interp, "j300DisableControls", NULL);

    J300_Probe();
    if (jv2Grabber) {
	return JV2_Start(grabtype, min_framespacing, config, reconfig,
			 enc_state);
    } else {
#ifdef J300_MME
	return MME_Start(grabtype, min_framespacing, config, reconfig,
			 enc_state);
#else
	return NULL;
#endif
    }
}

void J300_Stop(void)
{
    Tcl_VarEval(interp, "j300EnableControls", NULL);

    if (jv2Grabber) {
	JV2_FreeBuf(&grabbuf[0]);
	JV2_FreeBuf(&grabbuf[1]);
	close(jvfd);
	jvfd = -1;
    } else {
#ifdef J300_MME
	videoClose(*lphvideo);
	mmeFreeMem(structHdrsPtr);
	lphvideo = NULL;
	structHdrsPtr = NULL;
#endif
    }
}
#endif /* J300 */
