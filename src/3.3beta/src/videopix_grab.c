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

static int xmit_size, xmit_color;
static VfcDev *vfcdev;

/*ARGSUSED*/
static int VideoPix_GrabSmallNTSCGrey(uint8 *y_data, int8 *uv_data)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, mask, y, *yptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;
 
    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr;
 
    data0 = *dataptr;
    data1 = *dataptr;
    yptr = (uint32 *)y_data;
    mask = 0xff000000;
    for (i=0; i<NTSC_HEIGHT/2; i++) {
	for (j=0; j<736; j++) data1 = *dataptr;

	for (j=0; j<NTSC_WIDTH/2; j+=20) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data1 & mask;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 8;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 16;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data0 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 8;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 16;

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
	    y = data0 & mask;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data0 & mask) >> 8;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 16;

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
	    y = data1 & mask;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data0 & mask) >> 8;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data0 & mask) >> 16;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data1 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data1 & mask;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 8;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data0 & mask) >> 16;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data0 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	}
    }

    return 1;
}

/*ARGSUSED*/
static int VideoPix_GrabSmallPALGrey(uint8 *y_data, int8 *uv_data)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, mask, y, *yptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;
 
    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr;
 
    data0 = *dataptr;
    data1 = *dataptr;
    yptr = (uint32 *)y_data;
    mask = 0xff000000;
    for (i=0; i<PAL_HEIGHT/2; i++) {
	for (j=0; j<736; j++) data0 = *dataptr;

	for (j=0; j<PAL_WIDTH/2; j+=12) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data0 & mask) >> 8;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data0 & mask) >> 16;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data3 >> 24);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y = data3 & mask;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data3 & mask) >> 8;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 16;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data2 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y = data2 & mask;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 8;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 16;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data1 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	}
    }

    return 1;
}

static int VideoPix_GrabSmallNTSCColor(uint8 *y_data, int8 *uv_data)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, y1, y2, uv1, uv2, *yptr, *uvptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr;

    yptr = (uint32 *)y_data;
    uvptr = (uint32 *)uv_data;
    for (i=0; i<NTSC_HEIGHT/2; i++) {
	for (j=0; j<736; j++) data0 = *dataptr;

	for (j=0; j<NTSC_WIDTH/2; j+=20) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data1 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 += (data1 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data1 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 += (data1 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data1 >> 16) & 0xff00);

	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;

	    y1 += (data3 >> 24);

            uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data0 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 += (data0 >> 24);

            uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data0 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 += (data0 >> 24);

            uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data1 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 += (data1 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data1 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 += (data1 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data1 >> 16) & 0xff00);

	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;

	    y2 += (data3 >> 24);

            uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data0 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 += (data0 >> 24);

            uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data0 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 += (data0 >> 24);

            uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;
	}
    }

    return 1;
}

static int VideoPix_GrabSmallPALColor(uint8 *y_data, int8 *uv_data)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, y1, y2, uv1, uv2, *yptr, *uvptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr;

    yptr = (uint32 *)y_data;
    uvptr = (uint32 *)uv_data;
    for (i=0; i<PAL_HEIGHT/2; i++) {
	for (j=0; j<736; j++) data0 = *dataptr;

	for (j=0; j<PAL_WIDTH/2; j+=12) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data0 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 += (data0 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data0 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 += (data0 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    y1 = ((data2 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 += (data1 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data1 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 += (data1 >> 24);

            uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data1 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 += (data0 >> 24);

            uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data0 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 += (data0 >> 24);

            uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;
	}
    }

    return 1;
}

/*ARGSUSED*/
static int VideoPix_GrabMediumNTSCGrey(uint8 *y_data, int8 *uv_data)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, mask, y, *yptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;
 
    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr;
 
    data0 = *dataptr;
    data1 = *dataptr;
    yptr = (uint32 *)y_data;
    mask = 0xff000000;
    for (i=0; i<NTSC_HEIGHT; i++) {
	for (j=0; j<16; j++) data1 = *dataptr;

	for (j=0; j<NTSC_WIDTH; j+=20) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data1 & mask;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data3 & mask) >> 8;
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 16;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data3 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data1 & mask;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 8;
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data0 & mask) >> 16;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data2 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 8;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 16;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data3 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data1 & mask;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data3 & mask) >> 8;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 16;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data2 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 8;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data0 & mask) >> 16;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data2 >> 24);
	}
    }

    return 1;
}

/*ARGSUSED*/
static int VideoPix_GrabMediumPALGrey(uint8 *y_data, int8 *uv_data)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, mask, y, *yptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;
 
    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr;
 
    data0 = *dataptr;
    data1 = *dataptr;
    yptr = (uint32 *)y_data;
    mask = 0xff000000;
    for (i=0; i<PAL_HEIGHT; i++) {
	for (j=0; j<16; j++) data0 = *dataptr;

	for (j=0; j<PAL_WIDTH; j+=24) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 8;
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data0 & mask) >> 16;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data2 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 8;
	    y += (data3 & mask) >> 16;
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data1 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y = data3 & mask;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 8;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data3 & mask) >> 16;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data1 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y = data2 & mask;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data0 & mask) >> 8;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 16;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    *yptr++ = y + (data0 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y = data2 & mask;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data0 & mask) >> 8;
	    y += (data1 & mask) >> 16;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data3 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data1 & mask;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data3 & mask) >> 8;

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y += (data1 & mask) >> 16;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    *yptr++ = y + (data3 >> 24);
	}
    }

    return 1;
}

static int VideoPix_GrabMediumNTSCColor(uint8 *y_data, int8 *uv_data)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, y1, y2, uv1, uv2, *yptr, *uvptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr;

    yptr = (uint32 *)y_data;
    uvptr = (uint32 *)uv_data;
    for (i=0; i<NTSC_HEIGHT; i++) {
	for (j=0; j<16; j++) data0 = *dataptr;

	for (j=0; j<NTSC_WIDTH; j+=20) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data1 >> 16) & 0xff00) + (data3 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data1 >> 16) & 0xff00) + (data3 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data1 >> 16) & 0xff00) + (data3 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data1 >> 16) & 0xff00) + (data3 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data1 >> 16) & 0xff00) + (data3 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;

	    y2 = ((data3 >> 16) & 0xff00) + (data2 >> 24);

            uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data3 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data0 >> 16) & 0xff00) + (data2 >> 24);

            uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data0 >> 16) & 0xff00) + (data2 >> 24);

            uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data0 >> 16) & 0xff00) + (data2 >> 24);

            uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data0 >> 16) & 0xff00) + (data2 >> 24);

            uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;
	}
    }

    return 1;
}

static int VideoPix_GrabMediumPALColor(uint8 *y_data, int8 *uv_data)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, y1, y2, uv1, uv2, *yptr, *uvptr;
    volatile uint32 *dataptr=vfcdev->vfc_port1;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr;

    yptr = (uint32 *)y_data;
    uvptr = (uint32 *)uv_data;
    for (i=0; i<PAL_HEIGHT; i++) {
	for (j=0; j<16; j++) data0 = *dataptr;

	for (j=0; j<PAL_WIDTH; j+=24) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data0 >> 16) & 0xff00) + (data2 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data0 >> 16) & 0xff00) + (data2 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data0 >> 16) & 0xff00) + (data2 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data0 >> 16) & 0xff00) + (data1 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    y1 = ((data2 >> 16) & 0xff00) + (data3 >> 24);

	    uv1 = uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data1 >> 16) & 0xff00) + (data3 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data1 >> 16) & 0xff00) + (data3 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data1 >> 16) & 0xff00) + (data3 >> 24);

            uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data1 >> 16) & 0xff00) + (data3 >> 24);

            uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data0 >> 16) & 0xff00) + (data2 >> 24);

            uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y1 = ((data0 >> 16) & 0xff00) + (data2 >> 24);

            uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y2 = ((data0 >> 16) & 0xff00) + (data2 >> 24);

            uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    *yptr++ = (y1 << 16) + y2;
	    *uvptr++ = (uv1 << 16) + uv2;
	}
    }

    return 1;
}

/*ARGSUSED*/
static int VideoPix_GrabLargeNTSCGrey(uint8 *y_data, int8 *uv_data)
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
    yptr = (uint32 *)y_data;
    mask = 0xff000000;
    for (i=0; i<NTSC_HEIGHT*2; i++) {
	dataptr = (i & 1) ? dataptr1 : dataptr2;

	for (j=0; j<15; j++) data0 = *dataptr;
	data1 = *dataptr;

	for (j=0; j<NTSC_WIDTH*2; j+=20) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;
	    y += (data1 & mask) >> 8;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 16;
	    *yptr++ = y + (data3 >> 24);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;
	    y += (data1 & mask) >> 8;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 16;
	    *yptr++ = y + (data3 >> 24);

	    data2 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;
	    y += (data1 & mask) >> 8;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 16;
	    *yptr++ = y + (data3 >> 24);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;
	    y += (data1 & mask) >> 8;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 16;
	    *yptr++ = y + (data3 >> 24);

	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;
	    y += (data1 & mask) >> 8;

	    data0 = *dataptr;
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 16;
	    *yptr++ = y + (data3 >> 24);
	}
    }

    return 1;
}

/*ARGSUSED*/
static int VideoPix_GrabLargePALGrey(uint8 *y_data, int8 *uv_data)
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
    yptr = (uint32 *)y_data;
    mask = 0xff000000;
    for (i=0; i<PAL_HEIGHT*2; i++) {
	dataptr = (i & 1) ? dataptr1 : dataptr2;

	for (j=0; j<15; j++) data0 = *dataptr;
	data1 = *dataptr;

	for (j=0; j<PAL_WIDTH*2; j+=12) {
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;
	    y += (data1 & mask) >> 8;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 16;
	    *yptr++ = y + (data3 >> 24);
 
	    data2 = *dataptr;
	    data3 = *dataptr;
	    y = data0 & mask;
	    y += (data1 & mask) >> 8;
 
	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 16;
	    *yptr++ = y + (data3 >> 24);

	    data2 = *dataptr;
	    data3 = data2;
	    y = data0 & mask;
	    y += (data1 & mask) >> 8;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    y += (data2 & mask) >> 16;
	    *yptr++ = y + (data3 >> 24);
	}
    }

    return 1;
}

static int VideoPix_GrabLargeNTSCColor(uint8 *y_data, int8 *uv_data)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, y, uv1, uv2, *yptr, *uvptr;
    volatile uint32 *dataptr1=vfcdev->vfc_port1, *dataptr2=vfcdev->vfc_port2;
    volatile uint32 *dataptr;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr1;
    for (i=0; i<VFC_OSKIP_NTSC-1; i++) data0 = *dataptr2;

    yptr = (uint32 *)y_data;
    uvptr = (uint32 *)uv_data;
    for (i=0; i<NTSC_HEIGHT*2; i++) {
	dataptr = (i & 1) ? dataptr1 : dataptr2;

	for (j=0; j<16; j++) data0 = *dataptr;

	for (j=0; j<NTSC_WIDTH*2; j+=40) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y = ((data0 >> 16) & 0xff00) + (data1 >> 24);
	    *yptr++ = (y << 16) + ((data2 >> 16) & 0xff00) + (data3 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv1 << 16) + uv1;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y = ((data0 >> 16) & 0xff00) + (data1 >> 24);
	    *yptr++ = (y << 16) + ((data2 >> 16) & 0xff00) + (data3 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv2 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y = ((data0 >> 16) & 0xff00) + (data1 >> 24);
	    y = (y << 16) + ((data3 >> 16) & 0xff00);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    *yptr++ = y + (data0 >> 24);
	    *uvptr++ = (uv1 << 16) + uv1;

	    y = ((data1 >> 16) & 0xff00) + (data2 >> 24);
	    y = (y << 16) + ((data3 >> 16) & 0xff00);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    *yptr++ = y + (data0 >> 24);
	    *uvptr++ = (uv2 << 16) + uv2;

	    y = ((data1 >> 16) & 0xff00) + (data2 >> 24);
	    y = (y << 16) + ((data3 >> 16) & 0xff00);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    *yptr++ = y + (data0 >> 24);
	    *uvptr++ = (uv1 << 16) + uv1;

	    y = ((data2 >> 16) & 0xff00) + (data3 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    *yptr++ = (y << 16) + ((data0 >> 16) & 0xff00) + (data1 >> 24);

	    y = ((data2 >> 16) & 0xff00) + (data3 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv2 << 16) + uv1;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    *yptr++ = (y << 16) + ((data0 >> 16) & 0xff00) + (data1 >> 24);

	    y = ((data2 >> 16) & 0xff00) + (data3 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv1 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    *yptr++ = (y << 16) + ((data1 >> 16) & 0xff00) + (data2 >> 24);

	    y = ((data3 >> 16) & 0xff00);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv2 << 16) + uv1;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y = y + (data0 >> 24);
	    *yptr++ = (y << 16) + ((data1 >> 16) & 0xff00) + (data2 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv1 << 16) + uv2;

	    y = ((data3 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y = y + (data0 >> 24);
	    *yptr++ = (y << 16) + ((data1 >> 16) & 0xff00) + (data2 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv2 << 16) + uv1;
	}
    }

    return 1;
}

static int VideoPix_GrabLargePALColor(uint8 *y_data, int8 *uv_data)
{
    static int grab=CAPTRCMD;
    int i, j;
    uint32 data0, data1, data2, data3, y, uv1, uv2, *yptr, *uvptr;
    volatile uint32 *dataptr1=vfcdev->vfc_port1, *dataptr2=vfcdev->vfc_port2;
    volatile uint32 *dataptr;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) == -1) return 0;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr1;
    for (i=0; i<VFC_ESKIP_PAL-1; i++) data0 = *dataptr2;

    yptr = (uint32 *)y_data;
    uvptr = (uint32 *)uv_data;
    for (i=0; i<PAL_HEIGHT*2; i++) {
	dataptr = (i & 1) ? dataptr1 : dataptr2;

	for (j=0; j<16; j++) data0 = *dataptr;

	for (j=0; j<PAL_WIDTH*2; j+=48) {
	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y = ((data0 >> 16) & 0xff00) + (data1 >> 24);
	    *yptr++ = (y << 16) + ((data2 >> 16) & 0xff00) + (data3 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv1 << 16) + uv1;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y = ((data0 >> 16) & 0xff00) + (data1 >> 24);
	    *yptr++ = (y << 16) + ((data2 >> 16) & 0xff00) + (data3 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv2 << 16) + uv2;

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y = ((data0 >> 16) & 0xff00) + (data1 >> 24);
	    *yptr++ = (y << 16) + ((data2 >> 16) & 0xff00) + (data2 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv1 << 16) + uv1;

	    y = ((data3 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y = y + (data0 >> 24);
	    *yptr++ = (y << 16) + ((data1 >> 16) & 0xff00) + (data2 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv1 << 16) + uv2;

	    y = ((data3 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y = y + (data0 >> 24);
	    *yptr++ = (y << 16) + ((data1 >> 16) & 0xff00) + (data2 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv2 << 16) + uv1;

	    y = ((data3 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    y = y + (data0 >> 24);
	    *yptr++ = (y << 16) + ((data1 >> 16) & 0xff00) + (data1 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv1 << 16) + uv2;

	    y = ((data2 >> 16) & 0xff00) + (data3 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    *yptr++ = (y << 16) + ((data0 >> 16) & 0xff00) + (data1 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv2 << 16) + uv1;

	    y = ((data2 >> 16) & 0xff00) + (data3 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    *yptr++ = (y << 16) + ((data0 >> 16) & 0xff00) + (data1 >> 24);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv1 << 16) + uv2;

	    y = ((data2 >> 16) & 0xff00) + (data3 >> 24);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    *yptr++ = (y << 16) + ((data0 >> 16) & 0xff00) + (data0 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv2 << 16) + uv1;

	    y = ((data1 >> 16) & 0xff00) + (data2 >> 24);
	    y = (y << 16) + ((data3 >> 16) & 0xff00);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    *yptr++ = y + (data0 >> 24);
	    *uvptr++ = (uv1 << 16) + uv1;

	    y = ((data1 >> 16) & 0xff00) + (data2 >> 24);
	    y = (y << 16) + ((data3 >> 16) & 0xff00);

	    uv2 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

	    *yptr++ = y + (data0 >> 24);
	    *uvptr++ = (uv2 << 16) + uv2;

	    y = ((data1 >> 16) & 0xff00) + (data2 >> 24);
	    *yptr++ = (y << 16) + ((data3 >> 16) & 0xff00) + (data3 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);
	    *uvptr++ = (uv1 << 16) + uv1;
	}
    }

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
grabproc_t *VideoPix_Start(int max_framerate, int config,
			   reconfigproc_t *reconfig)
{
    int tmpfd, port, fmt, format, width, height;
    grabproc_t *grab;

    xmit_size = (config & VID_SIZEMASK);
    xmit_color = (config & VID_COLOR);

    /* This is a horrible hack to prevent the VFC library from printing
       error messages that we'd rather not print. */
    tmpfd = dup(2);
    close(2);
    open("/dev/null", O_RDONLY);
    vfcdev = vfc_open("/dev/vfc0", VFC_LOCKDEV);
    dup2(tmpfd, 2);
    close(tmpfd);
    if (vfcdev == NULL) return NULL;

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
	    grab = xmit_color? VideoPix_GrabSmallNTSCColor
			     : VideoPix_GrabSmallNTSCGrey;
	    break;
	case VID_MEDIUM:
	    width = NTSC_WIDTH;
	    height = NTSC_HEIGHT;
	    grab = xmit_color? VideoPix_GrabMediumNTSCColor
			     : VideoPix_GrabMediumNTSCGrey;
	    break;
	case VID_LARGE:
	    width = NTSC_WIDTH*2;
	    height = NTSC_HEIGHT*2;
	    grab = xmit_color? VideoPix_GrabLargeNTSCColor
			     : VideoPix_GrabLargeNTSCGrey;
	    break;
	}
	break;
    case PAL_COLOR:
    case PAL_NOCOLOR:
	switch (xmit_size) {
	case VID_SMALL:
	    width = PAL_WIDTH/2;
	    height = PAL_HEIGHT/2;
	    grab = xmit_color? VideoPix_GrabSmallPALColor
			     : VideoPix_GrabSmallPALGrey;
	    break;
	case VID_MEDIUM:
	    width = PAL_WIDTH;
	    height = PAL_HEIGHT;
	    grab = xmit_color? VideoPix_GrabMediumPALColor
			     : VideoPix_GrabMediumPALGrey;
	    break;
	case VID_LARGE:
	    width = PAL_WIDTH*2;
	    height = PAL_HEIGHT*2;
	    grab = xmit_color? VideoPix_GrabLargePALColor
			     : VideoPix_GrabLargePALGrey;
	    break;
	}
	break;
    }

    (*reconfig)(xmit_color, width, height);
    return grab;
}

void VideoPix_Stop(void)
{
    vfc_destroy(vfcdev);
    vfcdev = NULL;
}
#endif /* VIDEOPIX */
