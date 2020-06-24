/*
	Netvideo version 3.2
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
#include <sys/types.h>
#include "video.h"
#include "vidcode.h"
#include "vidgrab.h"

#define VIDENCODE_THRESHOLD	64
#define AGE_MOTION_PENALTY	8

#define SETPRI(prip, pri) (((pri) > *(prip)) ? (*(prip) = (pri)) : *(prip))

static int width, height, block_width, block_height;
static short *sendpri, *age;

static struct {
    u_short width, height;
} videncode_hdr;

static u_char videncode_buf[262144], *videncode_datap=videncode_buf;
static int videncode_len;
static videncode_proc_t *videncode_callback;

static void VidEncode_EndBlock(int flush)
{
    static u_char *packp=videncode_buf, *last_datap=videncode_buf;

    if (videncode_datap-packp > videncode_len) {
	videncode_callback((u_char *) &videncode_hdr, sizeof(videncode_hdr),
	    packp, last_datap-packp, 0);
	packp = last_datap;
    }

    if (flush) {
	videncode_callback((u_char *) &videncode_hdr, sizeof(videncode_hdr),
	    packp, last_datap-packp, 1);
	packp = videncode_datap = videncode_buf;
    }

    last_datap = videncode_datap;
}

static int VidEncode_ImageDiff(u_char block1[8], u_char block2[8])
{
    register int total;

#define	BLOCKDIFF(x)	(block1[x]-block2[x])
    total = BLOCKDIFF(0) + BLOCKDIFF(1) + BLOCKDIFF(2) + BLOCKDIFF(3)
	  + BLOCKDIFF(4) + BLOCKDIFF(5) + BLOCKDIFF(6) + BLOCKDIFF(7);
#undef	BLOCKDIFF

    return (total >= 0) ? total : -total;
}

static int VidEncode_PutBlock(u_char *curr_y_data, s_char *curr_uv_data,
			      int x0, int y0, int w, int loss)
{
    register int i, zcount, rem;
    register u_char b, *yp;
    register s_char *uvp;
    register short *agep;
    register signed char *blkp;
    register u_char *orig_datap=videncode_datap;
    register u_long blkw, *blkwp, *blkwLim;
    static u_long block[32];

    *videncode_datap++ = w;
    *videncode_datap++ = x0;
    *videncode_datap++ = y0;

    yp = &curr_y_data[y0*8*width+x0*8];
    uvp = curr_uv_data? &curr_uv_data[y0*8*width+x0*8] : 0;
    agep = &age[y0*block_width+x0];
    while (w-- > 0) {
	VidTransform_Fwd(yp, uvp, width, block);
	blkp = (s_char *)block;

	if (loss > 0) {
	    blkp = (s_char *)block;
	    if ((blkp[2]>=-loss) && (blkp[2]<=loss)) blkp[2] = 0;
	    if ((blkp[3]>=-loss) && (blkp[3]<=loss)) blkp[3] = 0;
	    if ((blkp[4]>=-loss) && (blkp[5]<=loss)) blkp[4] = 0;
	    if ((blkp[5]>=-loss) && (blkp[5]<=loss)) blkp[5] = 0;
	    if ((blkp[6]>=-loss) && (blkp[6]<=loss)) blkp[6] = 0;
	    if ((blkp[7]>=-loss) && (blkp[7]<=loss)) blkp[7] = 0;
	    blkp = ((signed char *)block)+10;
	    for (i=10; i<64; i+=6, blkp+=6) {
		if ((blkp[0]>=-loss) && (blkp[0]<=loss)) blkp[0] = 0;
		if ((blkp[1]>=-loss) && (blkp[1]<=loss)) blkp[1] = 0;
		if ((blkp[2]>=-loss) && (blkp[2]<=loss)) blkp[2] = 0;
		if ((blkp[3]>=-loss) && (blkp[3]<=loss)) blkp[3] = 0;
		if ((blkp[4]>=-loss) && (blkp[4]<=loss)) blkp[4] = 0;
		if ((blkp[5]>=-loss) && (blkp[5]<=loss)) blkp[5] = 0;
	    }

	    if (uvp) {
		blkp = ((signed char *)block)+64;
		if ((blkp[2]>=-2*loss) && (blkp[2]<=2*loss)) blkp[2] = 0;
		if ((blkp[3]>=-2*loss) && (blkp[3]<=2*loss)) blkp[3] = 0;
		if ((blkp[4]>=-2*loss) && (blkp[5]<=2*loss)) blkp[4] = 0;
		if ((blkp[5]>=-2*loss) && (blkp[5]<=2*loss)) blkp[5] = 0;
		if ((blkp[6]>=-2*loss) && (blkp[6]<=2*loss)) blkp[6] = 0;
		if ((blkp[7]>=-2*loss) && (blkp[7]<=2*loss)) blkp[7] = 0;
		blkp = ((signed char *)block)+74;
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
	blkwLim = uvp? block+32 : block+16;
	i = 2;
	zcount = 0;
	blkw = ntohl(*blkwp++);
	videncode_datap[1] = blkw >> 24;
	blkw <<= 8;
	rem = 3;
	do {
	    while ((b = blkw >> 24) != 0) {
		videncode_datap[i++] = b;
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

	    *videncode_datap = ((i-1) << 6) + zcount;
	    videncode_datap += i;
	    i = 1;
	    zcount = 0;
	} while ((rem != 0) || (blkwp != blkwLim));

	agep++;
	yp += 8;
	if (uvp) uvp += 8;
    }

    VidEncode_EndBlock(0);
    return videncode_datap-orig_datap;
}

void VidEncode_Init(int w, int h)
{
    width = w;
    height = h;
    videncode_hdr.width = htons(w);
    videncode_hdr.height = htons(h);
    block_width = (w+7)/8;
    block_height = (h+7)/8;

    if (sendpri != NULL) free(sendpri);
    sendpri = (short *) malloc(block_height*block_width*sizeof(short));

    if (age != NULL) free(age);
    age = (short *) malloc(block_height*block_width*sizeof(short));

    VidEncode_Reset();
}

void VidEncode_Reset(void)
{
    memset(sendpri, 0, block_height*block_width*sizeof(short));
    memset(age, 0, block_height*block_width*sizeof(short));
}

int VidEncode(u_char *old_y_data, u_char *curr_y_data, s_char *curr_uv_data,
	      int max_packlen, int aging_bytes, videncode_proc_t *callback)
{
    static int scan_cycle=0, last_agex=0, last_agey=0;
    static short *last_agep=NULL;
    register int xmax=block_width-1, ymax=block_height-1;
    register int bytes=0, x0=0, y0=0, w, x, y, offset, pri, age_thresh, max_age;
    register u_char *oldp, *newp;
    register short *prip, *agep, new_age;

    videncode_len = max_packlen;
    videncode_callback = callback;

    videncode_hdr.width &= htons(VIDCODE_WIDTHMASK);
    if (curr_uv_data) videncode_hdr.width |= htons(VIDCODE_COLORFLAG);

    offset = scan_cycle*width;
    oldp = &old_y_data[offset];
    newp = &curr_y_data[offset];
    prip = sendpri;
    for (y=0; y<=ymax; y++) {
	for (x=0; x<=xmax; x++) {
	    if (old_y_data == NULL)
		pri = VIDENCODE_THRESHOLD;
	    else
		pri = VidEncode_ImageDiff(oldp, newp);

	    if (y != 0) {
		if (x != 0) SETPRI(prip-block_width-1, pri-8);
		SETPRI(prip-block_width, pri-4);
		if (x != xmax) SETPRI(prip-block_width+1, pri-8);
	    }

	    if (x != 0) SETPRI(prip-1, pri-4);
	    SETPRI(prip, pri);
	    if (x != xmax) SETPRI(prip+1, pri-4);

	    if (y != ymax) {
		if (x != 0) SETPRI(prip+block_width-1, pri-8);
		SETPRI(prip+block_width, pri-4);
		if (x != xmax) SETPRI(prip+block_width+1, pri-8);
	    }

	    oldp += 8;
	    newp += 8;
	    prip++;
	}
	oldp += 8*(width-block_width);
	newp += 8*(width-block_width);
    }
    scan_cycle = (scan_cycle+1) % 8;

    max_age = 0;
    prip = sendpri;
    agep = age;
    w = 0;
    for (y=0; y<=ymax; y++) {
	for (x=0; x<=xmax; x++) {
	    if (*prip >= VIDENCODE_THRESHOLD) {
		if (w == 0) {
		    x0 = x;
		    y0 = y;
		}
		*prip = 0;
		new_age = *agep - AGE_MOTION_PENALTY;
		*agep = (new_age < 0) ? 0 : new_age;
		w++;
		if (w == VIDCODE_MAX_RUNW) {
		    bytes += VidEncode_PutBlock(curr_y_data, curr_uv_data,
						x0, y0, w, 2);
		    w = 0;
		}
	    } else {
		if (++(*agep) > max_age) max_age = *agep;
		if (w > 0) {
		    bytes += VidEncode_PutBlock(curr_y_data, curr_uv_data,
						x0, y0, w, 2);
		    w = 0;
		}
	    }

	    prip++;
	    agep++;
	}

	if (w > 0) {
	    bytes += VidEncode_PutBlock(curr_y_data, curr_uv_data,
					x0, y0, w, 2);
	    w = 0;
	}
    }

    if (last_agep == NULL) last_agep = age;
    age_thresh = max_age/2;
    while (aging_bytes > 0) {
	if (*last_agep > age_thresh) {
	    int blocklen=VidEncode_PutBlock(curr_y_data, curr_uv_data,
					    last_agex, last_agey, 1, 0);
	    *last_agep = 0;
	    bytes += blocklen;	
	    aging_bytes -= blocklen;
	}
	last_agep++;
	if (++last_agex > xmax) {
	    last_agex=0;
	    if (++last_agey > ymax) {
		last_agey = 0;
		last_agep = age;
		break;
	    }
	}
    }

    VidEncode_EndBlock(1);
    return bytes;
}
