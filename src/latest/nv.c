/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#ifdef AIX
#include <net/nh.h>
#endif
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <tk.h>
#include "sized_types.h"
#include "rtp.h"
#include "vid_image.h"
#include "vid_util.h"
#include "vid_widget.h"
#include "vid_code.h"
#include "nv.h"
#include "cellb.h"
#ifdef CPV_DECODE
#include "cpv.h"
#endif
#ifdef MAC
#include "mac_grab.h"
#endif
#ifdef PARCVID
#include "parcvid_grab.h"
#endif
#ifdef PARALLAX
#include "parallax_grab.h"
#endif
#ifdef SUNVIDEO
#include "sunvideo_grab.h"
#endif
#ifdef VIDEOPIX
#include "videopix_grab.h"
#endif
#ifdef X11GRAB
#include "x11_grab.h"
#endif
#ifdef INDIGO
#include "indigo_grab.h"
#endif
#ifdef SGIVL
#include "sgivl_grab.h"
#endif
#ifdef PIP
#include "pip_grab.h"
#endif
#ifdef J300
#include "j300_grab.h"
#endif
#ifdef VIDEOLIVE
#include "videolive_grab.h"
#endif
#ifdef IBMVCA
#include "ibmvca_grab.h"
#endif

#define MAX_ICON_WIDTH	96
#define MAX_ICON_HEIGHT	72

#define MAX_LOSS	50

#define MAX_NAMELEN	64
#define MAX_PACKLEN	8192
#define NV_PACKLEN	1200

#define MAX_SOURCES	32

#define GRAB_ERROR_TIME	100	/* msec */
#define IDLE_POLL_TIME	1000	/* msec */

#define MAX_FLUSH_TIME	1	/* sec */
#define MAX_IDLE_TIME	15	/* sec */

extern char *inet_ntoa();

/* Forward declarations */
static void RecvVideoPackets(ClientData clientData, int mask);
static void RecvCtrlPackets(ClientData clientData, int mask);

Tcl_Interp *interp;
Tk_Window tkMainWin;		/* NULL means window has been deleted. */
int color_ok;

static int xmitfd = -1, recvfd = -1, ctrlfd = -1;
static int send_now=0, xmit_size, xmit_color, setup_sockets=1;
static int min_framespacing, max_bandwidth, pkts_sent=0, bytes_sent=0;
static struct sockaddr_in lcladdr, rmtaddr, ctladdr;
static uint8 ttl;
static uint8 *curr_y_data;
static int8 *curr_uv_data;
static char myname[MAX_NAMELEN], address[32];
static uint16 port;
static uint32 myssrc; /* in network byte order */
static Tk_TimerToken send_timer;

static char *grabpanel=NULL;
static grabber_t *grabber, grabbers[] = {
#ifdef MAC
    { "Mac Video", "mac", Mac_Probe, Mac_Attach,
      Mac_Detach, Mac_Start, Mac_Stop, 0 },
#endif
#ifdef PARCVID
    { "PARC Video", "parcvid", PARCVid_Probe, PARCVid_Attach,
      PARCVid_Detach, PARCVid_Start, PARCVid_Stop, 0 },
#endif
#ifdef PARALLAX
    { "Parallax XVideo", "parallax", Parallax_Probe, Parallax_Attach,
      Parallax_Detach, Parallax_Start, Parallax_Stop, 0 },
#endif
#ifdef SUNVIDEO
    { "SunVideo", "sunvideo", SunVideo_Probe, SunVideo_Attach,
      SunVideo_Detach, SunVideo_Start, SunVideo_Stop, 0 },
#endif
#ifdef VIDEOPIX
    { "Sun VideoPix", "videopix", VideoPix_Probe, VideoPix_Attach,
      VideoPix_Detach, VideoPix_Start, VideoPix_Stop, 0 },
#endif
#ifdef SGIVL
    { "SGI Indy/Galileo", "sgivl", SGIVL_Probe, SGIVL_Attach,
      SGIVL_Detach, SGIVL_Start, SGIVL_Stop, 0 },
#endif
#ifdef INDIGO
    { "IndigoVideo", "indigo", Indigo_Probe, Indigo_Attach,
      Indigo_Detach, Indigo_Start, Indigo_Stop, 0 },
#endif
#ifdef PIP
    { "DECstation PIP", "pip", PIP_Probe, PIP_Attach,
      PIP_Detach, PIP_Start, PIP_Stop, 0 },
#endif
#ifdef J300
    { "DEC J300", "j300", J300_Probe, J300_Attach,
      J300_Detach, J300_Start, J300_Stop, 0 },
#endif
#ifdef VIDEOLIVE
    { "HP VideoLive", "videolive", VideoLive_Probe, VideoLive_Attach,
      VideoLive_Detach, VideoLive_Start, VideoLive_Stop, 0 },
#endif
#ifdef IBMVCA
    { "IBM VCA Video", "ibmvca", IBMVCA_Probe, IBMVCA_Attach, IBMVCA_Detach,
      IBMVCA_Start, IBMVCA_Stop, 0 },
#endif
#ifdef X11GRAB
    { "X11 Screen Grab", "x11", X11Grab_Probe, X11Grab_Attach,
      X11Grab_Detach, X11Grab_Start, X11Grab_Stop, 0 },
#endif
    { "", "", 0, 0, 0, 0, 0, 0 }
};

static void *enc_state;
static encoding_t *encoding, encodings[] = {
    { "Native NV", "nv", RTP_PT_NV, NV_Encode_Probe,
      NV_Encode_Start, NV_Encode_Restart, NV_Encode,
      NV_Encode_Stop, 0 },
    { "Sun CellB", "cellb", RTP_PT_CELLB, CellB_Encode_Probe,
      CellB_Encode_Start, CellB_Encode_Restart, CellB_Encode,
      CellB_Encode_Stop, 0 },
    { "", "", 0, 0, 0, 0, 0 }
};

static char *rtp_payload_name[] =
    { "", "", "", "", "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "", "", "", "", "", "",
      "CellB", "JPEG", "CU-SeeMe", "nv", "PicWin", "CPV", "H.261" };

typedef struct {
    uint32		ssrc;
    char		name[MAX_NAMELEN];
    uint8		pt;
    uint8		inuse;
    uint8		active;
    uint8		flushed;
    struct timeval	lastrecv;
    uint32		lastflush;
    uint32		laststamp;
    vidimage_t		*image;
    uint32		pkts_expected, pkts_rcvd, last_sr, last_srtime;
    uint16		startseq[8], maxseq[8];
    uint32		pkts[8], frames[8], shownframes[8], time[8], bytes[8];
    uint32		totseq, totpkts, totframes, totshown, tottime, totbytes;
    int			avgpos;
} source_t;

static int max_source=0;
static source_t source[MAX_SOURCES];

extern char TK_Init[], NV_Init[], NV_GrabPanels[];

char *display=NULL;
int displayMono=0;
static int recvOnly=0;

Tk_ArgvInfo dispArgTable[] = {
    {"-display", TK_ARGV_STRING, NULL, (char *) &display, "Display to use"},
    {NULL, TK_ARGV_END, NULL, NULL, NULL}
};

Tk_ArgvInfo argTable[] = {
    {"-brightness", TK_ARGV_OPTION_VALUE, NULL, "Nv.brightness",
	"Default video receive brightness"},
    {"-bwLimit", TK_ARGV_OPTION_VALUE, NULL, "Nv.maxBandwidthLimit",
	"Video transmit max bandwidth slider limit"},
    {"-contrast", TK_ARGV_OPTION_VALUE, NULL, "Nv.contrast",
	"Default video receive contrast"},
    {"-display", TK_ARGV_STRING, NULL, (char *) &display,
	"Display to use"},
    {"-encoding", TK_ARGV_OPTION_VALUE, NULL, "Nv.encoding",
	"Video encoding method to use"},
    {"-grabber", TK_ARGV_OPTION_VALUE, NULL, "Nv.grabber",
	"Video grabber to use"},
    {"-interface", TK_ARGV_OPTION_VALUE, NULL, "Nv.interface",
	"Network interface to transmit from"},
    {"-maxBandwidth", TK_ARGV_OPTION_VALUE, NULL, "Nv.maxBandwidth",
	"Video transmit maximum bandwidth"},
    {"-maxBandwidthLimit", TK_ARGV_OPTION_VALUE, NULL, "Nv.maxBandwidthLimit",
	"Video transmit maximum bandwidth limit"},
    {"-maxFrameRate", TK_ARGV_OPTION_VALUE, NULL, "Nv.maxFrameRate",
	"Video transmit maximum frame rate"},
    {"-name", TK_ARGV_OPTION_VALUE, NULL, "Nv.name",
	"Video transmit stream name"},
    {"-recvColor", TK_ARGV_CONST_OPTION, "color", "Nv.recvColor",
	"Default video receive to be in color"},
    {"-recvGray", TK_ARGV_CONST_OPTION, "grey", "Nv.recvColor",
	"Default video receive to be greyscale"},
    {"-recvGrey", TK_ARGV_CONST_OPTION, "grey", "Nv.recvColor",
	"Default video receive to be greyscale"},
    {"-mono", TK_ARGV_CONSTANT, (char *) 1, (char *) &displayMono,
	"Run in monochrome mode"},
    {"-recvOnly", TK_ARGV_CONSTANT, (char *) 1, (char *) &recvOnly,
	"Don't try to initialize a frame grabber"},
    {"-recvSize", TK_ARGV_OPTION_VALUE, NULL, "Nv.recvSize",
	"Video receive window size"},
    {"-title", TK_ARGV_OPTION_VALUE, NULL, "Nv.title",
	"Main window title"},
    {"-ttl", TK_ARGV_OPTION_VALUE, NULL, "Nv.ttl",
	"Video transmit multicast TTL"},
    {"-xmitColor", TK_ARGV_CONST_OPTION, "color", "Nv.xmitColor",
	"Set video transmit to be in color"},
    {"-xmitGray", TK_ARGV_CONST_OPTION, "grey", "Nv.xmitColor",
	"Set video transmit to be greyscale"},
    {"-xmitGrey", TK_ARGV_CONST_OPTION, "grey", "Nv.xmitColor",
	"Set video transmit to be greyscale"},
    {"-xmitSize", TK_ARGV_OPTION_VALUE, NULL, "Nv.xmitSize",
	"Video transmit size"},
    {NULL, TK_ARGV_END, NULL, NULL, NULL}
};

/*ARGSUSED*/
static void StructureProc(ClientData clientData, XEvent *eventPtr)
{
    if (eventPtr->type == DestroyNotify) tkMainWin = NULL;
}

static void Cleanup()
{
    Tcl_Eval(interp, "exit");
    /*NOTREACHED*/
}

uint32 RTPTime(void)
{
    struct timeval t;

    gettimeofday(&t, NULL);
    return ((t.tv_sec + RTP_EPOCH_OFFSET) << 16) | ((t.tv_usec << 10) / 15625);
}

static int FindSource(uint32 ssrc, int create)
{
    int i;
 
    for (i=0; i<=max_source; i++)
	if (ssrc == source[i].ssrc) break;

    if (i <= max_source) return i;
    if (!create) return -1;

    if (ssrc == myssrc) {
	i = 0;
    } else {
	for (i=1; i<=max_source; i++) if (!source[i].inuse) break;
    }
 
    if (i > max_source) max_source = i;
 
    memset((char *)&source[i], 0, sizeof(source[i]));
    source[i].ssrc = ssrc;
    sprintf(source[i].name, "%08x", ssrc);
    source[i].inuse = 1;

    return i;
}

static void SetupXmitFD(void)
{
    if (xmitfd != -1) close(xmitfd);

    if ((xmitfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	perror("xmitfd: socket");
	return;
    }

    if (bind(xmitfd, (struct sockaddr *)&lcladdr, sizeof(lcladdr)) == -1)
	perror("xmitfd: bind");

#ifdef IP_ADD_MEMBERSHIP
    if (IN_MULTICAST(ntohl(rmtaddr.sin_addr.s_addr))) {
	if (setsockopt(xmitfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
		       sizeof(ttl)) == -1) perror("ip_multicast_ttl");
    }
#endif

    fcntl(xmitfd, F_SETFL, fcntl(xmitfd, F_GETFL)|O_NDELAY);
}

static void SetupRecvFD(int *fdp, struct sockaddr_in *addr,
			Tk_FileProc *handler)
{
    int fd, one=1, err;
    struct sockaddr_in bindaddr;
#ifdef IP_ADD_MEMBERSHIP
    struct ip_mreq mreq;
#endif

    if (*fdp != -1) {
	Tk_DeleteFileHandler(*fdp);
	close(*fdp);
    }

    if ((*fdp = fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	perror("socket");
	return;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one,
		   sizeof(one)) == -1) perror("reuseaddr");

    bindaddr.sin_family = AF_INET;
    bindaddr.sin_port = addr->sin_port;
#ifdef IP_ADD_MEMBERSHIP
    if (IN_MULTICAST(ntohl(addr->sin_addr.s_addr))) {
	bindaddr.sin_addr.s_addr = addr->sin_addr.s_addr;
	if ((err = bind(fd, (struct sockaddr *)&bindaddr,
			sizeof(bindaddr))) == -1) {
	    bindaddr.sin_addr.s_addr = INADDR_ANY;
	    err = bind(fd, (struct sockaddr *)&bindaddr, sizeof(bindaddr));
	}
    } else
#endif
    {
	bindaddr.sin_addr.s_addr = INADDR_ANY;
	err = bind(fd, (struct sockaddr *)&bindaddr, sizeof(bindaddr));
    }

    if (err == -1) perror("recvfd: bind");

#ifdef IP_ADD_MEMBERSHIP
    if (IN_MULTICAST(ntohl(addr->sin_addr.s_addr))) {
	mreq.imr_multiaddr = addr->sin_addr;
	mreq.imr_interface = lcladdr.sin_addr;
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq,
		       sizeof(mreq)) == -1) perror("recvfd: ip_add_membership");
    }
#endif

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NDELAY);

    Tk_CreateFileHandler(fd, TK_READABLE, handler, NULL);
}

/*ARGSUSED*/
static void SetupSockets(ClientData clientData)
{
    struct hostent *hp;

    if (!setup_sockets) return;

    lcladdr.sin_family = AF_INET;
    (void) Tcl_Eval(interp, "option get . interface Nv");
    lcladdr.sin_addr.s_addr = inet_addr(interp->result);
    if (lcladdr.sin_addr.s_addr == -1) {
	hp = gethostbyname(interp->result);
	if (hp == NULL) {
	    fprintf(stderr, "Can't resolve host %s\n", interp->result);
	    lcladdr.sin_addr.s_addr = INADDR_ANY;
	} else {
	    memcpy((char *) &lcladdr.sin_addr, hp->h_addr, hp->h_length);
	}
    }
    lcladdr.sin_port = 0;

    rmtaddr.sin_family = AF_INET;
    rmtaddr.sin_addr.s_addr = inet_addr(address);
    if (rmtaddr.sin_addr.s_addr == -1) {
	hp = gethostbyname(address);
	if (hp == NULL) {
	    fprintf(stderr, "Can't resolve host %s\n", address);
	    rmtaddr.sin_addr.s_addr = INADDR_ANY;
	} else {
	    memcpy((char *) &rmtaddr.sin_addr, hp->h_addr, hp->h_length);
	}
    }
    rmtaddr.sin_port = htons(port);

    ctladdr = rmtaddr;
    ctladdr.sin_port = htons(port+1);

    SetupXmitFD();
    SetupRecvFD(&recvfd, &rmtaddr, RecvVideoPackets);
    SetupRecvFD(&ctrlfd, &ctladdr, RecvCtrlPackets);
    setup_sockets = 0;
}

void RecvVideoData(uint8 *packet, int len)
{
    int i, color, iscolor, width, height, avgpos, cc;
    int newframe=0, shownframe=0;
    rtphdr_t *rtp;
    char *name=NULL;
    uint8 pt, *data;
    struct timeval t;
    int16 seqdiff;
    int32 tsdiff, tdiff;
    char pathname[32], cmd[256];
    decodeproc_t *decode;
 
    rtp = (rtphdr_t *) packet;
    rtp->flags = ntohs(rtp->flags);
    rtp->seq = ntohs(rtp->seq);
    rtp->ts = ntohl(rtp->ts);
    data = (uint8 *)(rtp+1);
    len -= sizeof(rtphdr_t);

    if ((rtp->flags & RTP_TYPEMASK) != RTP_V2) return;

    cc = (rtp->flags & RTP_CCMASK) >> RTP_CCSHIFT;
    data += cc*4;
    len -= cc*4;

    if (rtp->flags & RTP_X) {
	rtphdrx_t *hdrx=(rtphdrx_t *)data;
	int hdrxlen = sizeof(rtphdrx_t)+ntohs(hdrx->len);
	data += hdrxlen;
	len -= hdrxlen;
    }

    if (rtp->flags & RTP_P) len -= data[len-1];

    if (len <= 0) return;

    pt = rtp->flags & RTP_PTMASK;
    switch (pt) {
    case RTP_PT_CELLB:
	color = 1;
	width = NTSC_WIDTH;
	height = NTSC_HEIGHT;
	decode = CellB_Decode;
	break;
    case RTP_PT_NV:
	color = 1;
	width = NTSC_WIDTH;
	height = NTSC_HEIGHT;
	decode = NV_Decode;
	break;
#ifdef CPV_DECODE
    case RTP_PT_CPV:
	color = 1;
	width = CPV_WIDTH;
	height = CPV_HEIGHT;
	decode = CPV_Decode;
	break;
#endif
    default:
	/* Unknown payload type -- discard */
	return;
    }

    i = FindSource(rtp->ssrc, 1);
    if (source[i].image == NULL) {
	source[i].pt = pt;
	source[i].active = 0;
	source[i].flushed = 1;
	source[i].lastflush = 0;
	source[i].laststamp = rtp->ts;

	source[i].image = VidImage_Create(color, width, height);

	sprintf(cmd, "toplevel .nvsource%d", i);
	(void) Tcl_Eval(interp, cmd);
	sprintf(cmd, ".nvsource%d", i);
	sprintf(pathname, ".nvsource%d.video", i);
	(void) VidWidget_Create(interp, pathname, False, (void *) tkMainWin,
				source[i].image, 0, 0);
	sprintf(pathname, ".sources.video%d", i);
	(void) VidWidget_Create(interp, pathname, False, (void *) tkMainWin,
				source[i].image, MAX_ICON_WIDTH,
				MAX_ICON_HEIGHT);
	sprintf(cmd, "addSource %d \"%d (%s)\" %d", i, rtp->ssrc,
		rtp_payload_name[pt], color_ok);
	(void) Tcl_Eval(interp, cmd);
    } else if (source[i].pt != pt) {
	source[i].pt = pt;
	iscolor = ((source[i].image->flags & VIDIMAGE_ISCOLOR) != 0);
	if (color != iscolor)
	    VidImage_SetColor(source[i].image, color,
			      source[i].image->flags & VIDIMAGE_WANTCOLOR);
	sprintf(cmd, "setSourceInfo %d \"%d (%s)\"", i, rtp->ssrc,
		rtp_payload_name[pt]);
	(void) Tcl_Eval(interp, cmd);
    }

    if (source[i].lastrecv.tv_sec == 0) {
	sprintf(cmd, "setSourceName %d \"%s\"", i, source[i].name);
	(void) Tcl_Eval(interp, cmd);
    }

    gettimeofday(&t, NULL);
    tdiff = (t.tv_sec-source[i].lastrecv.tv_sec)*1000000 +
	t.tv_usec-source[i].lastrecv.tv_usec;
    source[i].lastrecv = t;

    tsdiff = rtp->ts - source[i].laststamp;
    if (tsdiff < 0) {
	return;
    } else if (tsdiff > 0) {
	newframe = 1;
	source[i].laststamp = rtp->ts;
	if (((uint32)(t.tv_sec-source[i].lastflush) >= MAX_FLUSH_TIME)  &&
	    !source[i].flushed) {
	    shownframe++;
	    VidImage_Redraw(source[i].image);
	    source[i].flushed = 1;
	    source[i].lastflush = t.tv_sec;
	}
    }

    (void) decode(source[i].image, data, len);

    if (rtp->flags & RTP_M) {
	if ((uint32)(t.tv_sec-source[i].lastflush) >= MAX_FLUSH_TIME) {
	    shownframe++;
	    VidImage_Redraw(source[i].image);
	    source[i].flushed = 1;
	    source[i].lastflush = t.tv_sec;
	}
    } else {
	source[i].flushed = 0;
    }

    source[i].pkts_rcvd++;
    avgpos = source[i].avgpos;
    seqdiff = rtp->seq - source[i].startseq[avgpos];
    if ((seqdiff > MAX_LOSS) || (seqdiff < -MAX_LOSS)) {
	source[i].startseq[avgpos] = rtp->seq;
	source[i].maxseq[avgpos] = rtp->seq-1;
	source[i].pkts[avgpos] = 0;
	seqdiff = 0;
    }
    if (seqdiff >= 0) source[i].pkts[avgpos]++;
    if ((seqdiff = (int16) (rtp->seq - source[i].maxseq[avgpos])) > 0) {
	source[i].pkts_expected += seqdiff;
	source[i].maxseq[avgpos] = rtp->seq;
    }
    source[i].frames[avgpos] += newframe;
    source[i].shownframes[avgpos] += shownframe;
    source[i].time[avgpos] += tdiff;
    source[i].bytes[avgpos] += len;

    if (source[i].time[avgpos] >= 500000) {
	uint16 maxseq = source[i].maxseq[avgpos];

	source[i].totseq += maxseq-source[i].startseq[avgpos]+1;
	source[i].totpkts += source[i].pkts[avgpos];
	source[i].totframes += source[i].frames[avgpos];
	source[i].totshown += source[i].shownframes[avgpos];
	source[i].tottime += source[i].time[avgpos];
	source[i].totbytes += source[i].bytes[avgpos];
	source[i].avgpos = avgpos = (avgpos+1) % 8;

	if (source[i].time[avgpos] != 0) {
	    if (source[i].totseq == 0) {
		sprintf(cmd, "setRecvStats %d -1 -1 -1 -1", i);
	    } else {
		sprintf(cmd, "setRecvStats %d %.1f %.1f %.1f %d", i,
		    1000000.*source[i].totframes/source[i].tottime,
		    1000000.*source[i].totshown/source[i].tottime,
		    8000.*source[i].totbytes/source[i].tottime,
		    100-100*source[i].totpkts/source[i].totseq);
	    }
	    (void) Tcl_Eval(interp, cmd);

	    source[i].totseq -=
		(source[i].maxseq[avgpos]-source[i].startseq[avgpos]+1);
	    source[i].totpkts -= source[i].pkts[avgpos];
	    source[i].totframes -= source[i].frames[avgpos];
	    source[i].totshown -= source[i].shownframes[avgpos];
	    source[i].tottime -= source[i].time[avgpos];
	    source[i].totbytes -= source[i].bytes[avgpos];
	}

	source[i].startseq[avgpos] = maxseq+1;
	source[i].maxseq[avgpos] = maxseq;
	source[i].pkts[avgpos] = 0;
	source[i].frames[avgpos] = 0;
	source[i].shownframes[avgpos] = 0;
	source[i].time[avgpos] = 0;
	source[i].bytes[avgpos] = 0;
    }
}

/*ARGSUSED*/
static void RecvVideoPackets(ClientData clientData, int mask)
{
    int i, len;
    static uint32 packet[MAX_PACKLEN/4];

    Tk_DeleteFileHandler(recvfd);

    while ((len = recv(recvfd, (char *)packet, sizeof(packet), 0)) > 0) {
	RecvVideoData((uint8 *)packet, len);

	/* Between receiving packets catch up on other events */
	while (Tk_DoOneEvent(TK_DONT_WAIT) != 0) ;
    }

    for (i=0; i<=max_source; i++) source[i].lastflush = 0;

    Tk_CreateFileHandler(recvfd, TK_READABLE, RecvVideoPackets, NULL);
}

static void ProcessCtrlPacket(uint8 *packet, int len)
{
    int i, j, count, sdestype, itemlen, chunklen, namelen, align;
    uint8 pt, *p;
    uint32 *ssrcp;
    rtcphdr_t *rtcp;
    rtcp_sr_t *sr;
    rtcp_rr_t *rr;
    rtcp_rritem_t *rritem;
    char cmd[256], name[MAX_NAMELEN];

    while (len > 0) {
	rtcp = (rtcphdr_t *) packet;
	rtcp->flags = ntohs(rtcp->flags);
	rtcp->len = ntohs(rtcp->len);
	packet += sizeof(rtcphdr_t);
	len -= sizeof(rtcphdr_t);

	if (rtcp->flags & RTP_P) len -= packet[len-1];

	if (len < rtcp->len*4) return;

	pt = rtcp->flags & RTCP_PTMASK;
	switch (pt) {
	case RTCP_PT_SR:
	    sr = (rtcp_sr_t *) packet;
	    i = FindSource(sr->ssrc, 1);
	    count = (rtcp->flags & RTCP_CNTMASK) >> RTCP_CNTSHIFT;
	    rritem = (rtcp_rritem_t *)(sr+1);
	    goto process_rrlist;
	case RTCP_PT_RR:
	    rr = (rtcp_rr_t *) packet;
	    i = FindSource(rr->ssrc, 1);
	    count = (rtcp->flags & RTCP_CNTMASK) >> RTCP_CNTSHIFT;
	    rritem = (rtcp_rritem_t *)(rr+1);
	process_rrlist:
	    break;
	case RTCP_PT_SDES:
	    count = (rtcp->flags & RTCP_CNTMASK) >> RTCP_CNTSHIFT;
	    p = packet;
	    chunklen = rtcp->len*4;
	    while ((count-- > 0) && (chunklen > 0)) {
		ssrcp = (uint32 *) p;
		i = FindSource(*ssrcp, 1);
		p += 4;
		chunklen -= 4;

		while ((sdestype = p[0]) != RTCP_SDES_END) {
		    itemlen = p[1];
		    p += 2;
		    chunklen -= (itemlen+2);
		    if (chunklen < 0) break;

		    switch (sdestype) {
		    case RTCP_SDES_CNAME:
			break;
		    case RTCP_SDES_NAME:
			namelen = (itemlen<MAX_NAMELEN-1) ? itemlen
							  : MAX_NAMELEN-1;
			strncpy(name, (const char *) p, namelen);
			name[namelen] = '\0';

			if (strcmp(source[i].name, name) != 0) {
			    strcpy(source[i].name, name);
			    if (source[i].image != NULL) {
				sprintf(cmd, "setSourceName %d \"%s\"", i,
					source[i].name);
				(void) Tcl_Eval(interp, cmd);
			    }
			}
			break;
		    default:
			break;
		    }

		    p += itemlen;
		}

		align = ((uintptr_t) p) & 3;
		if (align > 0) {
		    p += (4-align);
		    chunklen -= (4-align);
		}
	    }
	    break;
	case RTCP_PT_BYE:
	    count = (rtcp->flags & RTCP_CNTMASK) >> RTCP_CNTSHIFT;
	    ssrcp = (uint32 *) packet;
	    for (j=0; j<count; j++) {
		if ((i = FindSource(*ssrcp++, 0)) >= 0) {
		    source[i].inuse = 0;
		    if (source[i].image != NULL) {
			sprintf(cmd, "deleteSource %d", i);
			Tcl_Eval(interp, cmd);
			VidImage_Destroy(source[i].image);
			source[i].image = NULL;
		    }
		}
	    }
	    break;
	default:
	    break;
	}

	packet += rtcp->len*4;
	len -= rtcp->len*4;
    }
}

/*ARGSUSED*/
static void RecvCtrlPackets(ClientData clientData, int mask)
{
    int len;
    static uint32 packet[MAX_PACKLEN/4];

    while ((len = recv(ctrlfd, (char *)packet, sizeof(packet), 0)) > 0)
	ProcessCtrlPacket((uint8 *)packet, len);
}

/*ARGSUSED*/
static void SendNextPacket(ClientData clientData)
{
    if (enc_state != NULL) send_now = 1;
}

/*ARGSUSED*/
static void SendVideo(void)
{
    int len, marker, timeout;
    uint32 timestamp;
    rtphdr_t *rtp;
    uint8 *data;
    static uint16 seq=1;
    static uint32 packet[NV_PACKLEN/4];
 
    if (!send_now) return;

    data = ((uint8 *)packet)+sizeof(rtphdr_t);
    len = sizeof(packet)-sizeof(rtphdr_t);
    timeout = (*encoding->encode)(enc_state, source[0].image, data, &len,
				  &marker, &timestamp);
    if (timeout < 0) {
	send_now = 0;
	send_timer = Tk_CreateTimerHandler(GRAB_ERROR_TIME, SendNextPacket, 0);
	return;
    }

    rtp = (rtphdr_t *)packet;
    rtp->flags = htons(RTP_V2 | (marker ? RTP_M : 0) | encoding->rtp_pt);
    rtp->seq = htons(seq++);
    rtp->ts = htonl(timestamp);
    rtp->ssrc = myssrc;
    (void) sendto(xmitfd, (char *)packet, sizeof(rtphdr_t)+len, 0,
		  (struct sockaddr *)&rmtaddr, sizeof(rmtaddr));

    pkts_sent++;
    bytes_sent += len;

#ifdef IP_ADD_MEMBERSHIP
    if (IN_MULTICAST(ntohl(rmtaddr.sin_addr.s_addr)))
    {
	RecvVideoPackets(0, 0);
    } else
#endif
    {
	source[0].lastflush = 0;
	RecvVideoData((uint8 *)packet, sizeof(rtphdr_t)+len);
    }
    if (timeout >= 20) {
	send_now = 0;
	send_timer = Tk_CreateTimerHandler(timeout, SendNextPacket, 0);
    }
}

static void SendReport(void)
{
    struct {
	rtcphdr_t	h;
	uint32		ssrc;
	uint8		sdestype;
	uint8		sdeslen;
	char		name[MAX_NAMELEN];
    } namepkt;

    namepkt.h.flags = RTP_V2 | (1 << RTCP_CNTSHIFT) | RTCP_PT_SDES;
    namepkt.h.len = (6+strlen(myname))/4+1;
    namepkt.ssrc = myssrc;
    namepkt.sdestype = RTCP_SDES_NAME;
    namepkt.sdeslen = strlen(myname);
    strcpy(namepkt.name, myname);

    (void) sendto(xmitfd, (char *)&namepkt, sizeof(namepkt), 0,
		  (struct sockaddr *)&ctladdr, sizeof(ctladdr));
}

static void SendBye(void)
{
    struct {
	rtcphdr_t	h;
	uint32		ssrc;
    } byepkt;

    byepkt.h.flags = RTP_V2 | (1 << RTCP_CNTSHIFT) | RTCP_PT_BYE;
    byepkt.h.len = 1;
    byepkt.ssrc = myssrc;

    (void) sendto(xmitfd, (char *)&byepkt, sizeof(byepkt), 0,
		  (struct sockaddr *)&ctladdr, sizeof(ctladdr));
}

/*ARGSUSED*/
static void CheckIdle(ClientData clientData)
{
    int i;
    struct timeval t;
    char cmd[256];

    gettimeofday(&t, NULL);

    for (i=0; i<=max_source; i++) {
	if (source[i].image == NULL) continue;

	if (source[i].active) {
	    if ((source[i].lastrecv.tv_sec != 0) &&
		(t.tv_sec-source[i].lastrecv.tv_sec > MAX_IDLE_TIME)) {
		source[i].lastrecv.tv_sec = 0;
		sprintf(cmd, "setSourceName %d \"%s (signal lost)\"", i,
		    source[i].name);
		Tcl_Eval(interp, cmd);
		sprintf(cmd, "setRecvStats %d -1 -1 -1", i);
		Tcl_Eval(interp, cmd);
	    }
	} else {
	    if ((source[i].lastrecv.tv_sec == 0) ||
		(t.tv_sec-source[i].lastrecv.tv_sec > MAX_IDLE_TIME)) {
		sprintf(cmd, "deleteSource %d", i);
		Tcl_Eval(interp, cmd);
		VidImage_Destroy(source[i].image);
		source[i].image = NULL;
		continue;
	    }
	}
    }

    Tk_CreateTimerHandler(IDLE_POLL_TIME, CheckIdle, NULL);
}

/*ARGSUSED*/
static int ExitCmd(ClientData clientData, Tcl_Interp *interp, int argc,
		   char *argv[])
{
    int i;

    for (i=0; i<=max_source; i++) {
	if (source[i].image == NULL) continue;

	source[i].active = 0;
    }

    if (tk_NumMainWindows > 0)
	Tcl_VarEval(interp, "destroy .", NULL);

    if (enc_state != NULL) (*encoding->stop)(enc_state);
    SendBye();
    exit(0);
    /*NOTREACHED*/
}

/*ARGSUSED*/
static int ChangeBrightnessCmd(ClientData clientData, Tcl_Interp *interp,
			       int argc, char *argv[])
{
    int i;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " source brightness\"", NULL);
	return TCL_ERROR;
    }

    i = atoi(argv[1]);
    if ((i < 0) || (i > max_source) || (source[i].image == NULL)) {
	Tcl_AppendResult(interp, "invalid source", NULL);
	return TCL_ERROR;
    }

    VidImage_SetBrightness(source[i].image, atoi(argv[2]));
    return TCL_OK;
}

/*ARGSUSED*/
static int ChangeContrastCmd(ClientData clientData, Tcl_Interp *interp,
			     int argc, char *argv[])
{
    int i;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " source contrast\"", NULL);
	return TCL_ERROR;
    }

    i = atoi(argv[1]);
    if ((i < 0) || (i > max_source) || (source[i].image == NULL)) {
	Tcl_AppendResult(interp, "invalid source", NULL);
	return TCL_ERROR;
    }

    VidImage_SetContrast(source[i].image, atoi(argv[2]));
    return TCL_OK;
}

/*ARGSUSED*/
static int ChangeMaxBandwidthCmd(ClientData clientData, Tcl_Interp *interp,
				 int argc, char *argv[])
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " bandwidth\"", NULL);
	return TCL_ERROR;
    }

    max_bandwidth = atoi(argv[1]);

    if (enc_state != NULL) {
	enc_state = (*encoding->restart)(enc_state, max_bandwidth,
					 min_framespacing,
					 xmit_size|xmit_color);
	if (enc_state == NULL) send_now = 0;
    }

    return TCL_OK;
}

/*ARGSUSED*/
static int ChangeAddressCmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[])
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " address\"", NULL);
	return TCL_ERROR;
    }

    strncpy(address, argv[1], sizeof(address)-1);
    address[sizeof(address)-1] = '\0';
    setup_sockets = 1;
    Tk_DoWhenIdle(SetupSockets, 0);
    return TCL_OK;
}

/*ARGSUSED*/
static int ChangePortCmd(ClientData clientData, Tcl_Interp *interp, int argc,
			 char *argv[])
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " port\"", NULL);
	return TCL_ERROR;
    }

    port = atoi(argv[1]);
    setup_sockets = 1;
    Tk_DoWhenIdle(SetupSockets, 0);
    return TCL_OK;
}

/*ARGSUSED*/
static int ChangeNameCmd(ClientData clientData, Tcl_Interp *interp, int argc,
			 char *argv[])
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " name\"", NULL);
	return TCL_ERROR;
    }

    strncpy(myname, argv[1], sizeof(myname)-1);
    myname[sizeof(myname)-1] = '\0';
    return TCL_OK;
}

/*ARGSUSED*/
static int ChangeTTLCmd(ClientData clientData, Tcl_Interp *interp, int argc,
			char *argv[])
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " ttl\"", NULL);
	return TCL_ERROR;
    }

    ttl = atoi(argv[1]);
    setup_sockets = 1;
    Tk_DoWhenIdle(SetupSockets, 0);
    return TCL_OK;
}

/*ARGSUSED*/
static int ChangeEncodingCmd(ClientData clientData, Tcl_Interp *interp,
			     int argc, char *argv[])
{
    int i, sending=(enc_state != NULL);

    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " encoding\"", NULL);
	return TCL_ERROR;
    }

    if (argv[1][0] == '\0') {
	for (i=0; encodings[i].probe != NULL; i++)
	    if (encodings[i].available) break;
    } else {
	for (i=0; encodings[i].probe != NULL; i++)
	    if (!strcmp(encodings[i].keyword, argv[1])) break;
    }

    if (!encodings[i].available) {
	encoding = NULL;
	Tcl_AppendResult(interp, "Encoding '", argv[1], "' not found", NULL);
	return TCL_ERROR;
    }

    if (sending) (void) Tcl_Eval(interp, "stopSending");

    encoding = &encodings[i];
    (void) Tcl_SetVar(interp, "nvEncoding", encoding->keyword, TCL_GLOBAL_ONLY);

    if (sending) (void) Tcl_Eval(interp, "startSending");

    return TCL_OK;
}

/*ARGSUSED*/
static int ChangeGrabberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
			    char *argv[])
{
    int i, sending=(enc_state != NULL);
    static char cmd[256];

    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " grabber\"", NULL);
	return TCL_ERROR;
    }

    if (argv[1][0] == '\0') {
	for (i=0; grabbers[i].probe != NULL; i++)
	    if (grabbers[i].config_mask != 0) break;
    } else {
	for (i=0; grabbers[i].probe != NULL; i++)
	    if (!strcmp(grabbers[i].keyword, argv[1])) break;
    }

    if (grabbers[i].config_mask == 0) {
	Tcl_AppendResult(interp, "Grabber '", argv[1], "' not found", NULL);
	return TCL_ERROR;
    }

    if (sending) (void) Tcl_Eval(interp, "stopSending");

    if (grabpanel) (void) Tcl_VarEval(interp, "pack unpack ", grabpanel, NULL);
    (void) Tcl_Eval(interp, "stopSending");
    if (grabber) (*grabber->detach)();

    grabber = &grabbers[i];
    if ((grabpanel = (*grabber->attach)()) != NULL) {
	(void) Tcl_VarEval(interp, "pack append .grabControls ", grabpanel,
	    " {top fill}", NULL);
	(void) Tcl_VarEval(interp,
	    ".menu.panels.m enable \"Show grabber controls\"", NULL);
    } else {
	(void) Tcl_VarEval(interp, "set nvGrabPanel 0; checkPanels", NULL);
	(void) Tcl_VarEval(interp,
	    ".menu.panels.m disable \"Show grabber controls\"", NULL);
    }

    sprintf(cmd, "updateXmitPanel %d", grabber->config_mask);
    (void) Tcl_Eval(interp, cmd);

    (void) Tcl_SetVar(interp, "nvGrabber", grabber->keyword, TCL_GLOBAL_ONLY);

    if (sending) (void) Tcl_Eval(interp, "startSending");

    return TCL_OK;
}

/*ARGSUSED*/
static int SendVideoCmd(ClientData clientData, Tcl_Interp *interp, int argc,
			char *argv[])
{
    int new_send_state;

    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " {0|1}\"", NULL);
	return TCL_ERROR;
    }

    new_send_state = atoi(argv[1]);
    if ((enc_state == NULL) && new_send_state) {
	if (encoding == NULL) {
	    Tcl_AppendResult(interp, "invalid encoding selected", NULL);
	    return TCL_ERROR;
	}

	enc_state = (*encoding->start)(grabber, max_bandwidth,
				       min_framespacing,
				       xmit_size|xmit_color);
	if (enc_state == NULL) {
	    Tcl_AppendResult(interp, "Unable to initialize ", encoding->name,
			     " encoding on ", grabber->name, ".", NULL);
	    return TCL_ERROR;
	} else {
	    send_now = 1;
	    Tk_DeleteTimerHandler(send_timer);
	}
    } else if ((enc_state != NULL) && !new_send_state) {
	(*encoding->stop)(enc_state);
	enc_state = NULL;
	send_now = 0;
	Tk_DeleteTimerHandler(send_timer);
    }

    return TCL_OK;
}

/*ARGSUSED*/
static int ReceiveVideoCmd(ClientData clientData, Tcl_Interp *interp, int argc,
			   char *argv[])
{
    int i;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " source {0|1}\"", NULL);
	return TCL_ERROR;
    }

    i = atoi(argv[1]);
    if ((i < 0) || (i > max_source) || (source[i].image == NULL)) {
	Tcl_AppendResult(interp, "invalid source", NULL);
	return TCL_ERROR;
    }

    source[i].active = atoi(argv[2]);

    return TCL_OK;
}

/*ARGSUSED*/
static char *TraceXmitSize(ClientData clientData, Tcl_Interp *interp,
			   char *name1, char *name2, int flags)
{
    char *value;

    value = Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY);
    if (!strcmp(value, "small")) {
	xmit_size = VID_SMALL;
    } else if (!strcmp(value, "medium")) {
	xmit_size = VID_MEDIUM;
    } else if (!strcmp(value, "large")) {
	xmit_size = VID_LARGE;
    }

    if (enc_state != NULL) {
	enc_state = (*encoding->restart)(enc_state, max_bandwidth,
					 min_framespacing,
					 xmit_size|xmit_color);
	if (enc_state == NULL) send_now = 0;
    }

    return NULL;
}

/*ARGSUSED*/
static char *TraceXmitColor(ClientData clientData, Tcl_Interp *interp,
			    char *name1, char *name2, int flags)
{
    char *value;

    value = Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY);
    if (!strcmp(value, "gray") || !strcmp(value, "grey")) {
	xmit_color = VID_GREYSCALE;
    } else if (!strcmp(value, "color")) {
	xmit_color = VID_COLOR;
    }

    if (enc_state != NULL) {
	enc_state = (*encoding->restart)(enc_state, max_bandwidth,
					 min_framespacing,
					 xmit_size|xmit_color);
	if (enc_state == NULL) send_now = 0;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    static char buf[256];
    char *cmd, *slash, *addr, *port, *enc;
    struct hostent *hp;
    struct passwd *pw;
    int i, grabber_found=0, max_framerate;

    srand48(time(0));
    myssrc = lrand48();

    interp = Tcl_CreateInterp();
    cmd = Tcl_Concat(argc, argv);
    (void) Tk_ParseArgv(interp, NULL, &argc, argv, dispArgTable,
	TK_ARGV_NO_DEFAULTS);

    tkMainWin = Tk_CreateMainWindow(interp, display, "nv", "Nv");
    if (tkMainWin == NULL) {
	fprintf(stderr, "%s\n", interp->result);
	exit(1);
    }

    (void) Tcl_VarEval(interp, "wm command . \"", cmd, "\"", NULL);
    free(cmd);

    Tk_CreateEventHandler(tkMainWin, StructureNotifyMask, StructureProc, NULL);

    if (Tk_ParseArgv(interp, tkMainWin, &argc, argv, argTable, 0) != TCL_OK) {
	fprintf(stderr, "%s\n", interp->result);
	exit(1);
    }

    VidUtil_Init(Tk_Display(tkMainWin));

    if (!recvOnly) {
	for (i=0; grabbers[i].probe != NULL; i++) {
	    if ((grabbers[i].config_mask = (*grabbers[i].probe)()) != 0)
		grabber_found = 1;
	}
    }

    sprintf(buf, "%d", grabber_found);
    Tcl_SetVar(interp, "grabber_found", buf, TCL_GLOBAL_ONLY);

    if (argc < 2) {
	fprintf(stderr, "Usage: nv [options] address\n");
	fprintf(stderr, "   or: nv -help     to list possible options\n");
	exit(1);
    }

    addr = argv[1];
    argv += 2;
    argc -= 2;

    if ((slash = strchr(addr, '/')) != NULL) {
	*slash = '\0';
	port = slash+1;
    } else if (argc > 0) {
	port = *argv++;
	argc--;
    } else {
	port = "";
    }

    if ((slash = strchr(port, '/')) != NULL) {
	*slash = '\0';
	enc = slash+1;
    } else if (argc > 0) {
	enc = *argv++;
	argc--;
    } else {
	enc = "";
    }

    (void) Tcl_VarEval(interp, "option add Nv.address \"", addr, "\"", NULL);

    if (port[0] != '\0')
	(void) Tcl_VarEval(interp, "option add Nv.port \"", port,
			   "\"", NULL);

    if (enc[0] != '\0')
	(void) Tcl_VarEval(interp, "option add Nv.encoding \"", enc,
			   "\"", NULL);

    (void) Tcl_VarEval(interp, "option add Nv.interface 0.0.0.0 widgetDefault",
		       NULL);

    if ((pw = getpwuid(getuid())) == NULL) {
	strcpy(myname, "nobody");
    } else {
	strcpy(myname, pw->pw_name);
    }

    strcat(myname, "@");

    gethostname(buf, sizeof(buf)-1);
    if ((hp = gethostbyname(buf)) != NULL)
	    strncpy(buf, hp->h_name, sizeof(buf)-1);
    strncpy(myname+strlen(myname), buf, sizeof(myname)-strlen(myname)-1);
    myname[sizeof(myname)-1] = '\0';

    Tcl_CreateCommand(interp, "exit", ExitCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeBrightness", ChangeBrightnessCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeContrast", ChangeContrastCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeMaxBandwidth", ChangeMaxBandwidthCmd, 0,
		      NULL);
    Tcl_CreateCommand(interp, "changeAddress", ChangeAddressCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changePort", ChangePortCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeName", ChangeNameCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeTTL", ChangeTTLCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeEncoding", ChangeEncodingCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeGrabber", ChangeGrabberCmd, 0, NULL);
    Tcl_CreateCommand(interp, "sendVideo", SendVideoCmd, 0, NULL);
    Tcl_CreateCommand(interp, "receiveVideo", ReceiveVideoCmd, 0, NULL);

    Tcl_TraceVar(interp, "nvXmitSize", TCL_TRACE_WRITES, TraceXmitSize, NULL);
    Tcl_TraceVar(interp, "nvXmitColor", TCL_TRACE_WRITES, TraceXmitColor, NULL);

    (void) Tcl_VarEval(interp, "option add Nv.name \"", myname,
	"\" widgetDefault", NULL);

    if (Tcl_VarEval(interp, TK_Init, NV_Init, NV_GrabPanels, NULL) != TCL_OK) {
	fprintf(stderr, "%s\n", interp->result);
	exit(1);
    }

    for (i=0; grabbers[i].probe != NULL; i++) {
	char *state = (grabbers[i].config_mask != 0)? "normal" : "disabled";
	sprintf(buf, "addGrabber \"%s\" \"%s\" %s", grabbers[i].keyword,
		grabbers[i].name, state);
	(void) Tcl_Eval(interp, buf);
    }

    if (grabber_found) {
	for (i=0; encodings[i].probe != NULL; i++) {
	    encodings[i].available = (*encodings[i].probe)(grabber);
	}
    }

    for (i=0; encodings[i].probe != NULL; i++) {
	char *state = encodings[i].available? "normal" : "disabled";
	sprintf(buf, "addEncoding \"%s\" \"%s\" %s", encodings[i].keyword,
		encodings[i].name, state);
	(void) Tcl_Eval(interp, buf);
    }

    if (grabber_found) {
	if (Tcl_VarEval(interp, "changeGrabber $nvGrabber", NULL) != TCL_OK) {
	    fprintf(stderr, "%s\n", interp->result);
	    exit(1);
	}

	if (Tcl_VarEval(interp, "changeEncoding $nvEncoding", NULL) != TCL_OK) {
	    fprintf(stderr, "%s\n", interp->result);
	    exit(1);
	}
    }

    (void) Tcl_VarEval(interp, "option get . maxFrameRate Nv", NULL);
    max_framerate = atoi(interp->result);
    min_framespacing = (max_framerate == 0) ? 0 : 1000/max_framerate;

    color_ok = VidWidget_Init(Tk_Display(tkMainWin));

    (void) Tcl_Eval(interp, "update");

    signal(SIGHUP, Cleanup);
    signal(SIGINT, Cleanup);
    signal(SIGQUIT, Cleanup);
    signal(SIGPIPE, Cleanup);
    signal(SIGTERM, Cleanup);

    Tk_CreateTimerHandler(IDLE_POLL_TIME, CheckIdle, NULL);

    SendReport();
    while (tk_NumMainWindows > 0) {
	if (send_now) {
	    SendVideo();
	    while (tk_NumMainWindows > 0)
	        if (Tk_DoOneEvent(TK_DONT_WAIT) == 0) break;
	} else {
	    (void) Tk_DoOneEvent(0);
	}
    }

    Tcl_Eval(interp, "exit");
    exit(0);
    /*NOTREACHED*/
}
