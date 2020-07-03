/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>
 
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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <tk.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_util.h"
#include "vid_widget.h"

extern int color_ok;

static void VidImage_ComputeGreymap(vidimage_t *image)
{
    int j, newlevel, limit;
    int color = (image->flags & VIDIMAGE_ISCOLOR) &&
		(image->flags & VIDIMAGE_WANTCOLOR);
    double blacklev, step;

    if (color) {
	blacklev = (image->brightness-50)*0.63;
	step = image->contrast/200.;
	limit = 63;
    } else {
	blacklev = (image->brightness-50)*2.55;
	step = image->contrast/50.;
	limit = 255;
    }
 
    for (j=0; j<256; j++) {
        newlevel = blacklev + j*step + 0.5;
	if (newlevel < 0)
	    newlevel = 0;
	else if (newlevel > limit)
	    newlevel = limit;
        image->greymap[j] = newlevel;
    }
}

static void VidImage_AllocImage(vidimage_t *image, int width, int height)
{
    image->width = width;
    image->height = height;
    image->y_data = (uint8 *) malloc(width*height);
    memset(image->y_data, 128, width*height);
    if (image->flags & VIDIMAGE_ISCOLOR) {
	image->uv_data = (uint8 *) malloc(width*height);
	memset(image->uv_data, 128, width*height);
    } else {
	image->uv_data = 0;
    }
}

vidimage_t *VidImage_Create(int color, int width, int height)
{
    vidimage_t *image;

    image = (vidimage_t *)malloc(sizeof(vidimage_t));
    if (image != NULL) {
	image->flags = (color ? VIDIMAGE_ISCOLOR : 0) | VIDIMAGE_WANTCOLOR;
	VidImage_AllocImage(image, width, height);
	image->brightness = image->contrast = 0;
	image->num_widgets = 0;
    }

    return image;
}

void VidImage_Destroy(vidimage_t *image)
{
    if (image->num_widgets != 0)
	fprintf(stderr,
	    "Warning: vidimage freed without destroy all widgets.\n");

    free(image->y_data);
    if (image->uv_data) free(image->uv_data);
    free(image);
}

void VidImage_Redraw(vidimage_t *image)
{
    int i;

    for (i=0; i<image->num_widgets; i++)
	VidWidget_Redraw(image->widgetlist[i]);
}

void VidImage_Clear(vidimage_t *image)
{
    int size=image->width*image->height;

    memset(image->y_data, 128, size);
    if (image->uv_data) memset(image->uv_data, 128, size);
}

void VidImage_SetBrightness(vidimage_t *image, int brightness)
{
    image->brightness = brightness;
    VidImage_ComputeGreymap(image);
    VidImage_UpdateRect(image, 0, 0, image->width, image->height);
    VidImage_Redraw(image);
}

void VidImage_SetColor(vidimage_t *image, int iscolor, int wantcolor)
{
    int i;
    int width=image->width, height=image->height;

    image->flags &= ~(VIDIMAGE_ISCOLOR | VIDIMAGE_WANTCOLOR);
    image->flags |= iscolor? VIDIMAGE_ISCOLOR : 0;
    image->flags |= (wantcolor && color_ok)? VIDIMAGE_WANTCOLOR : 0;

    if (iscolor) {
	if (image->uv_data == 0) {
	    image->uv_data = (uint8 *) malloc(width*height);
	    memset(image->uv_data, 128, width*height);
	}
    } else {
	if (image->uv_data) {
	    free(image->uv_data);
	    image->uv_data = 0;
	}
    }

    VidImage_ComputeGreymap(image);
    for (i=0; i<image->num_widgets; i++)
	VidWidget_SetColor(image->widgetlist[i]);
}

void VidImage_SetContrast(vidimage_t *image, int contrast)
{
    image->contrast = contrast;
    VidImage_ComputeGreymap(image);
    VidImage_UpdateRect(image, 0, 0, image->width, image->height);
    VidImage_Redraw(image);
}

void VidImage_SetSize(vidimage_t *image, int width, int height)
{
    int i;

    free(image->y_data);
    if (image->uv_data) free(image->uv_data);
    VidImage_AllocImage(image, width, height);

    for (i=0; i<image->num_widgets; i++)
	VidWidget_Resize(image->widgetlist[i]);
}

void VidImage_UpdateRect(vidimage_t *image, int x, int y, int width, int height)
{
    int i;

    for (i=0; i<image->num_widgets; i++)
	VidWidget_UpdateRect(image->widgetlist[i], x, y, width, height);
}
