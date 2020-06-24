/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	IndigoVideo frame grab routines for the Silicon Graphics IRIS Indigo
	with Entry graphics.
	Written by Andrew Cherenson <arc@sgi.com>
*/

#ifdef INDIGO
#include <stdlib.h>
#include <svideo.h>
#include <math.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_code.h"
#include "indigo_grab.h"

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

static int xmit_size, xmit_color;
static SVhandle V;
static svCaptureInfo svci;

static int CaptureFrame(uint8 *y_data, uint8 *uv_data)
{
    void *captureData;
    int w, h;
    uint32 *s;

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
    s = (uint32 *)captureData;
    if (xmit_color) {
	uint32 d1, d2, d3, d4;

	for (h = 0; h < svci.height/2; h++) {
	    for (w = 0; w < svci.width/4; w++) {
		d1 = *s++;
		d2 = *s++;
		d3 = *s++;
		d4 = *s++;
		*y_data++ = d1 >> 4;
		*y_data++ = d3 >> 4;
		*uv_data++ = ((d1 << 4) & 0xc0)		/* u */
			   | ((d2 << 2) & 0x30)
			   | ((d3) & 0x0c)
			   | ((d4 >> 2) & 0x03);
		*uv_data++ = ((d1 << 6) & 0xc0)		/* v */
			   | ((d2 << 4) & 0x30)
			   | ((d3 << 2) & 0x0c)
			   | ((d4) & 0x03);
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

int Indigo_Probe(void)
{
    if ((V = svOpenVideo()) == NULL) {
	if (svideo_errno != SV_EXCLUSIVE) return 0;
    } else {
	(void) svCloseVideo(V);
	V = NULL;
    }

    return VID_MEDIUM|VID_GREYSCALE|VID_COLOR;
}
 
char *Indigo_Attach(void)
{
    return NULL;
}
 
void Indigo_Detach(void)
{
}

/*ARGSUSED*/
grabproc_t *Indigo_Start(int min_framespacing, int config,
			 reconfigproc_t *reconfig, void *enc_state)
{
    long params[2];

    if (V != NULL) Indigo_Stop();

    xmit_size = (config & VID_SIZEMASK);
    xmit_color = (config & VID_COLOR);

    if ((V = svOpenVideo()) == NULL) return NULL;
    if (svUseExclusive(V, TRUE, SV_IN_OUT) < 0) goto failed;

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

    if (min_framespacing < 100) {
	svci.samplingrate = DEFAULT_SAMPLING_RATE;
    } else {
	svci.samplingrate = (int) ceilf(30.0*min_framespacing/1000.0);
    }

    svci.format = SV_YUV411_FRAMES;
    svci.size = 1;			/* # of capture ring buffers */
    if (svInitContinuousCapture(V, &svci) < 0) {
	svPerror("init capt");
	goto failed;
    }

    (*reconfig)(enc_state, xmit_color, svci.width/2, svci.height/2);
    return CaptureFrame;

failed:
    (void) svCloseVideo(V);
    V = NULL;
    return NULL;
}

void Indigo_Stop(void)
{
    (void) svCloseVideo(V);
    V = NULL;
}
#endif /* INDIGO */
