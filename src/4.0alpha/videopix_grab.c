/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	VideoPix frame grab routines
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

#ifdef VIDEOPIX
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <tcl.h>
#include <vfc_lib.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_code.h"
#include "videopix_grab.h"

extern Tcl_Interp *interp;

#ifdef __GNUC__
#ifndef __svr4__
#undef VFCSCTRL
#define VFCSCTRL      _IOW('j', 1, int) /* set ctrl. attr. */
#endif /*__svr4__*/
#endif /*__GNUC__*/

static int xmit_size, xmit_color, width, height;
static VfcDev *vfcdev;
static uint8 *grabdata=NULL;

/*ARGSUSED*/
static int VideoPix_GrabGreySmallNTSC(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, mask, y, *yptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;
 
    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr;
 
    data0 = *dataptr;
    data1 = *dataptr;
    yptr = (uint32 *)grabdata;
    mask = 0xff000000;
    for (i=0; i<height; i++) {
	for (j=0; j<736; j++) data1 = *dataptr;

	for (j=0; j<width; j+=20) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data1 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 8);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 16);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data0 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 8);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 16);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data1 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data0 & mask) >> 8);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 16);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data1 >> 24);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data1 & mask);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data0 & mask) >> 8);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data0 & mask) >> 16);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data1 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data1 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 8);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data0 & mask) >> 16);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data0 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	}
    }

    *datap = grabdata;
    *lenp = width*height;
    return 1;
}

/*ARGSUSED*/
static int VideoPix_GrabGreySmallPAL(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, mask, y, *yptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;
 
    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr;
 
    data0 = *dataptr;
    data1 = *dataptr;
    yptr = (uint32 *)grabdata;
    mask = 0xff000000;
    for (i=0; i<height; i++) {
	for (j=0; j<736; j++) data0 = *dataptr;

	for (j=0; j<width; j+=12) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data0 & mask) >> 8);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data0 & mask) >> 16);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data3 >> 24);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y = (data3 & mask);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data3 & mask) >> 8);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data2 & mask) >> 16);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data2 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y = (data2 & mask);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 8);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 16);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data1 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	}
    }

    *datap = grabdata;
    *lenp = width*height;
    return 1;
}

static int VideoPix_GrabYUYVSmallNTSC(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, uv, yy, *yuyvptr;
    uint32 mask=0xff000000, uvflip=0x00800080;
    volatile uint32 *dataptr=vfcdev->vfc_port1;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr;

    yuyvptr = (uint32 *)grabdata;
    for (i=0; i<height; i++) {
	for (j=0; j<736; j++) data0 = *dataptr;

	for (j=0; j<width; j+=10) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask);

	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;

	    yy += ((data3 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);
	}
    }

    *datap = grabdata;
    *lenp = width*height*2;
    return 1;
}

static int VideoPix_GrabYUYVSmallPAL(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, yy, uv, *yuyvptr;
    uint32 mask=0xff000000, uvflip=0x00800080;
    volatile uint32 *dataptr=vfcdev->vfc_port1;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr;

    yuyvptr = (uint32 *)grabdata;
    for (i=0; i<height; i++) {
	for (j=0; j<736; j++) data0 = *dataptr;

	for (j=0; j<width; j+=12) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);
	}
    }

    *datap = grabdata;
    *lenp = width*height*2;
    return 1;
}

/*ARGSUSED*/
static int VideoPix_GrabGreyMediumNTSC(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, mask, y, *yptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;
 
    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr;
 
    data0 = *dataptr;
    data1 = *dataptr;
    yptr = (uint32 *)grabdata;
    mask = 0xff000000;
    for (i=0; i<height; i++) {
	for (j=0; j<16; j++) data1 = *dataptr;

	for (j=0; j<width; j+=20) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data1 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data3 & mask) >> 8);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 16);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data3 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data1 & mask);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data2 & mask) >> 8);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data0 & mask) >> 16);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data2 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data2 & mask) >> 8);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 16);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data3 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data1 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data3 & mask) >> 8);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 16);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data2 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data2 & mask) >> 8);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data0 & mask) >> 16);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data2 >> 24);
	}
    }

    *datap = grabdata;
    *lenp = width*height;
    return 1;
}

/*ARGSUSED*/
static int VideoPix_GrabGreyMediumPAL(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, mask, y, *yptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;
 
    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr;
 
    data0 = *dataptr;
    data1 = *dataptr;
    yptr = (uint32 *)grabdata;
    mask = 0xff000000;
    for (i=0; i<height; i++) {
	for (j=0; j<16; j++) data0 = *dataptr;

	for (j=0; j<width; j+=24) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data2 & mask) >> 8);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data0 & mask) >> 16);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data2 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data2 & mask) >> 8);
	    y += ((data3 & mask) >> 16);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data1 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y = (data3 & mask);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 8);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data3 & mask) >> 16);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data1 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y = (data2 & mask);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data0 & mask) >> 8);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data2 & mask) >> 16);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data0 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y = (data2 & mask);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data0 & mask) >> 8);
	    y += ((data1 & mask) >> 16);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data3 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data1 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += ((data3 & mask) >> 8);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += ((data1 & mask) >> 16);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data3 >> 24);
	}
    }

    *datap = grabdata;
    *lenp = width*height;
    return 1;
}

static int VideoPix_GrabYUYVMediumNTSC(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, yy, uv, *yuyvptr;
    uint32 mask=0xff000000, uvflip=0x00800080;
    volatile uint32 *dataptr=vfcdev->vfc_port1;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr;

    yuyvptr = (uint32 *)grabdata;
    for (i=0; i<height; i++) {
	for (j=0; j<16; j++) data0 = *dataptr;

	for (j=0; j<width; j+=20) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask) + ((data3 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask) + ((data3 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask) + ((data3 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask) + ((data3 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask) + ((data3 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;

	    yy = (data3 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);
	}
    }

    *datap = grabdata;
    *lenp = width*height*2;
    return 1;
}

static int VideoPix_GrabYUYVMediumPAL(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, yy, uv, *yuyvptr;
    uint32 mask=0xff000000, uvflip=0x00800080;
    volatile uint32 *dataptr=vfcdev->vfc_port1;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr;

    yuyvptr = (uint32 *)grabdata;
    for (i=0; i<height; i++) {
	for (j=0; j<16; j++) data0 = *dataptr;

	for (j=0; j<width; j+=24) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data3 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask) + ((data3 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask) + ((data3 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask) + ((data3 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask) + ((data3 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);
	}
    }

    *datap = grabdata;
    *lenp = width*height*2;
    return 1;
}

/*ARGSUSED*/
static int VideoPix_GrabGreyLargeNTSC(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, mask, y, *yptr;
    volatile uint32 *dataptr1=vfcdev->vfc_port1, *dataptr2=vfcdev->vfc_port2;
    volatile uint32 *dataptr;
 
    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr1;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr2;
 
    data0 = *dataptr1;
    data1 = *dataptr1;
    data0 = *dataptr2;
    data1 = *dataptr2;
    yptr = (uint32 *)grabdata;
    mask = 0xff000000;
    for (i=0; i<height; i++) {
	dataptr = (i & 1) ? dataptr1 : dataptr2;

	for (j=0; j<15; j++) data0 = *dataptr;
	data1 = *dataptr;

	for (j=0; j<width; j+=20) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask) + ((data1 & mask) >> 8);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + ((data2 & mask) >> 16) + (data3 >> 24);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask) + ((data1 & mask) >> 8);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + ((data2 & mask) >> 16) + (data3 >> 24);

	    data2 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask) + ((data1 & mask) >> 8);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + ((data2 & mask) >> 16) + (data3 >> 24);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask) + ((data1 & mask) >> 8);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + ((data2 & mask) >> 16) + (data3 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask) + ((data1 & mask) >> 8);

	    data0 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + ((data2 & mask) >> 16) + (data3 >> 24);
	}
    }

    *datap = grabdata;
    *lenp = width*height;
    return 1;
}

/*ARGSUSED*/
static int VideoPix_GrabGreyLargePAL(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, mask, y, *yptr;
    volatile uint32 *dataptr1=vfcdev->vfc_port1, *dataptr2=vfcdev->vfc_port2;
    volatile uint32 *dataptr;
 
    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr1;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr2;
 
    data0 = *dataptr1;
    data1 = *dataptr1;
    data0 = *dataptr2;
    data1 = *dataptr2;
    yptr = (uint32 *)grabdata;
    mask = 0xff000000;
    for (i=0; i<height; i++) {
	dataptr = (i & 1) ? dataptr1 : dataptr2;

	for (j=0; j<15; j++) data0 = *dataptr;
	data1 = *dataptr;

	for (j=0; j<width; j+=12) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask) + ((data1 & mask) >> 8);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + ((data2 & mask) >> 16) + (data3 >> 24);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = (data0 & mask) + ((data1 & mask) >> 8);
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + ((data2 & mask) >> 16) + (data3 >> 24);

	    data2 = *dataptr;
	    data3 = data2;
	    y = (data0 & mask) + ((data1 & mask) >> 8);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + ((data2 & mask) >> 16) + (data3 >> 24);
	}
    }

    *datap = grabdata;
    *lenp = width*height;
    return 1;
}

static int VideoPix_GrabYUYVLargeNTSC(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, yy, uv, *yuyvptr;
    uint32 mask=0xff000000, uvflip=0x00800080;
    volatile uint32 *dataptr1=vfcdev->vfc_port1, *dataptr2=vfcdev->vfc_port2;
    volatile uint32 *dataptr;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr1;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr2;

    yuyvptr = (uint32 *)grabdata;
    for (i=0; i<height; i++) {
	dataptr = (i & 1) ? dataptr1 : dataptr2;

	for (j=0; j<16; j++) data0 = *dataptr;

	for (j=0; j<width; j+=40) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data3 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data3 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data3 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data1 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data3 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data1 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data3 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data3 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data3 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data3 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data1 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data3 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data1 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data3 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data1 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);
	}
    }

    *datap = grabdata;
    *lenp = width*height*2;
    return 1;
}

static int VideoPix_GrabYUYVLargePAL(uint8 **datap, int *lenp)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, yy, uv, *yuyvptr;
    uint32 mask=0xff000000, uvflip=0x00800080;
    volatile uint32 *dataptr1=vfcdev->vfc_port1, *dataptr2=vfcdev->vfc_port2;
    volatile uint32 *dataptr;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr1;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr2;

    yuyvptr = (uint32 *)grabdata;
    for (i=0; i<height; i++) {
	dataptr = (i & 1) ? dataptr1 : dataptr2;

	for (j=0; j<16; j++) data0 = *dataptr;

	for (j=0; j<width; j+=48) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data3 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data3 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data2 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data3 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data1 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data3 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data1 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data3 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data1 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data3 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data3 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data1 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data2 & mask) + ((data3 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy = (data0 & mask) + ((data0 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data1 & mask) + ((data2 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data3 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data1 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data3 & mask);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    yy += ((data0 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data1 & mask) + ((data2 & mask) >> 16);

	    uv = ( data0       & 0xc00000) + ((data0 >> 14) & 0xc0) +
		 ((data1 >> 2) & 0x300000) + ((data1 >> 16) & 0x30) +
		 ((data2 >> 4) & 0x0c0000) + ((data2 >> 18) & 0x0c);

	    *yuyvptr++ = yy + (uv ^ uvflip);

	    yy = (data3 & mask) + ((data3 & mask) >> 16);
	    *yuyvptr++ = yy + (uv ^ uvflip);
	}
    }

    *datap = grabdata;
    *lenp = width*height*2;
    return 1;
}

/*ARGSUSED*/
static char *VideoPixTracePort(ClientData clientData, Tcl_Interp *interp,
			       char *name1, char *name2, int flags)
{
    char *value;

    value = Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY);
    if (vfcdev != NULL) vfc_set_port(vfcdev, atoi(value));

    return NULL;
}

/*ARGSUSED*/
static char *VideoPixTraceFmt(ClientData clientData, Tcl_Interp *interp,
			      char *name1, char *name2, int flags)
{
    int format;
    char *value;

    value = Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY);
    if (vfcdev != NULL) vfc_set_format(vfcdev, atoi(value), &format);

    return NULL;
}

int VideoPix_Probe(void)
{
    struct stat vfcstat;

    Tcl_TraceVar(interp, "vpixPort", TCL_TRACE_WRITES, VideoPixTracePort, NULL);
    Tcl_TraceVar(interp, "vpixFmt", TCL_TRACE_WRITES, VideoPixTraceFmt, NULL);

    return (stat("/dev/vfc0", &vfcstat) < 0) ?
		0 : VID_SMALL|VID_MEDIUM|VID_LARGE|VID_GREYSCALE|VID_COLOR;
}

char *VideoPix_Attach(void)
{
    return ".grabControls.videopix";
}

void VideoPix_Detach(void)
{
}

/*ARGSUSED*/
grabproc_t *VideoPix_Start(int grabtype, int min_framespacing, int config,
			   reconfigproc_t *reconfig, void *enc_state)
{
    int tmpfd, port, fmt, format;
    grabproc_t *grab;

    xmit_size = (config & VID_SIZEMASK);

    switch (grabtype) {
    case VIDIMAGE_GREY:
	xmit_color = 0;
	break;
    case VIDIMAGE_YUYV:
	xmit_color = 1;
	break;
    default:
	return NULL;
    }

    /* Open the VideoPix device if it isn't already open */
    if (vfcdev == NULL) {
	/* This is a horrible hack to prevent the VFC library from printing
	   error messages that we'd rather not print. */
	tmpfd = dup(2);
	close(2);
	open("/dev/null", O_RDONLY);
	vfcdev = vfc_open("/dev/vfc0", VFC_LOCKDEV);
	dup2(tmpfd, 2);
	close(tmpfd);
	if (vfcdev == NULL) return NULL;
    }

    port = atoi(Tcl_GetVar(interp, "vpixPort", TCL_GLOBAL_ONLY));
    fmt =  atoi(Tcl_GetVar(interp, "vpixFmt",  TCL_GLOBAL_ONLY));
    vfc_set_port(vfcdev, port);
    vfc_set_format(vfcdev, fmt, &format);
    switch (format) {
    default:
    case NO_LOCK:
	fprintf(stderr, "Warning: no video signal found!\n");
	/* Fall through */
    case NTSC_COLOR:
    case NTSC_NOCOLOR:
	switch (xmit_size) {
	case VID_SMALL:
	    width = NTSC_WIDTH/2;
	    height = NTSC_HEIGHT/2;
	    grab = xmit_color? VideoPix_GrabYUYVSmallNTSC
			     : VideoPix_GrabGreySmallNTSC;
	    break;
	case VID_MEDIUM:
	    width = NTSC_WIDTH;
	    height = NTSC_HEIGHT;
	    grab = xmit_color? VideoPix_GrabYUYVMediumNTSC
			     : VideoPix_GrabGreyMediumNTSC;
	    break;
	case VID_LARGE:
	    width = NTSC_WIDTH*2;
	    height = NTSC_HEIGHT*2;
	    grab = xmit_color? VideoPix_GrabYUYVLargeNTSC
			     : VideoPix_GrabGreyLargeNTSC;
	    break;
	}
	break;
    case PAL_COLOR:
    case PAL_NOCOLOR:
	switch (xmit_size) {
	case VID_SMALL:
	    width = PAL_WIDTH/2;
	    height = PAL_HEIGHT/2;
	    grab = xmit_color? VideoPix_GrabYUYVSmallPAL
			     : VideoPix_GrabGreySmallPAL;
	    break;
	case VID_MEDIUM:
	    width = PAL_WIDTH;
	    height = PAL_HEIGHT;
	    grab = xmit_color? VideoPix_GrabYUYVMediumPAL
			     : VideoPix_GrabGreyMediumPAL;
	    break;
	case VID_LARGE:
	    width = PAL_WIDTH*2;
	    height = PAL_HEIGHT*2;
	    grab = xmit_color? VideoPix_GrabYUYVLargePAL
			     : VideoPix_GrabGreyLargePAL;
	    break;
	}
	break;
    }

    if (grabdata != NULL) free(grabdata);
    grabdata = (uint8 *)malloc(xmit_color? width*height*2 : width*height);

    (*reconfig)(enc_state, width, height);
    return grab;
}

void VideoPix_Stop(void)
{
    vfc_destroy(vfcdev);
    vfcdev = NULL;
}
#endif /* VIDEOPIX */
