/*
	Netvideo version 2.7
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

#define LEFT_SKIP	16

static VfcDev *vfcdev;

int (*GrabImage)();

static int GrabImage_NTSC(image)
    unsigned char *image;
{
    static int grab=CAPTRCMD;
    register int x, y;
    register int group;
    register unsigned long tmp, *vfcdata = vfcdev->vfc_port1;
    register unsigned char *ip=image;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) < 0) return 0;

    for (x=0; x<VFC_OSKIP_NTSC; x++) tmp = *vfcdata;

    for (y=0; y<NTSC_HEIGHT; y++) {
	for (x=0; x<LEFT_SKIP; x++) tmp = *vfcdata;

	for (x=0; x<NTSC_WIDTH; x+=5) {
	    *ip++ = VFC_PREVIEW_Y0(*vfcdata);
	    tmp = *vfcdata;
	    *ip++ = VFC_PREVIEW_Y0(*vfcdata);
	    tmp = *vfcdata;
	    *ip++ = VFC_PREVIEW_Y0(*vfcdata);
	    tmp = *vfcdata;
	    *ip++ = VFC_PREVIEW_Y0(*vfcdata);
	    tmp = *vfcdata;
	    *ip++ = VFC_PREVIEW_Y0(*vfcdata);
	    tmp = *vfcdata;
	    tmp = *vfcdata;
	}
    }

    return 1;
}

static int GrabImage_PAL(image)
    unsigned char *image;
{
    static int grab=CAPTRCMD;
    register int x, y;
    register int group;
    register unsigned long tmp, *vfcdata = vfcdev->vfc_port1;
    register unsigned char *ip=image;

    if (ioctl(vfcdev->vfc_fd, VFCSCTRL, &grab) < 0) return 0;

    for (x=0; x<VFC_ESKIP_PAL; x++) tmp = *vfcdata;

    for (y=0; y<PAL_HEIGHT; y++) {
	for (x=0; x<LEFT_SKIP; x++) tmp = *vfcdata;

	for (x=0; x<PAL_WIDTH; x+=6) {
	    *ip++ = VFC_PREVIEW_Y0(*vfcdata);
	    tmp = *vfcdata;
	    *ip++ = VFC_PREVIEW_Y0(*vfcdata);
	    tmp = *vfcdata;
	    *ip++ = VFC_PREVIEW_Y0(*vfcdata);
	    tmp = *vfcdata;
	    *ip++ = VFC_PREVIEW_Y0(*vfcdata);
	    tmp = *vfcdata;
	    *ip++ = VFC_PREVIEW_Y0(*vfcdata);
	    tmp = *vfcdata;
	    *ip++ = VFC_PREVIEW_Y0(*vfcdata);
	}
    }

    return 1;
}

int GrabImage_Init(framerate, widthp, heightp)
    int framerate;
    int *widthp, *heightp;
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

void GrabImage_Cleanup()
{
    vfc_destroy(vfcdev);
}
