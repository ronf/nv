/*
	Netvideo version 2.7
	Written by Ron Frederick <frederick@parc.xerox.com>

	Video image & widget definitions
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

#define GREY_BITS		7
#define GREY_LEVELS		(1<<GREY_BITS)
#define GREY_MASK		(GREY_LEVELS-1)

#define MAX_WIDGETS		64

typedef struct {
    unsigned char	*data;
    short		width, height;
    short		brightness, contrast;
    unsigned char	greymap[GREY_LEVELS];
    unsigned long	num_widgets;
    void		*widgetlist[MAX_WIDGETS];
} vidimage_t;

extern void VidImage_Init();
extern vidimage_t *VidImage_Create(/* int width, int height */);
extern void VidImage_Destroy(/* vidimage_t *image */);
extern void VidImage_Redraw(/* vidimage_t *image */);
extern void VidImage_Reset(/* vidimage_t *image */);
extern void VidImage_SetBrightness(/* vidimage_t *image, int brightness */);
extern void VidImage_SetContrast(/* vidimage_t *image, int contrast */);
extern void VidImage_UpdateRect(/* vidimage_t *image, int x, int y,
				   int width, int height */);

extern void VidWidget_Init(/* Tk_Window mainWindow */);
extern void *VidWidget_Create(/* Tcl_Interp *interp, char *name,
				 int toplevel, vidimage_t *image */);
extern void VidWidget_Redraw(/* ClientData vidwidget */);
extern void VidWidget_UpdateRect(/* ClientData vidwidget, int x, int y,
				    int width, int height */);
