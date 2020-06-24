/*
	Netvideo version 3.2
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

int (*GrabImage)(u_char *y_data, signed char *uv_data);

static SVhandle V;
static svCaptureInfo svci;


static int
CaptureFrame(u_char *y_data, signed char *uv_data)
{
    void *captureData;
    unsigned int *s;
    int w, h;

    while (1) {
	(void) svGetCaptureData(V, &captureData, NULL);
	if (captureData != NULL) {
	    break;
	}
	sginap(1);	/* wait a tick */
    }

    /*
     * Decimate the full-size image by 2 in each dimension.
     */
    s = (unsigned int *)captureData;
    if (uv_data != NULL) {
	unsigned int d1, d2, d3, d4;
	unsigned char c;

	for (h = 0; h < svci.height/2; h++) {
	    for (w = 0; w < svci.width/4; w++) {
		d1 = *s++;
		d2 = *s++;
		d3 = *s++;
		d4 = *s++;
		*y_data++ = d1 >> 4;
		*y_data++ = d3 >> 4;
		c = ((d1 << 4) & 0xc0)		/* u */
		    | ((d2 << 2) & 0x30)
		    | ((d3) & 0x0c)
		    | ((d4 >> 2) & 0x03);
		*uv_data++ = c - 128;
		c = ((d1 << 6) & 0xc0)		/* v */
		    | ((d2 << 4) & 0x30)
		    | ((d3 << 2) & 0x0c)
		    | ((d4) & 0x03);
		*uv_data++ = c - 128;
	    }
	}
    } else {
	for (h = 0; h < svci.height/2; h++) {
	    for (w = 0; w < svci.width/2; w++) {
		*y_data++ = *s >> 4;
		s += 2;
	    }
	}
    }
    (void) svUnlockCaptureData(V, captureData);
    return 1;
}


void
GrabImage_Cleanup(void)
{
    (void) svCloseVideo(V);
}

int
GrabImage_Init(int framerate, int *widthp, int *heightp)
{
    long params[2];

    /* Open the video device */
    if ((V = svOpenVideo()) == NULL) {
	goto failed;
    }

    /*
     * Set sizes based on broadcast standard.
     */
    params[0] = SV_BROADCAST;
    if (svGetParam(V, params, 2) < 0) {
	svPerror("getparam");
	goto failed;
    }
    if (params[1] == SV_PAL) {
	svci.width = SV_PAL_XMAX;
	svci.height = SV_PAL_YMAX;
    } else {
	svci.width = SV_NTSC_XMAX ;
	svci.height = SV_NTSC_YMAX;
    }

    if (framerate <= 0 || framerate > 15) {
	svci.samplingrate = DEFAULT_SAMPLING_RATE;
    } else {
	svci.samplingrate = (int) ceilf(30.0 / (float)framerate);
    }
    svci.format = SV_YUV411_FRAMES;
    svci.size = 1;			/* # of capture ring buffers */
    if (svInitContinuousCapture(V, &svci)< 0) {
	svPerror("init capt");
	goto failed;
    }

    GrabImage = CaptureFrame;
    *widthp = svci.width / 2;
    *heightp = svci.height / 2;
    return 1;

failed:
    (void) svCloseVideo(V);
    *widthp = *heightp = 0;
    return 0;
}
