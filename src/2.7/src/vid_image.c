/*
        Netvideo version 2.7
        Written by Ron Frederick <frederick@parc.xerox.com>
 
        Video image handling routines
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
#include "video.h"

static void VidImage_ComputeGreymap(image)
    vidimage_t *image;
{
    register int j, newlevel;
    register double blacklev=(image->brightness-50)*1.27;
    register double span=image->contrast*2.54;
 
    for (j=0; j<128; j++) {
        newlevel = blacklev + j*span/127.;
        image->greymap[j] = (newlevel<0) ? 0 : (newlevel>127) ? 127 : newlevel;
    }
 
    VidImage_UpdateRect(image, 0, 0, image->width, image->height);
    VidImage_Redraw(image);
}

void VidImage_Init()
{
}

vidimage_t *VidImage_Create(width, height)
    int width, height;
{
    vidimage_t *image;

    image = (vidimage_t *)malloc(sizeof(vidimage_t));
    if (image != NULL) {
	image->data = (unsigned char *) malloc(width*height);
	memset(image->data, GREY_LEVELS/2, width*height);
	image->width = width;
	image->height = height;
	image->brightness = image->contrast = 0;
	image->num_widgets = 0;
    }

    return image;
}

void VidImage_Destroy(image)
    vidimage_t *image;
{
    if (image->num_widgets != 0)
	fprintf(stderr,
	    "Warning: vidimage freed without destroy all widgets.\n");

    free(image->data);
    free(image);
}

void VidImage_Redraw(image)
    vidimage_t *image;
{
    register int i;

    for (i=0; i<image->num_widgets; i++)
	VidWidget_Redraw(image->widgetlist[i]);
}

void VidImage_Reset(image)
    vidimage_t *image;
{
    memset(image->data, GREY_LEVELS/2, image->width*image->height);
    VidImage_UpdateRect(image, 0, 0, image->width, image->height);
    VidImage_Redraw(image);
}

void VidImage_SetBrightness(image, brightness)
    vidimage_t *image;
    int brightness;
{
    image->brightness = brightness;
    VidImage_ComputeGreymap(image);
}

void VidImage_SetContrast(image, contrast)
    vidimage_t *image;
    int contrast;
{
    image->contrast = contrast;
    VidImage_ComputeGreymap(image);
}

void VidImage_UpdateRect(image, x, y, width, height)
    vidimage_t *image;
    int x, y, width, height;
{
    register int i;

    for (i=0; i<image->num_widgets; i++)
	VidWidget_UpdateRect(image->widgetlist[i], x, y, width, height);
}
