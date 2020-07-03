/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video encode routines
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#ifdef AIX
#include <net/nh.h>
#endif
#include <X11/Xlib.h>
#include "sized_types.h"
#include "vid_util.h"
#include "vid_image.h"
#include "vid_code.h"
#include "nv.h"

#define AGE_THRESHOLD		8
#define XMIT_THRESHOLD		64

int nv_aging=1;

typedef struct {
    grabber_t *grabber;
    grabproc_t *grab;
    int max_bandwidth, min_framespacing, config;
    int grabtype, xmit_color, width, height, block_width, block_height;
    int start_x, start_y, age_x, age_y, run_x, run_y, frame_ok, scan_cycle;
    int aging_bytes, aging_count, encoding;
    uint32 rtp_frame_time;
    uint8 *grabdata;
    uint8 *xmit, *age;
    struct timeval xmit_time, frame_time;
    nv_fwdtransform_t *fwdTransform;
} nv_state_t;

typedef struct {
    uint16 width, height;
} nvhdr_t;

static void NV_ImageDiff_Grey(nv_state_t *state, vidimage_t *image)
{
    int x, y, offset, shift, mask=0xff00ff, w=state->width, h=state->height;
    int bw=state->block_width, bh=state->block_height, xmax=bw-1, ymax=bh-1;
    int32 pri;
    uint8 *xmitp=state->xmit;
    uint32 *oldp, *newp;

    if ((image == NULL) || (image->width != w) || (image->height != h)) {
	memset(state->xmit, 1, bw*bh);
    } else {
	offset = (state->scan_cycle & 3)*w;
	shift = (state->scan_cycle & 4)*2;
	oldp = (uint32 *) &image->y_data[offset];
	newp = (uint32 *) &state->grabdata[offset];

	for (y=0; y<=ymax; y++) {
	    for (x=0; x<=xmax; x++) {
		pri = ((newp[0]>>shift) & mask) +
		      ((newp[1]>>shift) & mask) +
		      ((newp[w]>>shift) & mask) +
		      ((newp[w+1]>>shift) & mask) -
		      ((oldp[0]>>shift) & mask) -
		      ((oldp[1]>>shift) & mask) -
		      ((oldp[w]>>shift) & mask) -
		      ((oldp[w+1]>>shift) & mask);
		pri = ((pri << 16) + pri) >> 16;
		if (pri < 0) pri = -pri;

		if (pri >= XMIT_THRESHOLD) xmitp[0] = 1;

		if (pri >= XMIT_THRESHOLD+4) {
		    if (x != 0) xmitp[-1] = 1;
		    if (x != xmax) xmitp[1] = 1;
		    if (y != 0) xmitp[-bw] = 1;
		    if (y != ymax) xmitp[bw] = 1;
		}

		if (pri >= XMIT_THRESHOLD+8) {
		    if ((x != 0) && (y != 0)) xmitp[-bw-1] = 1;
		    if ((x != xmax) && (y != 0)) xmitp[-bw+1] = 1;
		    if ((x != 0) && (y != ymax)) xmitp[bw-1] = 1;
		    if ((x != xmax) && (y != ymax)) xmitp[bw+1] = 1;
		}

		oldp += 2;
		newp += 2;
		xmitp++;
	    }

	    oldp += 7*w/4;
	    newp += 7*w/4;
	}

	state->scan_cycle = (state->scan_cycle+1) & 7;
    }
}

static void NV_ImageDiff_YUYV(nv_state_t *state, vidimage_t *image)
{
    int x, y, offset, shift, shiftfix, newshift, mask=0xff00ff;
    int w=state->width, h=state->height;
    int bw=state->block_width, bh=state->block_height, xmax=bw-1, ymax=bh-1;
    int32 pri;
    uint8 *xmitp=state->xmit;
    uint32 *oldp, *newp;

    shiftfix = ((state->grabtype == VIDIMAGE_YUYV) ^ LITTLEENDIAN) ? 8 : 0;

    if ((image == NULL) || (image->width != w) || (image->height != h)) {
	memset(state->xmit, 1, bw*bh);
    } else {
	offset = (state->scan_cycle & 3)*w;
	shift = (state->scan_cycle & 4)*2;
	newshift = shift*2 + shiftfix;
	oldp = (uint32 *) &image->y_data[offset];
	newp = (uint32 *) &state->grabdata[offset*2];

	for (y=0; y<=ymax; y++) {
	    for (x=0; x<=xmax; x++) {
		pri = ((oldp[0]>>shift) & mask) +
		      ((oldp[1]>>shift) & mask) +
		      ((oldp[w]>>shift) & mask) +
		      ((oldp[w+1]>>shift) & mask);
		pri = ((pri << 16) + pri) >> 16;
		pri = ((newp[0]>>newshift) & 0xff) +
		      ((newp[1]>>newshift) & 0xff) +
		      ((newp[2]>>newshift) & 0xff) +
		      ((newp[3]>>newshift) & 0xff) +
		      ((newp[2*w]>>newshift) & 0xff) +
		      ((newp[2*w+1]>>newshift) & 0xff) +
		      ((newp[2*w+2]>>newshift) & 0xff) +
		      ((newp[2*w+3]>>newshift) & 0xff) -
		      pri;
		if (pri < 0) pri = -pri;

		if (pri >= XMIT_THRESHOLD) xmitp[0] = 1;

		if (pri >= XMIT_THRESHOLD+4) {
		    if (x != 0) xmitp[-1] = 1;
		    if (x != xmax) xmitp[1] = 1;
		    if (y != 0) xmitp[-bw] = 1;
		    if (y != ymax) xmitp[bw] = 1;
		}

		if (pri >= XMIT_THRESHOLD+8) {
		    if ((x != 0) && (y != 0)) xmitp[-bw-1] = 1;
		    if ((x != xmax) && (y != 0)) xmitp[-bw+1] = 1;
		    if ((x != 0) && (y != ymax)) xmitp[bw-1] = 1;
		    if ((x != xmax) && (y != ymax)) xmitp[bw+1] = 1;
		}

		oldp += 2;
		newp += 4;
		xmitp++;
	    }

	    oldp += 7*w/4;
	    newp += 7*w/2;
	}

	state->scan_cycle = (state->scan_cycle+1) & 7;
    }
}

static uint8 *NV_PutBlock(nv_state_t *state, uint8 *data, uint8 **runhdrp,
			  int x, int y, int loss)
{
    int i, offset, zcount, rem;
    uint8 b, *runhdr=*runhdrp;
    uint32 blkw, *blkwp, *blkwLim, block[32];
    int8 *blkp;

    if ((runhdr == NULL) || (state->run_x != x) || (state->run_y != y)) {
	*runhdrp = runhdr = data;
	runhdr[0] = 1;
	runhdr[1] = state->run_x = x;
	runhdr[2] = state->run_y = y;
	data += 3;
    } else {
	runhdr[0]++;
    }

    offset = y*8*state->width+x*8;
    if (state->grabtype != VIDIMAGE_GREY) offset *= 2;

    (*state->fwdTransform)(state->grabtype, &state->grabdata[offset],
			   state->width, block);

    blkp = (int8 *)block;

    if (loss > 0) {
	blkp = (int8 *)block;
	if ((blkp[2]>=-loss) && (blkp[2]<=loss)) blkp[2] = 0;
	if ((blkp[3]>=-loss) && (blkp[3]<=loss)) blkp[3] = 0;
	if ((blkp[4]>=-loss) && (blkp[4]<=loss)) blkp[4] = 0;
	if ((blkp[5]>=-loss) && (blkp[5]<=loss)) blkp[5] = 0;
	if ((blkp[6]>=-loss) && (blkp[6]<=loss)) blkp[6] = 0;
	if ((blkp[7]>=-loss) && (blkp[7]<=loss)) blkp[7] = 0;
	blkp = ((int8 *)block)+10;
	for (i=10; i<64; i+=6, blkp+=6) {
	    if ((blkp[0]>=-loss) && (blkp[0]<=loss)) blkp[0] = 0;
	    if ((blkp[1]>=-loss) && (blkp[1]<=loss)) blkp[1] = 0;
	    if ((blkp[2]>=-loss) && (blkp[2]<=loss)) blkp[2] = 0;
	    if ((blkp[3]>=-loss) && (blkp[3]<=loss)) blkp[3] = 0;
	    if ((blkp[4]>=-loss) && (blkp[4]<=loss)) blkp[4] = 0;
	    if ((blkp[5]>=-loss) && (blkp[5]<=loss)) blkp[5] = 0;
	}

	if (state->xmit_color) {
	    blkp = ((int8 *)block)+64;
	    if ((blkp[2]>=-2*loss) && (blkp[2]<=2*loss)) blkp[2] = 0;
	    if ((blkp[3]>=-2*loss) && (blkp[3]<=2*loss)) blkp[3] = 0;
	    if ((blkp[4]>=-2*loss) && (blkp[4]<=2*loss)) blkp[4] = 0;
	    if ((blkp[5]>=-2*loss) && (blkp[5]<=2*loss)) blkp[5] = 0;
	    if ((blkp[6]>=-2*loss) && (blkp[6]<=2*loss)) blkp[6] = 0;
	    if ((blkp[7]>=-2*loss) && (blkp[7]<=2*loss)) blkp[7] = 0;
	    blkp = ((int8 *)block)+74;
	    for (i=10; i<64; i+=6, blkp+=6) {
		if ((blkp[0]>=-2*loss) && (blkp[0]<=2*loss)) blkp[0] = 0;
		if ((blkp[1]>=-2*loss) && (blkp[1]<=2*loss)) blkp[1] = 0;
		if ((blkp[2]>=-2*loss) && (blkp[2]<=2*loss)) blkp[2] = 0;
		if ((blkp[3]>=-2*loss) && (blkp[3]<=2*loss)) blkp[3] = 0;
		if ((blkp[4]>=-2*loss) && (blkp[4]<=2*loss)) blkp[4] = 0;
		if ((blkp[5]>=-2*loss) && (blkp[5]<=2*loss)) blkp[5] = 0;
	    }
	}
    }

    /* XXX: The ntohl() calls here need to be redone as something more
	    efficient for little-endian machines! */
    blkwp = block;
    blkwLim = state->xmit_color? block+32 : block+16;
    i = 2;
    zcount = 0;
    blkw = ntohl(*blkwp++);
    data[1] = blkw >> 24;
    blkw <<= 8;
    rem = 3;
    do {
	while ((b = blkw >> 24) != 0) {
	    data[i++] = b;
	    blkw <<= 8;
	    rem--;
	    if (i == 4) break;
	    if (rem == 0) {
		if (blkwp == blkwLim) break;
		blkw = ntohl(*blkwp++);
		rem = 4;
	    }
	}

	while (1) {
	    if (zcount+rem >= 63) {
		break;
	    } else if (blkw == 0) {
		zcount += rem;
		rem = 0;
		if (blkwp == blkwLim) break;
		blkw = ntohl(*blkwp++);
		rem = 4;
	    } else if ((blkw >> 24) == 0) {
		zcount++;
		blkw <<= 8;
		rem--;
	    } else break;
	}

	*data = ((i-1) << 6) + zcount;
	data += i;
	i = 1;
	zcount = 0;
    } while ((rem != 0) || (blkwp != blkwLim));

    if (++state->run_x == state->block_width) state->run_x = -1;

    return data;
}

static int NV_SendMotion(nv_state_t *state, uint8 *data, uint8 *dataLim,
			 int *syncp)
{
    int x, y, offset;
    uint8 *p=data, *runhdr=NULL, *xmitp, *agep;

    /* Leave room for the largest single block, so we don't get partial ones */
    dataLim -= 175;

    offset = state->start_y*state->block_width+state->start_x;
    xmitp = &state->xmit[offset];
    agep = &state->age[offset];
    for (y=state->start_y; y < state->block_height; y++) {
	for (x=state->start_x; x < state->block_width; x++) {
	    if (*xmitp) {
		*xmitp = 0;
		*agep = 1;
		p = NV_PutBlock(state, p, &runhdr, x, y, 2);
	    } else if ((*agep > 0) && (++*agep == AGE_THRESHOLD)) {
		*agep = 0;
		p = NV_PutBlock(state, p, &runhdr, x, y, 0);
	    }

	    if (p >= dataLim) {
		state->start_x = x;
		state->start_y = y;
		*syncp = 0;
		return p-data;
	    }

	    xmitp++;
	    agep++;
	}
	state->start_x = 0;
    }

    state->start_x = state->start_y = 0;
    *syncp = 1;
    return p-data;
}

static int NV_SendBackground(nv_state_t *state, uint8 *data, uint8 *dataLim,
			     int *syncp)
{
    int x, y, offset;
    uint8 *p=data, *runhdr=NULL, *xmitp, *agep;

    /* Leave room for the largest single block, so we don't get partial ones */
    dataLim -= 175;

    offset = state->age_y*state->block_width+state->age_x;
    xmitp = &state->xmit[offset];
    agep = &state->age[offset];
    for (y=state->age_y; y < state->block_height; y++) {
	for (x=state->age_x; x < state->block_width; x++) {
	    if (*agep == 0) {
		*xmitp = 0;
		p = NV_PutBlock(state, p, &runhdr, x, y, 0);

		if (p >= dataLim) {
		    state->age_x = x;
		    state->age_y = y;
		    *syncp = 0;
		    return p-data;
		}
	    }

	    xmitp++;
	    agep++;
	}
	state->age_x = 0;
    }

    state->age_x = state->age_y = 0;
    *syncp = 0;
    return p-data;
}

static void NV_Encode_Reconfig(void *enc_state, int w, int h)
{
    nv_state_t *state=enc_state;
    int image_size, block_count;

    state->width = w;
    state->height = h;
    state->block_width = (w+7)/8;
    state->block_height = (h+7)/8;

    image_size = state->width*state->height;
    block_count = state->block_width*state->block_height;

    if (state->xmit != NULL) free(state->xmit);
    state->xmit = (uint8 *) malloc(block_count);
    memset(state->xmit, 0, block_count);

    if (state->age != NULL) free(state->age);
    state->age = (uint8 *) malloc(block_count);
    memset(state->age, 0, block_count);

    state->start_x = state->start_y = 0;
    state->age_x = state->age_y = 0;
    state->run_x = state->run_y = -1;
    state->frame_ok = state->scan_cycle = 0;
    state->aging_bytes = state->aging_count = 0;
    state->encoding = NV_ENCODING_STD;
    state->fwdTransform = NV_FwdTransform;
}

static int NV_Encode_GrabStart(nv_state_t *state, int min_framespacing,
			       int config)
{
    if (config & VID_COLOR) {
	if ((state->grab = (*state->grabber->start)(VIDIMAGE_YUYV,
						    min_framespacing,
						    config, NV_Encode_Reconfig,
						    state)) != 0) {
	    state->grabtype = VIDIMAGE_YUYV;
	} else if ((state->grab = (*state->grabber->start)(VIDIMAGE_UYVY,
							   min_framespacing,
							   config,
							   NV_Encode_Reconfig,
							   state)) != 0) {
	    state->grabtype = VIDIMAGE_UYVY;
	} else if ((state->grab = (*state->grabber->start)(VIDIMAGE_GREY,
							   min_framespacing,
							   config,
							   NV_Encode_Reconfig,
							   state)) != 0) {
	    state->grabtype = VIDIMAGE_GREY;
	} else {
	    return 0;
	}
    } else {
	if ((state->grab = (*state->grabber->start)(VIDIMAGE_GREY,
						    min_framespacing,
						    config, NV_Encode_Reconfig,
						    state)) != 0) {
	    state->grabtype = VIDIMAGE_GREY;
	} else if ((state->grab = (*state->grabber->start)(VIDIMAGE_YUYV,
							   min_framespacing,
							   config,
							   NV_Encode_Reconfig,
							   state)) != 0) {
	    state->grabtype = VIDIMAGE_YUYV;
	} else if ((state->grab = (*state->grabber->start)(VIDIMAGE_UYVY,
							   min_framespacing,
							   config,
							   NV_Encode_Reconfig,
							   state)) != 0) {
	    state->grabtype = VIDIMAGE_UYVY;
	} else {
	    return 0;
	}
    }

    if ((config & VID_COLOR) && (state->grabtype != VIDIMAGE_GREY))
	state->xmit_color = 1;
    else
	state->xmit_color = 0;

    return 1;
}

/*ARGSUSED*/
int NV_Encode_Probe(grabber_t *grabber)
{
    return 1;
}

void *NV_Encode_Start(grabber_t *grabber, int max_bandwidth,
		      int min_framespacing, int config)
{
    nv_state_t *state;

    if ((state = (nv_state_t *)malloc(sizeof(nv_state_t))) == NULL) return 0;
    memset(state, 0, sizeof(nv_state_t));

    state->grabber = grabber;
    if (!NV_Encode_GrabStart(state, min_framespacing, config)) {
	free(state);
	return 0;
    }

    state->max_bandwidth = max_bandwidth;
    state->min_framespacing = min_framespacing;
    state->config = config;

    gettimeofday(&state->xmit_time, NULL);
    state->frame_time = state->xmit_time;

    return state;
}

void *NV_Encode_Restart(void *enc_state, int max_bandwidth,
			int min_framespacing, int config)
{
    nv_state_t *state=enc_state;

    if ((min_framespacing != state->min_framespacing) ||
	(config != state->config)) {
	if (!NV_Encode_GrabStart(state, min_framespacing, config)) {
	    free(state);
	    return 0;
	}
    }

    state->max_bandwidth = max_bandwidth;
    state->min_framespacing = min_framespacing;
    state->config = config;

    gettimeofday(&state->xmit_time, NULL);
    state->frame_time = state->xmit_time;

    return state;
}

int NV_Encode(void *enc_state, vidimage_t *image, uint8 *data, int *lenp,
	      int *markerp, uint32 *timestampp)
{
    nv_state_t *state=enc_state;
    int len, bytes, sync, tdiff, timeout;
    struct timeval cur_time;
    nvhdr_t *nvhdr=(nvhdr_t *)data;
    uint8 *dataLim=data+*lenp;

    nvhdr->width = htons(state->width);
    if (state->xmit_color) nvhdr->width |= htons(NV_COLORFLAG);
    nvhdr->height = htons(state->encoding|state->height);
    data += sizeof(*nvhdr);

    if (!state->frame_ok) {
	state->rtp_frame_time = RTPTime();
	if ((*state->grab)(&state->grabdata, &len) == 0) return -1;
	state->frame_ok = 1;

	switch (state->grabtype) {
	case VIDIMAGE_GREY:
	    NV_ImageDiff_Grey(state, image);
	    break;
	case VIDIMAGE_YUYV:
	case VIDIMAGE_UYVY:
	    NV_ImageDiff_YUYV(state, image);
	    break;
	}
    }

    if (state->aging_bytes < *lenp) {
	bytes = NV_SendMotion(state, data, dataLim, &sync);
	state->aging_count = 0;
    } else if (++state->aging_count == 4) {
	bytes = NV_SendMotion(state, data, dataLim, &sync);
	state->aging_count = state->aging_bytes = 0;
    } else {
	bytes = NV_SendBackground(state, data, dataLim, &sync);
	state->aging_bytes -= bytes;
    }

    *markerp = sync;
    *timestampp = state->rtp_frame_time;
    *lenp = sizeof(*nvhdr)+bytes;
    state->frame_ok = !sync;

    gettimeofday(&cur_time, NULL);
    tdiff = (cur_time.tv_sec-state->xmit_time.tv_sec)*1000 +
	    (cur_time.tv_usec-state->xmit_time.tv_usec)/1000;
    if ((timeout = bytes*8/state->max_bandwidth-tdiff) < 0) timeout = 0;

    if (sync && (state->min_framespacing > 0)) {
	int frame_timeout;

	tdiff = (cur_time.tv_sec-state->frame_time.tv_sec)*1000 +
		(cur_time.tv_usec-state->frame_time.tv_usec)/1000;
	if ((frame_timeout = state->min_framespacing-tdiff) > timeout)
	    timeout = frame_timeout;
    }

    state->xmit_time = cur_time;
    state->xmit_time.tv_usec += timeout*1000;
    if (sync) state->frame_time = state->xmit_time;
    if (nv_aging) state->aging_bytes += state->max_bandwidth*tdiff/32;

    if (timeout > 0) {
	state->encoding = NV_ENCODING_DCT;
	state->fwdTransform = NVDCT_FwdTransform;
    } else {
	state->encoding = NV_ENCODING_STD;
	state->fwdTransform = NV_FwdTransform;
    }

    return timeout;
}

void NV_Encode_Stop(void *enc_state)
{
    nv_state_t *state=enc_state;

    (*state->grabber->stop)();

    if (state->xmit != NULL) free(state->xmit);
    if (state->age != NULL) free(state->age);

    free(state);
}
