
/************************************************************************
 *									*
 *			Copyright (c) 1993 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 *   Last modified on Mon Sep 12 13:21:23 PDT 1994 by berc              *
 *									*
 ************************************************************************/

/*
 * HISTORY
 * $Log: jvs.h,v $
 * Revision 1.12  1994/09/12  20:21:24  berc
 * Added definitions for GetDither() and SetDither(), plus
 * commented-out defintitions for setting the analog params.
 *
 * Revision 1.11  1994/07/05  19:39:45  berc
 * Rewrote buffer scheme to use a shared pool of buffers for all widgets
 * instead of statically allocating per-widget buffers.
 *
 * Revision 1.10  1994/06/28  04:56:55  berc
 * Added JvsColormapQuery
 *
 * Revision 1.9  1994/06/10  05:35:21  berc
 * Added definitions for JvsIdentity().
 *
 * Revision 1.8  1994/03/15  18:39:27  berc
 * Added linePadding return variable to SetDecompress().
 *
 * Revision 1.7  1994/03/08  08:17:53  berc
 * Added c++ declaration for Steve McCanne.
 *
 * Revision 1.6  1994/03/08  08:03:20  berc
 * Added JvsStartAnalogDecomp() interface
 *
 * Revision 1.5  1994/01/06  06:16:32  berc
 * Added function prototypes for the jvdriver interface calls.
 *
 * Removed old_StartDecompress.
 *
 * Revision 1.4  1994/01/01  06:18:25  berc
 * Removed AllocateSizedBuf by merging functionality into AllocateBuf.
 * Added fields for specifying buffer use.\
 *
 * Revision 1.3  1993/12/30  06:39:19  berc
 * *** empty log message ***
 *
 * $EndLog$
 */
#ifndef JVS_H
#define JVS_H 1

#define JVS_SOCKET 2510

#define JVS_NONE 0

#define JVS_ALLOCATE 1
#define JVS_COMPRESS 2
#define JVS_SETCOMPRESS 4
#define JVS_SETDECOMPRESS 5
#define JVS_COLORMAP 6
#define JVS_PING 7
#define JVS_DEALLOCATE 8
#define JVS_DECOMPRESS 9
#define JVS_COMMAND 10
#define JVS_DECOMPRESS_ANALOG 11
#define JVS_IDENTITY 12
#define JVS_COLORMAP_QUERY 13
#define JVS_ALLOCATE2 14
#define JVS_GETDITHER 15
#define JVS_SETDITHER 16
#define JVS_GET_ANALOGPARAMS 17
#define JVS_SET_ANALOGPARAMS 18

/* Buffers types */
#define JVS_JPEG 1
#define	JVS_DITHERED 2
#define JVS_YUV 3

/* Buffer directions */
#define JVS_INPUT 1
#define JVS_OUTPUT 2

typedef struct _JvsPingReq {
  int	requestCode;
} JvsPingReq;

typedef struct _JvsPingRep {
  int	requestCode;
} JvsPingRep;

typedef struct _JvsAllocateReq {
  int	requestCode;
  int	direction;
  int	type;
  int	width;
  int	height;
} JvsAllocateReq;

typedef struct _JvsAllocateRep {
  int	requestCode;
  int	shmid;
} JvsAllocateRep;

typedef struct _JvsDeallocateReq {
  int	requestCode;
  int	shmid;
} JvsDeallocateReq;

typedef struct _JvsDeallocateRep {
  int	requestCode;
  int   replyCode;
} JvsDeallocateRep;

typedef struct _JvsCompressReq {
  int	requestCode;
  int	shmid;
} JvsCompressReq;

typedef struct _JvsCompressRep {
  int	requestCode;
  int	shmid;
  int	length;
} JvsCompressRep;

typedef struct _JvsDecompressReq {
  int	requestCode;
  int	cshmid, dshmid;
  int   length; /* length of compressed image */
} JvsDecompressReq;

typedef struct _oldJvsDecompressReq {
  int	requestCode;
  int	cshmid, dshmid;
} oldJvsDecompressReq;

typedef struct _JvsDecompressRep {
  int	requestCode;
  int	cshmid, dshmid;
  int	nColors;
} JvsDecompressRep;

typedef struct _JvsSetCompressReq {
  int	requestCode;
  int	qfactor;
  int	xdec, ydec;
  int	frameskip;
  int	type; /* JVS_JPEG or JVS_YUV */
} JvsSetCompressReq;

typedef struct _JvsSetCompressRep {
  int	requestCode;
  int   width, height; /* result dimensions of compressed image */
} JvsSetCompressRep;

typedef struct _JvsSetDecompressReq {
  int	requestCode;
  int	qfactor;
  int   inX, inY, outX, outY;
  int	brightness, contrast, saturation;
} JvsSetDecompressReq;

typedef struct _JvsSetDecompressRep {
  int	requestCode;
  int   actualOutX, actualOutY, linePadding;
} JvsSetDecompressRep;

typedef struct _JvsColormapReq {
  int	requestCode;
  int	monochrome;
  int	nColors;
  int	id;	/* Colormap id */
  char	serverName[256]; /* X server (DISPLAY variable) to talk to */
} JvsColormapReq;

typedef struct _JvsColormapRep {
  int	requestCode;
  int	nColors;
} JvsColormapRep;

typedef struct _JvsIdentityReq {
  int	requestCode;
} JvsIdentityReq;

#define JVS_IDENTITY_STRING 32
typedef struct _JvsIdentityRep {
  int	requestCode;
  char  identity[JVS_IDENTITY_STRING];
} JvsIdentityRep;

typedef struct _JvsCommandReq {
  int	requestCode;
  int   command;
  int	dataLength;
  /* put data (if any) here */
} JvsCommandReq;

typedef struct _JvsCommandRep {
  int	requestCode;
  int	replyCode;
  int	dataLength;
} JvsCommandRep;

typedef struct _JvsColormapQueryReq {
  int	requestCode;
  int	id;	/* Colormap id */
  char	serverName[256]; /* X server (DISPLAY variable) to talk to */
} JvsColormapQueryReq;

typedef struct _JvsColormapQueryRep {
  int	requestCode;
  int	nColors;
  int	indices[256];
} JvsColormapQueryRep;

typedef struct _JvsGetDitherReq {
  int	requestCode;
} JvsGetDitherReq;

typedef struct _JvsGetDitherRep {
  int	requestCode;
  int brightness;
  int contrast;
  int saturation;
} JvsGetDitherRep;

typedef struct _JvsSetDitherReq {
  int	requestCode;
  int brightness;
  int contrast;
  int saturation;
} JvsSetDitherReq;

typedef struct _JvsSetDitherRep {
  int	requestCode;
} JvsSetDitherRep;

#ifdef undef

typedef enum {
  JvsNTSC,
  JvsPAL,
} JvsAnalogStandard;

typedef enum {
  Input,
  Output,
} JvsAnalogDirection;

typedef enum {
  Standard,
  SVideo,
} JvsAnalogEncoding;

typedef struct _JvsGetAnalogParamsReq {
  int	requestCode;
  JvsAnalogDirection	direction;
} JvsGetAnalogParamsReq;

typedef struct _JvsGetAnalogParamsRep {
  int			requestCode;
  JvsAnalogStandard	standard;
  JvsAnalogEncoding	encoding;
  int			port;
} JvsSetAnalogParamsRep;

typedef struct _JvsSetAnalogParamsReq {
  int	requestCode;
  JvsAnalogDirection	direction;
  JvsAnalogStandard	standard;
  JvsAnalogEncoding	encoding;
  int			port;
} JvsSetAnalogParamsReq;

typedef struct _JvsSetAnalogParamsRep {
  int	requestCode;
} JvsSetAnalogParamsRep;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Open a stream to/from jvdriver. */
int JvsOpen(char *hostname, int port);

/* Allocate a shared memory buffer that is locked into physical memory
 * so that it can be DMAed to/from.  The direction is JVS_INPUT if it's
 * going to be the recipient of live capture, or JVS_OUTPUT if it's either
 * the source or destination of a image being decompressed.  The type is
 * JVS_JPEG for compressed data, JVS_DITHERED for 8-bit dithered output
 * rasters, and JVS_YUV for true-color capture.  
 *
 * A single stream may request input frames or decomression services, but 
 * should not ask for both.
 *
 * A JVS_JPEG buffer must be dedicated to input or output use.
 * 
 * A single stream should not ask for both JVS_JPEG and JVS_YUV input
 * frames.
 *
 * To allocate a buffer to be compressed into:
 *     status = JvsAllocateBuf(jvfd, JVS_INPUT, JVS_JPEG, 0, 0, &shmid);
 *
 * To allocate a buffer to be decompressed from:
 *     status = JvsAllocateBuf(jvfd, JVS_OUTPUT, JVS_JPEG, 0, 0, &shmid);
 *
 * To allocate a buffer to be decompressed to:
 *     status = JvsAllocateBuf(jvfd, JVS_OUTPUT, JVS_DITHERED, w, h, &shmid);
 */
int JvsAllocateBuf(int fd, int direction, int type, int w, int y, int *shmid);
int JvsAllocateBuf2(int fd, int direction, int type, int w, int y, int *shmid);
int JvsDeallocBuf(int fd, int shmid);

/* Compresses a live video image into shmid. */
int JvsStartComp(int fd, int shmid);

/* Wait for a compression request to complete */
int JvsWaitComp(int fd, int *shmid, int *len);

/* Decompresses cshmid into dshmid.  Will zero the compressed data block
 * from the end of the compressed data to the next 512-byte boundry.
 */
int JvsStartDecomp(int fd, int cshmid, int dshmid, int length);

/* Decompresses cshmid to analog port.  Will zero the compressed data block
 * from the end of the compressed data to the next 512-byte boundry.
 */
int JvsStartAnalogDecomp(int fd, int cshmid, int length);

/* Wait for a decompression request to complete. */
int JvsWaitDecomp(int fd, int *cshmid, int *dshmid);

/* Set a stream's compression parameters */
int JvsSetComp(int fd, 
	       int qfactor, 
	       int xdec, 
	       int ydec, 
	       int frameskip, 
	       int *width, 
	       int *height);

int JvsSetCompRaw(int fd, JvsSetCompressReq *req, JvsSetCompressRep *rep);

/* Set a stream's decompression parameters */
int JvsSetDecomp(int fd, 
		 int qfactor,
		 int inX, 
		 int inY, 
		 int *outX,
		 int *outY, 
		 int brightness, 
		 int contrast, 
		 int saturation,
		 int *linePadding);

int JvsSetDecompRaw(int fd,
		    JvsSetDecompressReq *req,
		    JvsSetDecompressRep *rep);

/* Set the number of colors to dither to for all J-Video users of the
 * (Xserver "name", colormap id "id") pair */
int JvsColormap(int fd, char *name, int id, int *nColors, int monochrome);

/* Returns the number of colors and indices currently being dithered to
 * for all users of the (Xserver "name", colormap id "id") pair. */
int JvsColormapQuery(int fd, char *name, int id, int *nColors, int indices[]);

/* Copies the indentity of the server to *identity, which must be a 
 * preallocated string of at least JVS_IDENTITY_LENGTH bytes.
 * The two current identity strings are "J-Video" and "J300".
 */
int JvsIdentity(int fd, char *identity);

/* Get and set the dither color parameters */
int JvsGetDither(int fd, int *brightness, int *contrast, int *saturation);
int JvsSetDither(int fd, int brightness, int contrast, int saturation);

#ifdef undef
/* Get and set analog input/output  parameters */
int JvsGetAnalogParams(int fd, 
		       JvsAnalogDirection	direction,
		       JvsAnalogStandard	*standard,
		       JvsAnalogEncoding	*encoding,
		       int			*port);
int JvsSetAnalogParams(int fd, 
		       JvsAnalogDirection	direction,
		       JvsAnalogStandard	standard,
		       JvsAnalogEncoding	encoding,
		       int			port);
#endif

#ifdef __cplusplus
}
#endif
#endif
