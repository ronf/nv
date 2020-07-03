/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video image definitions
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

#define MAX_WIDGETS	64

typedef struct {
    uint8	*y_data, *uv_data;
    int16	width, height;
    int16	brightness, contrast;
    uint8	greymap[256];
    uint32	flags;
    uint32	num_widgets;
    void	*widgetlist[MAX_WIDGETS];
} vidimage_t;

#define VIDIMAGE_ISCOLOR	0x1
#define VIDIMAGE_WANTCOLOR	0x2

extern vidimage_t *VidImage_Create(int color, int width, int height);
extern void VidImage_Destroy(vidimage_t *image);
extern void VidImage_Redraw(vidimage_t *image);
extern void VidImage_Clear(vidimage_t *image);
extern void VidImage_SetBrightness(vidimage_t *image, int brightness);
extern void VidImage_SetColor(vidimage_t *image, int iscolor, int wantcolor);
extern void VidImage_SetContrast(vidimage_t *image, int contrast);
extern void VidImage_SetSize(vidimage_t *image, int width, int height);
extern void VidImage_UpdateRect(vidimage_t *image, int x, int y, int width,
				int height);
