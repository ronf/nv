/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>
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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <netinet/in.h>
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
#include "cuseeme.h"
#include "cellb.h"
#ifdef CPV_DECODE
#include "cpv.h"
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

#define MAX_SOURCES	32

#define GRAB_ERROR_TIME	100	/* msec */
#define IDLE_POLL_TIME	1000	/* msec */

#define MAX_FLUSH_TIME	1	/* sec */
#define NAME_XMIT_TIME	5	/* sec */
#define MAX_IDLE_TIME	15	/* sec */

extern char *inet_ntoa();

/* Forward declarations */
static void RecvVideoPackets(ClientData clientData, int mask);

Tcl_Interp *interp;
Tk_Window tkMainWin;		/* NULL means window has been deleted. */
int color_ok;

static int xmitfd = -1, recvfd = -1;
static int send_active=0, send_now=0, xmit_size, xmit_color, setup_sockets=1;
static int max_framerate, max_bandwidth;
static struct sockaddr_in myaddr, rmtaddr;
static uint8 ttl;
static uint8 *curr_y_data;
static int8 *curr_uv_data;
static uint32 last_namexmit;
static char myname[MAX_NAMELEN], address[32];
static uint16 port; /* in network byte order */
static uint8 chanid;
static Tk_TimerToken send_timer;

static char *grabpanel=NULL;
static grabber_t *grabber, grabbers[] = {
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

static encodeproc_t *encode;
static encoding_t *encoding, encodings[] = {
    { "Native NV", "nv", NV_Encode_Probe, NV_Encode_Start,
      NV_Encode_Stop, 0 },
    { "Sun CellB", "cellb", CellB_Encode_Probe, CellB_Encode_Start,
      CellB_Encode_Stop, 0 },
    { "CU-SeeMe", "cuseeme", CUSeeMe_Encode_Probe, CUSeeMe_Encode_Start,
      CUSeeMe_Encode_Stop, 0 },
    { "", "", 0, 0, 0, 0 }
};

static char *rtpcont_name[] =
    { "", "", "", "", "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "", "", "", "", "", "",
      "CellB", "JPEG", "CU-SeeMe", "nv", "PicWin", "CPV", "H.261" };

typedef struct {
    struct sockaddr_in	srcaddr;
    uint16		ssrc;
    uint8		content;
    uint8		active;
    uint8		flushed;
    char		name[MAX_NAMELEN];
    struct timeval	lastrecv;
    uint32		lastflush;
    uint32		laststamp;
    vidimage_t		*image;
    uint16		startseq[8], maxseq[8];
    uint32		pkts[8], frames[8], shownframes[8], time[8], bytes[8];
    uint32		totseq, totpkts, totframes, totshown, tottime, totbytes;
    int			avgpos;
} source_t;

static int max_source=0;
static source_t source[MAX_SOURCES];

extern char TK_Init[], NV_Subr[], NV_UIProcs[], NV_API[];
extern char NV_Init[], NV_GrabPanels[];

char *display=NULL;
static int detach=0, recvOnly=0;

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
    {"-detach", TK_ARGV_CONSTANT, (char *) 1, (char *) &detach,
	"Detach into the background"},
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

static int MatchNetAddr(struct sockaddr_in *addr1, struct sockaddr_in *addr2)
{
    return ((addr1->sin_addr.s_addr == addr2->sin_addr.s_addr) &&
	    (addr1->sin_port == addr2->sin_port));
}

static void FillRTPHeader(struct rtphdr *hdrp, int chanid, int opts, int sync,
			  int content, uint16 seq, uint32 timestamp)
{
    hdrp->rh_vers = RTP_VERSION;
    hdrp->rh_chanid = chanid;
    hdrp->rh_opts = opts;
    hdrp->rh_sync = sync;
    hdrp->rh_content = content;
    hdrp->rh_seq = htons(seq++);
    hdrp->rh_ts = htonl(timestamp);
}

static int FillSDESCOption(struct rtcpsdeschdr *opt, int fin,
			   struct sockaddr_in *addr, char *name)
{
    char *p=(char *) (opt+1);
    int optlen=(sizeof(*opt)+strlen(name))/4+1;

    opt->rtsh_fin = fin;
    opt->rtsh_type = RTPOPT_SDESC;
    opt->rtsh_optlen = optlen;
    opt->rtsh_id = 0;
    strcpy(p, name);

    return optlen*4;
}

static int NewSource(struct sockaddr_in *addr, uint16 ssrc, int color,
		     int width, int height, int content)
{
    int i;
    char pathname[32], cmd[256];
 
    if (MatchNetAddr(addr, &myaddr)) {
	i = 0;
    } else {
	for (i=1; i<=max_source; i++)
	    if (source[i].image == NULL) break;
    }
 
    if (i > max_source) max_source = i;
 
    memset((char *)&source[i], 0, sizeof(source[i]));
    source[i].srcaddr = *addr;
    source[i].ssrc = ssrc;
    source[i].content = content;
    sprintf(source[i].name, "%s/%d/%d (%s)", inet_ntoa(addr->sin_addr),
	addr->sin_port, ssrc, rtpcont_name[content]);

    source[i].image = VidImage_Create(color, width, height);

    sprintf(cmd, "toplevel .nvsource%d", i);
    (void) Tcl_Eval(interp, cmd);
    sprintf(cmd, ".nvsource%d", i);
    sprintf(pathname, ".nvsource%d.video", i);
    (void) VidWidget_Create(interp, pathname, False, (void *) tkMainWin,
			    source[i].image, 0, 0);
    sprintf(pathname, ".sources.video%d", i);
    (void) VidWidget_Create(interp, pathname, False, (void *) tkMainWin,
			    source[i].image, MAX_ICON_WIDTH, MAX_ICON_HEIGHT);
    sprintf(cmd, "addSource %d \"%s\" %d", i, source[i].name, color_ok);
    (void) Tcl_Eval(interp, cmd);

    return i;
}

static int ParseRTPOptions(uint8 *packet, uint16 *ssrcp, char **namep)
{
    int done;
    struct rtphdr *rtp=(struct rtphdr *)packet;
    struct rtpopthdr *opt=(struct rtpopthdr *)(rtp+1);
    struct rtpssrchdr *ssrcopt;
    struct rtcpsdeschdr *sopt;

    *ssrcp = 0;
    *namep = NULL;
    done = !rtp->rh_opts;
    while (!done) {
	if (opt->roh_optlen == 0) break;

	switch (opt->roh_type) {
	case RTPOPT_SSRC:
	    ssrcopt = (struct rtpssrchdr *) opt;
	    *ssrcp = ssrcopt->rsh_id;
	    break;
	case RTPOPT_SDESC:
	    sopt = (struct rtcpsdeschdr *) opt;
	    *namep = (char *) (sopt+1);
	    break;
	}

	done = opt->roh_fin;
	opt = (struct rtpopthdr *) (((uint32 *)opt)+opt->roh_optlen);
    }

    return ((uint8 *)opt)-packet;
}

static void SetupXmitFD(void)
{
    int addrlen;
    struct hostent *hp;

    if (xmitfd == -1) {
	if ((xmitfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	    perror("xmitfd: socket");

	myaddr.sin_family = AF_INET;
	(void) Tcl_Eval(interp, "option get . interface Nv");
	myaddr.sin_addr.s_addr = inet_addr(interp->result);
	if (myaddr.sin_addr.s_addr == -1) {
	    hp = gethostbyname(interp->result);
	    if (hp == NULL) {
		fprintf(stderr, "Can't resolve host %s\n", interp->result);
	    } else {
		memcpy((char *) &myaddr.sin_addr, hp->h_addr, hp->h_length);
	    }
	}
	myaddr.sin_port = 0;

	if (bind(xmitfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1)
	    perror("xmitfd: bind");

	fcntl(xmitfd, F_SETFL, fcntl(xmitfd, F_GETFL)|O_NDELAY);
    }

    if (connect(xmitfd, (struct sockaddr *)&rmtaddr, sizeof(rmtaddr)) == -1)
	perror("xmitfd: connect");

#ifdef IP_ADD_MEMBERSHIP
    if (IN_MULTICAST(ntohl(rmtaddr.sin_addr.s_addr))) {
	if (setsockopt(xmitfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
		       sizeof(ttl)) == -1) {
	    perror("ip_multicast_ttl");
	    exit(1);
	}
    }
#endif

    addrlen = sizeof(myaddr);
    getsockname(xmitfd, (struct sockaddr *)&myaddr, &addrlen);

#ifdef IP_ADD_MEMBERSHIP
    /* The BSD networking code puts a real interface address in myaddr.sin_addr,
     * whereas SunOS 5.x leave it set to whatever it was (zero in this case).
     * We need to know the interface address, however, so that we can recognize
     * our own packets in NewSource().
     */
    if ((myaddr.sin_addr.s_addr == htonl(INADDR_ANY)) &&
	(IN_MULTICAST(ntohl(rmtaddr.sin_addr.s_addr))) &&
	(get_multicast_interface(&myaddr.sin_addr) == 0)) {
	fprintf(stderr, "Warning: Can't get multicast interface address!\n");
    }
#endif
}

static void SetupRecvFD(void)
{
    int one=1, err;
    struct sockaddr_in bindaddr;
#ifdef IP_ADD_MEMBERSHIP
    struct ip_mreq mreq;
#endif

    if (recvfd != -1) {
	Tk_DeleteFileHandler(recvfd);
	close(recvfd);
    }

    if ((recvfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	perror("socket");

    if (setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, (char *) &one,
		   sizeof(one)) == -1) perror("reuseaddr");

    bindaddr = rmtaddr;
#ifdef IP_ADD_MEMBERSHIP
    if (IN_MULTICAST(ntohl(rmtaddr.sin_addr.s_addr))) {
	if ((err = bind(recvfd, (struct sockaddr *)&bindaddr,
			sizeof(bindaddr))) == -1) {
	    bindaddr.sin_addr.s_addr = INADDR_ANY;
	    err = bind(recvfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr));
	}
    } else
#endif
    {
	bindaddr.sin_addr.s_addr = INADDR_ANY;
	err = bind(recvfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr));
    }

    if (err == -1)
	perror("recvfd: bind");

#ifdef IP_ADD_MEMBERSHIP
    if (IN_MULTICAST(ntohl(rmtaddr.sin_addr.s_addr))) {
	mreq.imr_multiaddr = rmtaddr.sin_addr;
	mreq.imr_interface = myaddr.sin_addr;
	if (setsockopt(recvfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq,
		       sizeof(mreq)) == -1)
	    perror("recvfd: ip_add_membership");
    }
#endif

    fcntl(recvfd, F_SETFL, fcntl(recvfd, F_GETFL)|O_NDELAY);

    Tk_CreateFileHandler(recvfd, TK_READABLE, RecvVideoPackets, NULL);
}

/*ARGSUSED*/
static void SetupSockets(ClientData clientData)
{
    struct hostent *hp;

    if (!setup_sockets) return;

    rmtaddr.sin_family = AF_INET;
    rmtaddr.sin_addr.s_addr = inet_addr(address);
    if (rmtaddr.sin_addr.s_addr == -1) {
	hp = gethostbyname(address);
	if (hp == NULL)
	    fprintf(stderr, "Can't resolve host %s\n", address);
	else
	    memcpy((char *) &rmtaddr.sin_addr, hp->h_addr, hp->h_length);
    }
    rmtaddr.sin_port = port;

    SetupXmitFD();
    SetupRecvFD();
    setup_sockets = 0;
}

void RecvVideoData(struct sockaddr_in *srcaddrp, uint8 *packet, int len)
{
    int i, j, color, iscolor, width, height, content, avgpos;
    int newframe=0, shownframe=0, optlen;
    struct rtphdr *rtp;
    uint16 ssrc;
    char *name;
    uint8 *data;
    struct timeval t;
    int16 seqdiff;
    int32 tsdiff, tdiff;
    static char cmd[256];
    decodeproc_t *decode;
 
    rtp = (struct rtphdr *) packet;
    if ((rtp->rh_vers != RTP_VERSION) || (rtp->rh_chanid != chanid)) return;

    optlen = ParseRTPOptions(packet, &ssrc, &name);
    data = packet+optlen;
    len -= optlen;

    content = rtp->rh_content;
    switch (content) {
    case RTPCONT_CELLB:
	color = 1;
	width = NTSC_WIDTH;
	height = NTSC_HEIGHT;
	decode = CellB_Decode;
	break;
    case RTPCONT_CUSEEME:
	color = 0;
	width = CUSEEME_FULLWIDTH;
	height = CUSEEME_FULLHEIGHT;
	decode = CUSeeMe_Decode;
	break;
    case RTPCONT_NV:
	color = 1;
	width = NTSC_WIDTH;
	height = NTSC_HEIGHT;
	decode = NV_Decode;
	break;
#ifdef CPV_DECODE
    case RTPCONT_CPV:
	color = 1;
	width = CPV_WIDTH;
	height = CPV_HEIGHT;
	decode = CPV_Decode;
	break;
#endif
    default:
	/* Unknown packet content type -- discard */
	return;
    }

    for (i=0, j=0; i<=max_source; i++) {
	if (source[i].image == NULL) continue;
	if ((MatchNetAddr(srcaddrp, &source[i].srcaddr)) &&
	    (ssrc == source[i].ssrc)) break;
	if ((j == 0)  &&
	    (srcaddrp->sin_addr.s_addr == source[i].srcaddr.sin_addr.s_addr) &&
	    (source[i].lastrecv.tv_sec == 0)) j = i;
    }

    rtp->rh_seq = ntohs(rtp->rh_seq);
    rtp->rh_ts = ntohl(rtp->rh_ts);

    if (i > max_source) {
	if (j != 0) {
	    i = j;
	    source[i].srcaddr = *srcaddrp;
	    source[i].ssrc = ssrc;
	} else {
	    i = NewSource(srcaddrp, ssrc, color, width, height,
		rtp->rh_content);
	    source[i].active = 0;
	}
	source[i].flushed = 1;
	source[i].lastflush = 0;
	source[i].laststamp = rtp->rh_ts;
    } else if (source[i].content != content) {
	source[i].content = content;
	iscolor = ((source[i].image->flags & VIDIMAGE_ISCOLOR) != 0);
	if (color != iscolor)
	    VidImage_SetColor(source[i].image, color,
			      source[i].image->flags & VIDIMAGE_WANTCOLOR);
	sprintf(cmd, "setSourceInfo %d \"%s/%d/%d (%s)\"", i,
		inet_ntoa(source[i].srcaddr.sin_addr),
		source[i].srcaddr.sin_port, ssrc,
		rtpcont_name[content]);
	(void) Tcl_Eval(interp, cmd);
    }


    if ((name != NULL) && (strcmp(source[i].name, name) != 0)) {
	strncpy(source[i].name, name, sizeof(source[i].name)-1);
	source[i].name[sizeof(source[i].name)-1] = '\0';
	sprintf(cmd, "setSourceName %d \"%s\"", i, source[i].name);
	(void) Tcl_Eval(interp, cmd);
    } else if (source[i].lastrecv.tv_sec == 0) {
	sprintf(cmd, "setSourceName %d \"%s\"", i, source[i].name);
	(void) Tcl_Eval(interp, cmd);
    }

    gettimeofday(&t, NULL);
    tdiff = (t.tv_sec-source[i].lastrecv.tv_sec)*1000000 +
	t.tv_usec-source[i].lastrecv.tv_usec;
    source[i].lastrecv = t;

    if (source[i].image == NULL) return;

    tsdiff = rtp->rh_ts - source[i].laststamp;
    if (tsdiff < 0) {
	/*printf("Source %d: Old timestamp: expected %d, got %d\n", i,
	    source[i].laststamp, rtp->rh_ts);*/
	return;
    } else if (tsdiff > 0) {
	newframe = 1;
	source[i].laststamp = rtp->rh_ts;
	if (((uint32)(t.tv_sec-source[i].lastflush) >= MAX_FLUSH_TIME)  &&
	    !source[i].flushed) {
	    shownframe++;
	    VidImage_Redraw(source[i].image);
	    source[i].flushed = 1;
	    source[i].lastflush = t.tv_sec;
	}
    }

    if (len > 0) (void) decode(source[i].image, data, len);

    if (rtp->rh_sync) {
	if ((uint32)(t.tv_sec-source[i].lastflush) >= MAX_FLUSH_TIME) {
	    shownframe++;
	    VidImage_Redraw(source[i].image);
	    source[i].flushed = 1;
	    source[i].lastflush = t.tv_sec;
	}
    } else {
	source[i].flushed = 0;
    }

    avgpos = source[i].avgpos;
    seqdiff = rtp->rh_seq - source[i].startseq[avgpos];
    if ((seqdiff > MAX_LOSS) || (seqdiff < -MAX_LOSS)) {
	source[i].startseq[avgpos] = rtp->rh_seq;
	source[i].maxseq[avgpos] = rtp->rh_seq-1;
	source[i].pkts[avgpos] = 0;
	seqdiff = 0;
    }
    /*if ((int16)(rtp->rh_seq - source[i].maxseq[avgpos]) <= 0) {
	printf("Source %d: Late packet: expected %d, got %d\n", i,
	    (uint16) (source[i].maxseq[avgpos]+1), rtp->rh_seq);
    } else if ((int16)(rtp->rh_seq - source[i].maxseq[avgpos]) > 1) {
	printf("Source %d: Missing packet(s): expected %d, got %d\n", i,
	    (uint16) (source[i].maxseq[avgpos]+1), rtp->rh_seq);
    }*/
    if (seqdiff >= 0) source[i].pkts[avgpos]++;
    if ((int16) (rtp->rh_seq - source[i].maxseq[avgpos]) > 0)
	source[i].maxseq[avgpos] = rtp->rh_seq;
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
    int i, len, srcaddrlen;
    struct sockaddr_in srcaddr;
    static uint32 packet[MAX_PACKLEN/4];

    Tk_DeleteFileHandler(recvfd);

    srcaddrlen = sizeof(srcaddr);
    while ((len = recvfrom(recvfd, (char *)packet, sizeof(packet), 0,
			   (struct sockaddr *)&srcaddr, &srcaddrlen)) > 0) {
	RecvVideoData(&srcaddr, (uint8 *)packet, len);

	/* Between receiving packets catch up on other events */
	while (Tk_DoOneEvent(TK_DONT_WAIT) != 0) ;
    }

    for (i=0; i<=max_source; i++) source[i].lastflush = 0;

    Tk_CreateFileHandler(recvfd, TK_READABLE, RecvVideoPackets, NULL);
}

static void SendVideoData(int sync, int content, int timestamp,
			  uint8 *hdr, int hdrlen, uint8 *data, int datalen)
{
    int opts=0, optlen=0, len;
    static uint32 packet[MAX_PACKLEN/4];
    struct rtphdr *rtp;
    struct rtcpsdeschdr *opthdr;
    uint8 *p;
    struct timeval t;
    static uint16 seq=1;


    gettimeofday(&t, NULL);
    if (t.tv_sec-last_namexmit > NAME_XMIT_TIME) opts=1;

    rtp = (struct rtphdr *)packet;
    FillRTPHeader(rtp, chanid, opts, sync, content, seq++, timestamp);

    p = (uint8 *)(rtp+1);
    if (opts) {
	opthdr = (struct rtcpsdeschdr *)p;
        optlen = FillSDESCOption(opthdr, 1, &myaddr, myname);
	p += optlen;
	last_namexmit = t.tv_sec;
    }

    if (hdrlen > 0) memcpy(p, hdr, hdrlen);
    if (datalen > 0) memcpy(p+hdrlen, data, datalen);
    len = sizeof(struct rtphdr)+optlen+hdrlen+datalen;
    if (send(xmitfd, (char *)packet, len, 0) == -1) {
	/* Reconnect the socket just in case */
	SetupXmitFD();
    }

#ifdef IP_ADD_MEMBERSHIP
    if (IN_MULTICAST(ntohl(rmtaddr.sin_addr.s_addr)))
    {
	RecvVideoPackets(0, 0);
    } else
#endif
    {
	RecvVideoData(&myaddr, (uint8 *)packet, len);
    }
}

/*ARGSUSED*/
static void SendNextPacket(ClientData clientData)
{
    if (send_active) send_now = 1;
}

/*ARGSUSED*/
static void SendVideo(void)
{
    int timeout;
 
    if (!send_now) return;

    if ((timeout = (*encode)(source[0].image, SendVideoData)) < 0) {
	timeout = GRAB_ERROR_TIME;
    } else if (timeout < 20) {
	timeout = 0;
    }

    if (timeout > 0) {
	send_now = 0;
	send_timer = Tk_CreateTimerHandler(timeout, SendNextPacket, 0);
    }
}

/*ARGSUSED*/
static void CheckIdle(ClientData clientData)
{
    int i;
    struct timeval t;
    char cmd[256];

    gettimeofday(&t, NULL);

    /* Don't do this just yet, as it makes nv 3.2s unhappy */
    /*if (send_active && (t.tv_sec-last_namexmit > NAME_XMIT_TIME))
	SendVideoData(0, RTPCONT_NV, NULL, 0, NULL, 0);*/

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
	Tcl_Eval(interp, "destroy .");

    if (send_active && encoding) (*encoding->stop)(grabber);
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

    if (send_active && encoding) {
	(*encoding->stop)(grabber);
	encode = (*encoding->start)(grabber, max_bandwidth, max_framerate,
				    xmit_size|xmit_color);
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

    port = htons(atoi(argv[1]));
    setup_sockets = 1;
    Tk_DoWhenIdle(SetupSockets, 0);
    return TCL_OK;
}

/*ARGSUSED*/
static int ChangeChanCmd(ClientData clientData, Tcl_Interp *interp, int argc,
			 char *argv[])
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " chan\"", NULL);
	return TCL_ERROR;
    }

    chanid = atoi(argv[1]) % 64;
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
    int i, sending=send_active;

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
	Tcl_AppendResult(interp, "invalid encoding", NULL);
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
    int i, sending=send_active;
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
	Tcl_AppendResult(interp, "invalid grabber", NULL);
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
	(void) Tcl_Eval(interp,
	    ".menu.panels.m enable \"Show grabber controls\"");
    } else {
	(void) Tcl_Eval(interp, "set nvGrabPanel 0; checkPanels");
	(void) Tcl_Eval(interp,
	    ".menu.panels.m disable \"Show grabber controls\"");
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
    if (!send_active && new_send_state) {
	encode = (*encoding->start)(grabber, max_bandwidth, max_framerate,
				    xmit_size|xmit_color);
	if (encode == NULL) {
	    Tcl_AppendResult(interp, "Unable to initialize ", encoding->name,
			     " encoding on ", grabber->name, ".", NULL);
	    return TCL_ERROR;
	} else {
	    send_active = send_now = 1;
	    Tk_DeleteTimerHandler(send_timer);
	}
    } else if (send_active && !new_send_state) {
	(*encoding->stop)(grabber);

	send_active = send_now = 0;
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

    if (send_active && encoding) {
	(*encoding->stop)(grabber);
	encode = (*encoding->start)(grabber, max_bandwidth, max_framerate,
				    xmit_size|xmit_color);
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

    if (send_active && encoding) {
	(*encoding->stop)(grabber);
	encode = (*encoding->start)(grabber, max_bandwidth, max_framerate,
				    xmit_size|xmit_color);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    static char buf[256];
    char *cmd;
    struct hostent *hp;
    struct passwd *pw;
    int i, grabber_found=0;

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

    if (detach) {
	int childpid;
	childpid = fork();
	if (childpid != 0) {
	    /* Parent Process */
	    exit(0);
	}
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
	fprintf(stderr, "Usage: nv [options] address [port [chanid]]\n");
	fprintf(stderr, "   or: nv -help     to list possible options\n");
	exit(1);
    }

    (void) Tcl_VarEval(interp, "option add Nv.address \"", argv[1], "\"", NULL);

    if (argc >= 3)
	(void) Tcl_VarEval(interp, "option add Nv.port \"", argv[2], "\"",
	    NULL);
    if (argc >= 4)
	(void) Tcl_VarEval(interp, "option add Nv.chan \"", argv[3], "\"",
	    NULL);

    (void) Tcl_Eval(interp, "option add Nv.interface 0.0.0.0 widgetDefault");

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
    Tcl_CreateCommand(interp, "changeChan", ChangeChanCmd, 0, NULL);
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

    if (Tcl_VarEval(interp, TK_Init, NV_Subr, NV_UIProcs, NV_API,
	    NV_Init, NV_GrabPanels, NULL) != TCL_OK) {
	fprintf(stderr, "NV init failed:\n");
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
	(void) Tcl_VarEval(interp, "changeGrabber $nvGrabber", NULL);
	(void) Tcl_VarEval(interp, "changeEncoding $nvEncoding", NULL);
    }

    (void) Tcl_Eval(interp, "option get . maxFrameRate Nv");
    max_framerate = atoi(interp->result);
    if (max_framerate < 1) max_framerate = 1;
    if (max_framerate > 30) max_framerate = 30;

    color_ok = VidWidget_Init(Tk_Display(tkMainWin));

    if (tkMainWin != NULL) Tk_MapWindow(tkMainWin);
    (void) Tcl_Eval(interp, "update");

    signal(SIGHUP, Cleanup);
    signal(SIGINT, Cleanup);
    signal(SIGQUIT, Cleanup);
    signal(SIGPIPE, Cleanup);
    signal(SIGTERM, Cleanup);

    Tk_CreateTimerHandler(IDLE_POLL_TIME, CheckIdle, NULL);

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
