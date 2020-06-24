/****************************************************************************/
/*                                                                          */
/*	cpv.h -- Return codes from CPV_Decode subroutine to decode          */
/*	Concept/Bolter/ViewPoint Compressed Packet Video (CPV) (TM)         */
/*                                                                          */
/****************************************************************************/
/*									    */
/*	Copyright (c) 1994 by the University of Southern California.	    */
/*	All rights reserved.						    */
/*									    */
/*	COMMERCIAL USE OF THIS CODE IS STRICTLY PROHIBITED WITHOUT THE	    */
/*	SPECIFIC PRIOR WRITTEN AUTHORIZATION OF VIEWPOINT SYSTEMS, INC.	    */
/*	de-CPV-ware(TM), CPV(TM), and Compressed Packet Video(TM) are	    */
/*	trademarks of VIEWPOINT SYSTEMS, INC., 2247 Wisconsin Street	    */
/*	#110, DALLAS, TEXAS 75229					    */
/*									    */
/*	Permission to use, copy, and distribute this software and its	    */
/*	documentation in binary form for non-commercial purposes and	    */
/*	without fee is hereby granted, provided that the above copyright    */
/*	notice appears in all copies, that both the copyright notice and    */
/*	this permission notice appear in supporting documentation, and	    */
/*	that any documentation, advertising materials, and other	    */
/*	materials related to such distribution and use acknowledge that	    */
/*	the software was developed by the University of Southern	    */
/*	California, Information Sciences Institute.  The name of the	    */
/*	University may not be used to endorse or promote products derived   */
/*	from this software without specific prior written permission.	    */
/*									    */
/*	THE UNIVERSITY OF SOUTHERN CALIFORNIA makes no representations	    */
/*	about the suitability of this software for any purpose.  THIS	    */
/*	SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED	    */
/*	WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES   */
/*	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.	    */
/*									    */
/*	Other copyrights might apply to parts of this software and are	    */
/*	so noted when applicable.					    */
/*									    */
/****************************************************************************/
/*									    */
/*	This software decompression function is also known as		    */
/*	"de-CPV-ware", Version 1.0.			  		    */
/*									    */
/****************************************************************************/
/*									    */
/*	Author:		  Stephen Casner, casner@isi.edu		    */
/*			  USC Information Sciences Institute		    */
/*			  4676 Admiralty Way				    */
/*			  Marina del Rey, CA 90292-6695			    */
/*									    */
/****************************************************************************/
/*									    */
/*	Programming Interface						    */
/*									    */
/*	int CPV_Decode(image, pktptr, pktlen)				    */
/*	    struct vidimage {	   Output image arrays CPV_WIDTHxCPV_HEIGHT */
/*		unsigned char *y_data;	  Pixel luminance image		    */
/*		char *uv_data;		  Pixel chrominance image	    */
/*		short width, height;	  Size of pixel image		    */
/*					  Other stuff here that we ignore   */
/*	    } *image;		   Pointer to output image array struct	    */
/*	    unsigned char *pktptr; Pointer to start of video data in packet */
/*	    int pktlen;            Length of video data as received	    */
/*									    */
/*	The caller is responsible for allocating the image output	    */
/*	arrays y_data and uv_data; these arrays continuously maintain	    */
/*	the output image as it is updated for each call.  For each	    */
/*	call, one packet of compressed video data is decompressed and	    */
/*	written as Y and UV pixels at the addressed locations in the	    */
/*	y_data and uv_data output arrays.  The image is in "4:2:2"	    */
/*	format; that is, for each horizontal pair of Y pixels there is	    */
/*	one U and one V pixel that together form the chrominance shared	    */
/*	by both Y pixels.  The conversion from the encoding of video	    */
/*	data in the packet to Y and UV pixels is accomplished with	    */
/*	lookup tables indexed by an RGB value of 5 bits each.  These	    */
/*	tables occupy a total of 98304 bytes.  These lookup tables may	    */
/*	be supplied by the caller in the two arrays:			    */
/*									    */
/*	extern unsigned char rgb2y[32768];     RGB to Y or B&W table	    */
/*	extern unsigned short rgb2uv[32768];   RGB to U & V table	    */
/*									    */
/*	Or, if this module is compiled with FIRST_ENTRY defined,	    */
/*	static arrays will be allocated and on the first call tables	    */
/*	values will be calculated.					    */
/*									    */
/*	For each rectangular area of the image which has been updated,	    */
/*	a call is made to the following routine to allow the caller to	    */
/*	further process and display those parts of the image:		    */
/*									    */
/*	extern void VidImage_UpdateRect(image, x, y, width, height);	    */
/*	    struct vidimage *image; Pointer to output image array struct    */
/*	    int x,y;                Offsets of area within image (left,top) */
/*	    int width,height;       Size of update area in pixels	    */
/*									    */
/*	The return value from CPV_decode is an integer success/failure	    */
/*	code.								    */
/*									    */
/****************************************************************************/

#define CPV_SUCCESS	0	/* Packet successfully decoded */
#define CPV_EXTRAPIXEL	1	/* Extra pixel past end of pixmap */
#define CPV_BADADDRESS	2	/* Bad address in video data */
#define CPV_UNTERM	3	/* Packet not terminated */
#define CPV_BADHEADER	4	/* Bad video header format */
#define CPV_BADLENGTH	5	/* Unreasonable packet length */
#define CPV_NONMOTION	6	/* Non-motion video packet found */

#define CPV_WIDTH	256
#define CPV_HEIGHT	200

extern int CPV_Decode(vidimage_t *image, unsigned char *data, int len);
