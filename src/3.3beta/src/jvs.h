
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
 ************************************************************************/

/*
 * HISTORY
 * $Log: jvs.h,v $
 * Revision 1.3  1993/12/30  06:39:19  berc
 * *** empty log message ***
 *
 * $EndLog$
 */
#ifndef JVS_H
#define JVS_H 1

#define JVS_SOCKET 2510

#define JVS_ALLOCATE 1
#define JVS_COMPRESS 2
#define old_JVS_DECOMPRESS 3
#define JVS_SETCOMPRESS 4
#define JVS_SETDECOMPRESS 5
#define JVS_COLORMAP 6
#define JVS_PING 7
#define JVS_DEALLOCATE 8
#define JVS_DECOMPRESS 9
#define JVS_COMMAND 10

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

int JvsAllocateBuf(int fd, int direction, int type, int w, int y, int *shmid);
int JvsDeallocBuf(int fd, int shmid);

#endif
