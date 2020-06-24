/*
	Netvideo version 3.2
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

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <vfc_lib.h>
#include "vidgrab.h"

#ifdef __GNUC__
#undef VFCSCTRL
#define VFCSCTRL      _IOW('j', 1, int) /* set ctrl. attr. */
#endif /*__GNUC__*/

static VfcDev *vfcdev;

int (*GrabImage)(u_char *y_data, signed char *uv_data);

static int GrabImage_NTSC_Grey(u_char *y_data)
{
    static int grab=CAPTRCMD;
    register int i, j;
    register unsigned long data0, data1, data2, data3, mask, y, *yptr;
    register volatile unsigned long *dataptr=vfcdev->vfc_port1;
 
    ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab);
    for (i=0; i<VFC_OSKIP_NTSC+3; i++) data0 = *dataptr;
 
    data0 = *dataptr;
    data1 = *dataptr;
    yptr = (u_long *)y_data;
    mask = 0xff000000;
    for (i=0; i<NTSC_HEIGHT; i++) {
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
	for (j=0; j<16; j++) data1 = *dataptr;
    }

    return 1;
}

static int GrabImage_PAL_Grey(u_char *y_data)
{
    static int grab=CAPTRCMD;
    register int i, j;
    register unsigned long data0, data1, data2, data3, mask, y, *yptr;
    register volatile unsigned long *dataptr=vfcdev->vfc_port1;
 
    ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab);
    for (i=0; i<VFC_ESKIP_PAL+3; i++) data0 = *dataptr;
 
    data0 = *dataptr;
    data1 = *dataptr;
    yptr = (u_long *)y_data;
    mask = 0xff000000;
    for (i=0; i<PAL_HEIGHT; i++) {
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

	for (j=0; j<16; j++) data0 = *dataptr;

    }

    return 1;
}

static int GrabImage_NTSC_Color(u_char *y_data, signed char *uv_data)
{
    static int grab=CAPTRCMD;
    register int i, j;
    register unsigned long data0, data1, data2, data3;
    register unsigned long y1, y2, uv1, uv2, *yptr, *uvptr;
    register volatile unsigned long *dataptr=vfcdev->vfc_port1;

    ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab);
    for (i=0; i<VFC_OSKIP_NTSC+3; i++) data0 = *dataptr;

    yptr = (u_long *)y_data;
    uvptr = (u_long *)uv_data;
    for (i=0; i<NTSC_HEIGHT; i++) {
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

	    y1 = ((data1 >> 16) & 0xff00) + (data3 >> 24);

	    uv1 = ((data0 >>  8) & 0xc000) + ((data0 >> 14) & 0xc0) +
		  ((data1 >> 10) & 0x3000) + ((data1 >> 16) & 0x30) +
		  ((data2 >> 12) & 0x0c00) + ((data2 >> 18) & 0x0c);

	    data0 = *dataptr;
	    data1 = *dataptr;
	    data2 = *dataptr;
	    data3 = *dataptr;

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

	for (j=0; j<16; j++) data0 = *dataptr;
    }

    return 1;
}

static int GrabImage_PAL_Color(u_char *y_data, signed char *uv_data)
{
    static int grab=CAPTRCMD;
    register int i, j;
    register unsigned long data0, data1, data2, data3;
    register unsigned long y1, y2, uv1, uv2, *yptr, *uvptr;
    register volatile unsigned long *dataptr=vfcdev->vfc_port1;

    ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab);
    for (i=0; i<VFC_ESKIP_PAL+3; i++) data0 = *dataptr;

    yptr = (u_long *)y_data;
    uvptr = (u_long *)uv_data;
    for (i=0; i<PAL_HEIGHT; i++) {
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

	for (j=0; j<16; j++) data0 = *dataptr;
    }

    return 1;
}

static int GrabImage_NTSC(u_char *y_data, signed char *uv_data)
{
    if (uv_data)
	return GrabImage_NTSC_Color(y_data, uv_data);
     else
	return GrabImage_NTSC_Grey(y_data);
}

static int GrabImage_PAL(u_char *y_data, signed char *uv_data)
{
    if (uv_data)
	return GrabImage_PAL_Color(y_data, uv_data);
     else
	return GrabImage_PAL_Grey(y_data);
}

int GrabImage_Init(int framerate, int *widthp, int *heightp)
{
    int tmpfd, format;

    /* This is a horrible hack to prevent the VFC library from printing
       error messages that we'd rather not print. */
    tmpfd = dup(2);
    close(2);
    open("/dev/null", O_RDONLY);
    vfcdev = vfc_open("/dev/vfc0", VFC_LOCKDEV);
    dup2(tmpfd, 2);
    close(tmpfd);
    if (vfcdev == NULL) {
	*widthp = *heightp = 0;
	return 0;
    }

    vfc_set_port(vfcdev, VFC_PORT1);
    vfc_set_format(vfcdev, VFC_AUTO, &format);
    switch (format) {
    default:
    case NO_LOCK:
	fprintf(stderr, "Warning: no video signal found!\n");
	/* Fall through */
    case NTSC_COLOR:
    case NTSC_NOCOLOR:
	*widthp = NTSC_WIDTH;
	*heightp = NTSC_HEIGHT;
	GrabImage = GrabImage_NTSC;
	break;
    case PAL_COLOR:
    case PAL_NOCOLOR:
	*widthp = PAL_WIDTH;
	*heightp = PAL_HEIGHT;
	GrabImage = GrabImage_PAL;
	break;
    }

    return 1;
}

void GrabImage_Cleanup(void)
{
    vfc_destroy(vfcdev);
}
