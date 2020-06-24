/*
	Netvideo version 2.6
	Written by Ron Frederick <frederick@parc.xerox.com>

	IndigoVideo frame grab routines for the Silicon Graphics IRIS Indigo
	with Entry graphics.
	Written by Andrew Cherenson <arc@sgi.com>
*/

#include <stdlib.h>
#include <svideo.h>
#include <math.h>

/*
 * The sampling rate specifies the number of frames seen for each
 * frame captured. The minimum value is 2.
 *
 * For the Indigo R3000, a value of 2 consumes too much CPU time, so use 3.
 * For the Indigo R4000, a value of 2 is fine.
 *
 * Sampling			Approx. CPU Usage
 *  rate	Frames/s	R3000	R4000
 *   2		 15		 >90%	 10%
 *   3		 10		  31	  8
 *   4		  7.5		  23	 <8
 *   5		  6		  18	 <8
 *  30		  1		   2	  1
 *				(quiescent scene at 128 kbps)
 */

#define DEFAULT_SAMPLING_RATE  3		/* 10 fps */


/*----------------------------------------------------------------------*/

int (*GrabImage)();

static SVhandle V;
static svCaptureInfo svci;

#define OPLEN  6
static long origparams[OPLEN] = {
    SV_COLOR, 0,
    SV_DITHER, 0,
    SV_BROADCAST, 0		/* just piggybacking on the getparams call */
};
#define BCAST_VAL 5

static int paramsChanged;		/* True if color/dither changed */



static void
Interleave(unsigned char *src, unsigned char *dst, int width, int height)
{
    unsigned char *s, *d;
    int	i, j, k, half = height / 2;

    /*
     * Data from the second ("odd") video field come first, followed by 
     * data from the first ("even") field.  In the interleaved frame, an 
     * even line is followed by an odd line. While interleaving, convert
     * the pixels to 7-bit grayscale.
     *
     * Assume width is a multiple of 8.
     */
    for (i = 0, j = 1; i < half; i++, j += 2) {		/* odd field */
	s = &src[i * width];
	d = &dst[j * width];
	k = width / 8;
	while (--k >= 0) {
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	}
    }
    for (i = half, j = 0; i < height; i++, j += 2) {	/* even field */
	s = &src[i * width];
	d = &dst[j * width];
	k = width / 8;
	while (--k >= 0) {
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	    *d++ = *s++ >> 1;
	}
    }
}

static int
CaptureFrame(unsigned char *image)
{
    void *captureData;

    while (1) {
	(void) svGetCaptureData(V, &captureData, NULL);
	if (captureData != NULL) {
	    break;
	}
	sginap(1);	/* wait a tick */
    }
    Interleave(captureData, image, svci.width, svci.height);
    (void) svUnlockCaptureData(V, captureData);
    return 1;
}


void
GrabImage_Cleanup(void)
{
    if (paramsChanged) {
	/* Just restore color & dither */
	(void) svSetParam(V, origparams, 4);
    }
    (void) svCloseVideo(V);
}

int
GrabImage_Init(int framerate, int *widthp, int *heightp)
{
    long params[4];

    /* Open the video device */
    if ((V = svOpenVideo()) == NULL) {
	goto failed;
    }

    /* Determine the broadcast standard & color modes */
    if (svGetParam(V, origparams, OPLEN) < 0) {
	svPerror("getparam");
	goto failed;
    }

    /*
     * Set sizes based on broadcast standard.
     * N.B. the Interleave routine assumes the width is a multiple of 8.
     */
    if (origparams[BCAST_VAL] == SV_PAL) {
	svci.width = SV_PAL_XMAX / 2;
	svci.height = SV_PAL_YMAX / 2;
    } else {
	svci.width = SV_NTSC_XMAX / 2;
	svci.height = SV_NTSC_YMAX / 2;
    }

    /* Select black & white non-dithered mode */
    params[0] = SV_COLOR;
    params[1] = SV_MONO;
    params[2] = SV_DITHER;
    params[3] = FALSE;
    if (svSetParam(V, params, sizeof(params)/sizeof(params[0])) < 0) {
	svPerror("setparam");
	goto failed;
    }
    paramsChanged = 1;

    if (framerate <= 0 || framerate > 15) {
	svci.samplingrate = DEFAULT_SAMPLING_RATE;
    } else {
	svci.samplingrate = (int) ceilf(30.0 / (float)framerate);
    }
    svci.format = SV_RGB8_FRAMES;
    svci.size = 1;			/* # of capture ring buffers */
    if (svInitContinuousCapture(V, &svci)< 0) {
	svPerror("init capt");
	goto failed;
    }

    GrabImage = CaptureFrame;
    *widthp = svci.width;
    *heightp = svci.height;
    return 1;

failed:
    (void) svCloseVideo(V);
    *widthp = *heightp = 0;
    return 0;
}
