
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

/* This is the client interface to the J-Video driver. */
  
#ifdef J300
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include "jvs.h"

/* Communication between the clients and J-Video driver is done with 
 * Unix domain sockets for control information and shared memory segments
 * for data.
 */
/*#define TCP 1*/
  
#ifdef TCP
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif TCP

#define OK 0
#define ERROR -1
#define RETURN_ON_ERROR(A) if (A < 0) return(A)

#ifdef TCP
static unsigned atoaddr(s)
     char *s;
{
  int a,b,c,d;
  
  if (sscanf(s, "%d.%d.%d.%d", &a, &b, &c, &d) != 4) return 0;
  return (a << 24) + (b << 16) + (c << 8) + d;
}

int JvsOpen(char *hostname, int port);
{
  struct hostent      *h;
  struct sockaddr_in  sin;
  unsigned hexaddr;
  int fd;
  
  /* get address of host */
  bzero(&sin, sizeof(sin));
  if ((h = gethostbyname("localhost")) == NULL) {
    if ((hexaddr = htonl(atoaddr(hostname))) == 0) {
      perror("JvsOpen: Host lookup failed");
      return -1;
    }
    bcopy(&hexaddr, &sin.sin_addr, 4);
    sin.sin_family = AF_INET;
  } else {
    bcopy(h->h_addr, &sin.sin_addr, h->h_length);
    sin.sin_family = h->h_addrtype;
  }
  
  /* open socket */
  sin.sin_port = htons(port);
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("JvsOpen: failed to get socket");
    return -1;
  }
  
  if (connect(fd, &sin, sizeof(sin)) < 0) {
#ifdef notdef
    perror("JvsOpen: failed to connect");
#endif
    return -1;
  }
  
  return fd;
}
#else TCP
/* Creates a connection to the jvserver. */
int JvsOpen(char *hostname, int port)
{
  struct sockaddr_un unaddr;		/* UNIX socket data block */
  struct sockaddr *addr;		/* generic socket pointer */
  int addrlen;			/* length of addr */
  int fd;
  
  unaddr.sun_family = AF_UNIX;
  sprintf (unaddr.sun_path, "/tmp/jvideo/jvideo.%d", port);
  
  addr = (struct sockaddr *) &unaddr;
  addrlen = strlen(unaddr.sun_path) + sizeof(unaddr.sun_family);
  
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("JvsOpen");
    return -1;
  }
  
  if (connect (fd, addr, addrlen) < 0) {
    fprintf(stderr, "%s", unaddr.sun_path);
    perror("JvsOpen: failed to connect");
    return -1;
  }
  
  signal(SIGPIPE, SIG_IGN);
  
  return fd;
}
#endif TCP

/* Allocates a shared-memory buffer in the server and returns
   its shmid. */
int JvsAllocateBuf(int fd, int direction, int type, int w, int h, int *shmid)
{
  JvsAllocateReq req;
  JvsAllocateRep rep;
  
  int status;
  
  req.requestCode = JVS_ALLOCATE;
  req.direction = direction;
  req.type = type;
  req.width = w;
  req.height = h;
  
  status = JvsSend(fd, &req, sizeof(req));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, &rep, sizeof(rep));
  RETURN_ON_ERROR(status);
  
  if (rep.requestCode != JVS_ALLOCATE) {
    errno = rep.requestCode;
    return(ERROR);
  }
  *shmid = rep.shmid;
  return(OK);
}

/* Allocates a shared-memory buffer in the server and returns
   its shmid. */
int JvsAllocateBuf2(int fd, int direction, int type, int w, int h, int *shmid)
{
  JvsAllocateReq req;
  JvsAllocateRep rep;
  
  int status;
  
  req.requestCode = JVS_ALLOCATE2;
  req.direction = direction;
  req.type = type;
  req.width = w;
  req.height = h;
  
  status = JvsSend(fd, &req, sizeof(req));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, &rep, sizeof(rep));
  RETURN_ON_ERROR(status);
  
  if (rep.requestCode != JVS_ALLOCATE) {
    errno = rep.requestCode;
    return(ERROR);
  }
  *shmid = rep.shmid;
  return(OK);
}

/* Deallocate a buffer */
int JvsDeallocBuf(int fd, int shmid)
{
  JvsDeallocateReq req;
  JvsDeallocateRep rep;
  int status;
  
  req.requestCode = JVS_DEALLOCATE;
  req.shmid = shmid;
  
  status = JvsSend(fd, &req, sizeof(req));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, &rep, sizeof(rep));
  RETURN_ON_ERROR(status);
  
  if (rep.requestCode != req.requestCode)
    return(rep.replyCode);
  return(OK);
}

/* Compresses a live video image into shmid. */
int JvsStartComp(int fd, int shmid)
{
  JvsCompressReq req;
  
  req.requestCode = JVS_COMPRESS;
  req.shmid = shmid;
  
  return JvsSend(fd, &req, sizeof(req));
}

/* Waits for a compression request to complete. */
int JvsWaitComp(int fd, int *shmid, int *len)
{
  JvsCompressRep rep;
  int status;
  
  *shmid = 0;
  status = JvsRead(fd, &rep, sizeof(rep));
  RETURN_ON_ERROR(status);
  
  if (rep.requestCode != JVS_COMPRESS)
    return(ERROR);
  *shmid = rep.shmid;
  *len = rep.length;
  return(0);
}

/* Decompresses cshmid into dshmid.  Will zero the compressed data block
 * from the end of the compressed data to the next 512-byte boundry.
 */
int JvsStartDecomp(int fd, int cshmid, int dshmid, int length)
{
  JvsDecompressReq req;
  
  req.requestCode = JVS_DECOMPRESS;
  req.cshmid = cshmid;
  req.dshmid = dshmid;
  req.length = length;
  
  return JvsSend(fd, &req, sizeof(req));
}

int JvsStartAnalogDecomp(int fd, int cshmid, int length)
{
  JvsDecompressReq req;
  
  req.requestCode = JVS_DECOMPRESS_ANALOG;
  req.cshmid = cshmid;
  req.dshmid = 0;
  req.length = length;
  
  return JvsSend(fd, &req, sizeof(req));
}

/* Waits for the decompression request to complete. */
int JvsWaitDecomp(int fd, int *cshmid, int *dshmid)
{
  JvsDecompressRep rep;
  int status;
  
  *cshmid = *dshmid = 0;
  status = JvsRead(fd, &rep, sizeof(rep));
  if (status < 0) return(status);
  
  if (rep.requestCode != JVS_DECOMPRESS) 
    return (ERROR);
  *cshmid = rep.cshmid;
  *dshmid = rep.dshmid;
  return(0);
}     

/* Sets parameters for compression stream. */
int JvsSetComp(int fd, 
	       int qfactor, 
	       int xdec, 
	       int ydec, 
	       int frameskip, 
	       int *width, 
	       int *height)
{
  JvsSetCompressReq req;
  JvsSetCompressRep rep;
  int status;
  
  req.requestCode = JVS_SETCOMPRESS;
  req.qfactor = qfactor;
  req.xdec = xdec;
  req.ydec = ydec;
  req.frameskip = frameskip;
  req.type = JVS_JPEG;
  
  status = JvsSend(fd, &req, sizeof(req));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, &rep, sizeof(rep));
  RETURN_ON_ERROR(status);
  
  if (rep.requestCode != req.requestCode)
    return(ERROR);
  *width = rep.width;
  *height = rep.height;
  return(OK);
}

/* Sets parameters for compression stream. */
int JvsSetCompRaw(int fd, JvsSetCompressReq *req, JvsSetCompressRep *rep)
{
  int status;
  
  req->requestCode = JVS_SETCOMPRESS;
  
  status = JvsSend(fd, req, sizeof(JvsSetCompressReq));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, rep, sizeof(JvsSetCompressRep));
  RETURN_ON_ERROR(status);
  
  if (rep->requestCode != req->requestCode)
    return(ERROR);
  return(OK);
}

/* Sets parameters for decompression stream. */
int JvsSetDecomp(int fd, 
		 int qfactor,
		 int inX, 
		 int inY, 
		 int *outX,
		 int *outY, 
		 int brightness, 
		 int contrast,
		 int saturation,
		 int *linePadding)
{
  JvsSetDecompressReq req;
  JvsSetDecompressRep rep;
  int status, i, j;
  
  req.requestCode = JVS_SETDECOMPRESS;
  req.qfactor = qfactor;
  req.inX = inX;
  req.inY = inY;
  req.outX = *outX;
  req.outY = *outY;
  req.brightness = brightness;
  req.contrast = contrast;
  req.saturation = saturation;
  
  status = JvsSend(fd, &req, sizeof(req));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, &rep, sizeof(rep));
  RETURN_ON_ERROR(status);
  
  if (rep.requestCode != req.requestCode)
    return(ERROR);
  *outX = rep.actualOutX;
  *outY = rep.actualOutY;
  *linePadding = rep.linePadding;
  return(OK);
}

/* Sets parameters for decompression stream. */
int JvsSetDecompRaw(int fd,
		    JvsSetDecompressReq *req,
		    JvsSetDecompressRep *rep)
{
  int status;
  
  req->requestCode = JVS_SETDECOMPRESS;
  
  status = JvsSend(fd, req, sizeof(JvsSetDecompressReq));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, rep, sizeof(JvsSetDecompressRep));
  RETURN_ON_ERROR(status);
  
  if (rep->requestCode != req->requestCode)
    return(ERROR);
  return(OK);
}

int JvsColormap(int fd, char *name, int id, int *nColors, int monochrome)
{
  JvsColormapReq req;
  JvsColormapRep rep;
  int status;
  
  req.requestCode = JVS_COLORMAP;
  
  strcpy(req.serverName, name);
  req.id = id;
  req.nColors = *nColors;
  req.monochrome = monochrome;
  
  status = JvsSend(fd, &req, sizeof(JvsColormapReq));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, &rep, sizeof(JvsColormapRep));
  RETURN_ON_ERROR(status);
  
  if (rep.requestCode != req.requestCode)
    return(ERROR);
  
  *nColors = rep.nColors;
  return(OK);
}

int JvsColormapQuery(int fd, char *name, int id, int *nColors, int indices[])
{
  JvsColormapQueryReq req;
  JvsColormapQueryRep rep;
  int status, i;
  
  req.requestCode = JVS_COLORMAP_QUERY;
  
  strcpy(req.serverName, name);
  req.id = id;

  status = JvsSend(fd, &req, sizeof(JvsColormapQueryReq));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, &rep, sizeof(JvsColormapQueryRep));
  RETURN_ON_ERROR(status);
  
  if (rep.requestCode != req.requestCode)
    return(ERROR);
  
  *nColors = rep.nColors;
  for (i = 0; i < rep.nColors; i++) indices[i] = rep.indices[i];
  return(OK);
}

int JvsIdentity(int fd, char *identity)
{
  JvsIdentityReq req;
  JvsIdentityRep rep;
  int status;
  
  req.requestCode = JVS_IDENTITY;
  status = JvsSend(fd, &req, sizeof(JvsIdentityReq));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, &rep, sizeof(JvsIdentityRep));
  RETURN_ON_ERROR(status);
  
  strcpy(identity, rep.identity);
  return(OK);
}

int JvsGetDither(int fd, int *brightness, int *contrast, int *saturation)
{
  JvsGetDitherReq req;
  JvsGetDitherRep rep;
  int status;

  req.requestCode = JVS_GETDITHER;
  status = JvsSend(fd, &req, sizeof(JvsGetDitherReq));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, &rep, sizeof(JvsGetDitherRep));
  RETURN_ON_ERROR(status);

  *brightness = rep.brightness;
  *contrast = rep.contrast;
  *saturation = rep.saturation;

  return(OK);
}

int JvsSetDither(int fd, int brightness, int contrast, int saturation)
{
  JvsSetDitherReq req;
  JvsSetDitherRep rep;
  int status;

  req.requestCode = JVS_SETDITHER;
  req.brightness = brightness;
  req.contrast = contrast;
  req.saturation = saturation;

  status = JvsSend(fd, &req, sizeof(JvsSetDitherReq));
  RETURN_ON_ERROR(status);
  
  status = JvsRead(fd, &rep, sizeof(JvsSetDitherRep));
  RETURN_ON_ERROR(status);

  return(OK);
}

/* utilities for sending and receiving messages */

int JvsRead(int fd, char *buf, int len)
{
  int status;
  
  while (len > 0) {
    status = read(fd, buf, len);
    if (status <= 0) {
      if (status == 0) status = -1;
      return(status);
    }
    len -= status;
    buf += status;
  }
  return(OK);
}

int JvsSend(int fd, char *buf, int len)
{
  int status;
  
  while (len > 0) {
    status = write(fd, buf, len);
    RETURN_ON_ERROR(status);
    len -= status;
    buf += status;
  }
  return(OK);
}
#endif /*J300*/
