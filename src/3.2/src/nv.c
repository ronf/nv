/*
	Netvideo version 3.2
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
#include <tkInt.h>
#include "rtp.h"
#include "video.h"
#include "vidcode.h"
#include "vidgrab.h"
#include "cuseeme.h"
#include "nv.h"
#ifdef BOLTER
#include "bolter_decode.h"
#endif

#define MAX_PACKLEN	8192

extern char *inet_ntoa();

Tcl_Interp *interp;
Tk_Window tkMainWin;		/* NULL means window has been deleted. */
Visual *visual;
Colormap colmap;
int screenDepth, color_ok;

static int xmitfd = -1, recvfd = -1;
static int send_allowed=0, send_active=0, send_now=0, recv_active=0;
static int framerate, grab_width, grab_height;
static struct sockaddr_in myaddr, lcladdr, rmtaddr;
static u_char *curr_y_data;
static signed char *curr_uv_data;
static u_long last_namexmit, rtp_frame_time;
static struct timeval last_frame_time;
static char myname[MAX_NAMELEN], address[32];
static u_short port; /* in network byte order */
static double sendrate;

typedef struct {
    struct sockaddr_in	srcaddr;
    u_char		flow;
    u_char		active;
    char		name[MAX_NAMELEN];
    struct timeval	lastrecv;
    u_long		lastnotify;
    vidimage_t		*image;
    u_long		frames[8], time[8], bytes[8];
    u_long		totframes, tottime, totbytes;
    u_short		avgpos;
} source_t;

static int max_source=0;
static source_t source[MAX_SOURCES];

static int num_receivers=0;
static struct sockaddr_in receiver[MAX_RECEIVERS];

extern double atof();

extern char TK_Init[], NV_Subr[], NV_UIProcs[], NV_API[], NV_Init[];

static char *display=NULL;
static int detach=0, recvOnly=0;

Tk_ArgvInfo dispArgTable[] = {
    {"-display", TK_ARGV_STRING, NULL, (char *) &display, "Display to use"},
    {NULL, TK_ARGV_END, NULL, NULL, NULL}
};

Tk_ArgvInfo argTable[] = {
    {"-brightness", TK_ARGV_OPTION_VALUE, NULL, "Nv.brightness",
	"Default video receive brightness"},
    {"-contrast", TK_ARGV_OPTION_VALUE, NULL, "Nv.contrast",
	"Default video receive contrast"},
    {"-display", TK_ARGV_STRING, NULL, (char *) &display,
	"Display to use"},
    {"-detach", TK_ARGV_CONSTANT, (char *) 1, (char *) &detach,
	"Detach into the background"},
    {"-interface", TK_ARGV_OPTION_VALUE, NULL, "Nv.interface",
	"Network interface to transmit from"},
    {"-maxBandwidth", TK_ARGV_OPTION_VALUE, NULL, "Nv.maxBandwidth",
	"Video transmit maximum bandwidth"},
    {"-maxFrameRate", TK_ARGV_OPTION_VALUE, NULL, "Nv.maxFrameRate",
	"Video transmit maximum frame rate"},
    {"-name", TK_ARGV_OPTION_VALUE, NULL, "Nv.name",
	"Video transmit stream name"},
    {"-recvOnly", TK_ARGV_CONSTANT, (char *) 1, (char *) &recvOnly,
	"Don't try to initialize a frame grabber"},
    {"-recvSize", TK_ARGV_OPTION_VALUE, NULL, "Nv.recvSize",
	"Default video receive window size"},
    {"-title", TK_ARGV_OPTION_VALUE, NULL, "Nv.title",
	"Main window title"},
    {"-ttl", TK_ARGV_OPTION_VALUE, NULL, "Nv.ttl",
	"Video transmit multicast TTL"},
    {NULL, TK_ARGV_END, NULL, NULL, NULL}
};

/*ARGSUSED*/
static void StructureProc(ClientData clientData, XEvent *eventPtr)
{
    if (eventPtr->type == DestroyNotify) tkMainWin = NULL;
}

static void Cleanup()
{
    Tcl_VarEval(interp, "destroy .", NULL);
    GrabImage_Cleanup();
    exit(0);
}

static u_long RTPTime(void)
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

static void FillRTPHeader(struct rtphdr *hdrp, int flow, int opts, int sync,
			  int content, u_long timestamp)
{
    static u_short seq=0;

    hdrp->rh_vers = RTP_VERSION;
    hdrp->rh_flow = flow;
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
    opt->rtsh_uid = addr->sin_port;
    opt->rtsh_addr = addr->sin_addr.s_addr;
    strcpy(p, name);

    return optlen*4;
}

static int NewSource(struct sockaddr_in *addr, int flow, int color, int width,
		     int height)
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
    source[i].flow = flow;
    strcpy(source[i].name, inet_ntoa(addr->sin_addr));

    source[i].image = VidImage_Create(color, width, height);

    sprintf(cmd, "toplevel .nvsource%d", i);
    (void) Tcl_VarEval(interp, cmd, NULL);
    sprintf(cmd, ".nvsource%d", i);
    Tk_SetWindowVisual(Tk_NameToWindow(interp, cmd, tkMainWin), visual,
	screenDepth, colmap);
    sprintf(pathname, ".nvsource%d.video", i);
    (void) VidWidget_Create(interp, pathname, False, source[i].image);
    sprintf(pathname, ".sources.video%d", i);
    (void) VidWidget_Create(interp, pathname, False, source[i].image);
    sprintf(cmd, "addSource %d \"%s\"", i, source[i].name);
    (void) Tcl_VarEval(interp, cmd, NULL);

    return i;
}

static int RecvVideoOptions(int i, u_long *packet)
{
    int done;
    struct rtphdr *rtp=(struct rtphdr *)packet;
    struct rtpopthdr *opt=(struct rtpopthdr *)(rtp+1);
    struct rtcpsdeschdr *sopt;
    char cmd[256], *name;

    done = !rtp->rh_opts;
    while (!done) {
	if (opt->roh_optlen == 0) break;

	switch (opt->roh_type) {
	case RTPOPT_SDESC:
	    sopt = (struct rtcpsdeschdr *) opt;
	    name = (char *) (sopt+1);
	    if (strcmp(source[i].name, name)) {
		strncpy(source[i].name, name, sizeof(source[i].name)-1);
		source[i].name[sizeof(source[i].name)-1] = '\0';
		sprintf(cmd, "setSourceName %d \"%s\"", i, source[i].name);
		(void) Tcl_VarEval(interp, cmd, NULL);
	    }
	    break;
	}

	done = opt->roh_fin;
	opt = (struct rtpopthdr *) (((u_long *)opt)+opt->roh_optlen);
    }

    return ((u_long *)opt)-packet;
}

/*ARGSUSED*/
static void RecvNotifies(ClientData data, int mask)
{
    int i, done=0, len, srcaddrlen;
    struct sockaddr_in srcaddr;
    u_long packet[MAX_PACKLEN/4];
    static char cmd[256];
    struct rtcprevhdr *revp=(struct rtcprevhdr *)packet;
    struct rtpopthdr *opt=(struct rtpopthdr *)(revp+1);
    char *p = (char *)(opt+1);
 
    srcaddrlen = sizeof(srcaddr);
    while ((len = recvfrom(xmitfd, (char *)packet, sizeof(packet), 0, &srcaddr,
			   &srcaddrlen)) != -1) {
	for (i=0; i<num_receivers; i++)
	    if (MatchNetAddr(&receiver[i], &srcaddr)) break;

	if (i == num_receivers) {
	    if (num_receivers == MAX_RECEIVERS) continue;
	    receiver[i] = srcaddr;
	    num_receivers++;
	    Tcl_VarEval(interp, ".receivers insert end \"\"", NULL);
	}


	while (!done) {
	    if (opt->roh_optlen == 0) break;

	    switch (opt->roh_type) {
	    case RTPOPT_RAD:
		sprintf(cmd, ".receivers.list delete %d", i);
		Tcl_VarEval(interp, cmd, NULL);
		if (p[0]) {
		    sprintf(cmd, ".receivers.list insert %d \"%s (%s,%d)\"", i,
			p+1, inet_ntoa(srcaddr.sin_addr), srcaddr.sin_port);
		} else {
		    sprintf(cmd, ".receivers.list insert %d \"%s (%s,%d) (closed)\"",
			i, p+1, inet_ntoa(srcaddr.sin_addr), srcaddr.sin_port);
		}
		Tcl_VarEval(interp, cmd, NULL);
		break;
	    }

	    done = opt->roh_fin;
	    opt = (struct rtpopthdr *) (((u_long *)opt)+opt->roh_optlen);
	}
    }
}

/*ARGSUSED*/
void RecvVideoData(ClientData data, int mask)
{
    int i, j, color, width, height, len, optlen, srcaddrlen;
    void (*decode)();
    struct sockaddr_in srcaddr;
    u_long packet[MAX_PACKLEN/4];
    struct rtphdr *rtp=(struct rtphdr *)packet;
    struct timeval t;
    u_long tdiff, avgpos;
    static char cmd[256];
 
    srcaddrlen = sizeof(srcaddr);
    while ((len = recvfrom(recvfd, (char *)packet, sizeof(packet), 0, &srcaddr,
			   &srcaddrlen)) != -1) {
	if ((rtp->rh_vers != RTP_VERSION) || (rtp->rh_flow < NV_MIN_FLOW))
	    continue;

	switch (rtp->rh_content) {
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
	    decode = VidDecode;
	    break;
#ifdef BOLTER
	case RTPCONT_BOLT:
	    color = 0;
	    width = BOLTER_WIDTH;
	    height = BOLTER_HEIGHT;
	    decode = Bolter_Decode;
	    break;
#endif
	default:
	    /* Unknown packet content type -- discard */
	    continue;
	}

	for (i=0, j=0; i<=max_source; i++) {
	    if (source[i].image == NULL) continue;
	    if ((MatchNetAddr(&srcaddr, &source[i].srcaddr)) &&
		(rtp->rh_flow == source[i].flow)) break;
	    if ((j == 0)  &&
		(srcaddr.sin_addr.s_addr==source[i].srcaddr.sin_addr.s_addr) &&
		(source[i].lastrecv.tv_sec == 0)) j = i;
	}
 
	if (i > max_source) {
	    if (j != 0) {
		i = j;
		source[i].srcaddr = srcaddr;
		source[i].flow = rtp->rh_flow;
	    } else {
		i = NewSource(&srcaddr, rtp->rh_flow, color, width, height);
	    }
	}
 
	if (source[i].lastrecv.tv_sec == 0) {
	    sprintf(cmd, "setSourceName %d \"%s\"", i, source[i].name);
	    (void) Tcl_VarEval(interp, cmd, NULL);
	}
 
	gettimeofday(&t, NULL);
	tdiff = (t.tv_sec-source[i].lastrecv.tv_sec)*1000000 +
	    t.tv_usec-source[i].lastrecv.tv_usec;
	source[i].lastrecv = t;

	optlen = RecvVideoOptions(i, packet);
	if (source[i].image == NULL) continue;

	decode(source[i].image, (u_char *)(packet+optlen), len-4*optlen);
	if (rtp->rh_sync) VidImage_Redraw(source[i].image);
 
	avgpos = source[i].avgpos;
	if (rtp->rh_sync) source[i].frames[avgpos]++;
	source[i].time[avgpos] += tdiff;
	source[i].bytes[avgpos] += len;

	if (source[i].time[avgpos] >= 250000) {
	    source[i].totframes += source[i].frames[avgpos];
	    source[i].tottime += source[i].time[avgpos];
	    source[i].totbytes += source[i].bytes[avgpos];
	    source[i].avgpos = avgpos = (avgpos+1) % 8;

	    if (source[i].time[avgpos] != 0) {
		sprintf(cmd, "setFPSandBPS %d %.1f %.1f", i,
		    1000000.*source[i].totframes/source[i].tottime,
		    8000.*source[i].totbytes/source[i].tottime);
		(void) Tcl_VarEval(interp, cmd, NULL);

		source[i].totframes -= source[i].frames[avgpos];
		source[i].tottime -= source[i].time[avgpos];
		source[i].totbytes -= source[i].bytes[avgpos];

		source[i].frames[avgpos] = 0;
		source[i].time[avgpos] = 0;
		source[i].bytes[avgpos] = 0;
	    }
	}
    }
}

static void SendNotify(int i)
{
    u_long packet[MAX_PACKLEN/4];
    struct rtcprevhdr *revp=(struct rtcprevhdr *)packet;
    struct rtpopthdr *opt=(struct rtpopthdr *)(revp+1);
    char *p=(char *) (opt+1);
    int optlen=(sizeof(*opt)+strlen(myname)+1)/4+1;

    revp->rtrh_flow = source[i].flow;
    revp->rtrh_x1 = revp->rtrh_x2 = revp->rtrh_x3 = 0;
    opt->roh_fin = 1;
    opt->roh_type = RTPOPT_RAD;
    opt->roh_optlen = optlen;
    p[0] = source[i].active;
    strcpy(p+1, myname);

    sendto(xmitfd, (char *)packet, sizeof(*revp)+optlen*4, 0,
	&source[i].srcaddr, sizeof(source[i].srcaddr));
    source[i].lastnotify = time(0);
}

static void SendVideoData(u_char *hdr, int hdrlen, u_char *data, int len,
			  int end)
{
    int opts, t=time(0);
    static struct rtphdr rtp;
    static struct {
	struct rtcpsdeschdr hdr;
	char name[MAX_NAMELEN];
    } namebuf;
    static struct iovec iov[4] =
	 { { (char *)&rtp, sizeof(rtp) },
	   { (char *)&namebuf, 0 },
	   { 0, 0 },
	   { 0, 0 } };
    static struct msghdr msg = { 0, sizeof(struct sockaddr_in), iov, 4, 0, 0 };

    if (t-last_namexmit > NAME_XMIT_TIME) {
	opts = 1;
	iov[1].iov_len = FillSDESCOption(&namebuf.hdr, 1, &myaddr, myname);
	last_namexmit = t;
    } else {
	opts = 0;
	iov[1].iov_len = 0;
    }

    FillRTPHeader(&rtp, NV_MIN_FLOW, opts, end, RTPCONT_NV, rtp_frame_time);

    iov[2].iov_base = (char *) hdr;
    iov[2].iov_len = hdrlen;

    iov[3].iov_base = (char *) data;
    iov[3].iov_len = len;

    msg.msg_name = (char *) &rmtaddr;
    sendmsg(xmitfd, &msg, 0);

#ifdef IP_ADD_MEMBERSHIP
    if (!IN_MULTICAST(ntohl(rmtaddr.sin_addr.s_addr)))
#endif
    {
	msg.msg_name = (char *) &lcladdr;
	sendmsg(xmitfd, &msg, 0);
    }

    RecvVideoData(0, 0);
}

/*ARGSUSED*/
static void SetSendNow(ClientData data)
{
    gettimeofday(&last_frame_time, NULL);
    send_now = 1;
}

static void SendVideoFrame(void)
{
    static long tdiff=EST_FRAME_TIME, twait;
    int bytes, aging_bytes;
    u_char *xmitted_image;
    struct timeval frame_time;
 
    send_now = 0;
    if (!send_active) return;

    rtp_frame_time = RTPTime();
    if (GrabImage(curr_y_data, curr_uv_data)) {
	xmitted_image = source[0].image? source[0].image->y_data : NULL;
	aging_bytes = sendrate*tdiff/64; /* 1/8 of sendrate */
	bytes = VidEncode(xmitted_image, curr_y_data, curr_uv_data,
			  MAX_VID_PACKLEN, aging_bytes, SendVideoData);

	gettimeofday(&frame_time, NULL);
	tdiff = (frame_time.tv_sec-last_frame_time.tv_sec)*1000 +
	    (frame_time.tv_usec-last_frame_time.tv_usec)/1000;
	last_frame_time = frame_time;

	if ((twait = bytes*8/sendrate-tdiff) > 0) {
	    Tk_CreateTimerHandler(twait, SetSendNow, NULL);
	} else {
	    send_now = 1;
	}
    } else {
	Tk_CreateTimerHandler(GRAB_ERROR_TIME, SetSendNow, NULL);
    }
}

/*ARGSUSED*/
static void CheckIdle(ClientData data)
{
    int i;
    u_long t=time(0);
    char cmd[256];

    for (i=0; i<=max_source; i++) {
	if (source[i].image == NULL) continue;

	if (source[i].active) {
	    if ((source[i].lastrecv.tv_sec != 0) &&
		(t-source[i].lastrecv.tv_sec > MAXIDLE_TIME)) {
		source[i].lastrecv.tv_sec = 0;
		sprintf(cmd, "setSourceName %d \"%s (signal lost)\"", i,
		    source[i].name);
		Tcl_VarEval(interp, cmd, NULL);
	    }
	} else {
	    if ((source[i].lastrecv.tv_sec == 0) ||
		(t-source[i].lastrecv.tv_sec > MAXIDLE_TIME)) {
		sprintf(cmd, "deleteSource %d", i);
		Tcl_VarEval(interp, cmd, NULL);
		VidImage_Destroy(source[i].image);
		source[i].image = NULL;
		continue;
	    }
	}

	if (t-source[i].lastnotify > NOTIFY_TIME) {
	    if (source[i].active) SendNotify(i);
	}
    }

    Tk_CreateTimerHandler(IDLEPOLL_TIME*1000, CheckIdle, NULL);
}

static void OpenXmitFD(void)
{
    int addrlen;
    struct hostent *hp;

    if ((xmitfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	perror("xmitfd: socket");

    myaddr.sin_family = AF_INET;
    (void) Tcl_VarEval(interp, "option get . interface Nv", NULL);
    hp = gethostbyname(interp->result);
    if (hp == NULL) {
	fprintf(stderr, "Can't resolve host %s\n", interp->result);
    } else {
	memcpy((char *) &myaddr.sin_addr, hp->h_addr, hp->h_length);
    }
    myaddr.sin_port = 0;

    if (bind(xmitfd, &myaddr, sizeof(myaddr)) == -1)
	perror("xmitfd: bind");

    addrlen = sizeof(myaddr);
    getsockname(xmitfd, &myaddr, &addrlen);

    fcntl(xmitfd, F_SETFL, fcntl(xmitfd, F_GETFL)|O_NDELAY);
    Tk_CreateFileHandler(xmitfd, TK_READABLE, RecvNotifies, NULL);
}

static void OpenRecvFD(void)
{
    int one=1, err;
    struct hostent *hp;
#ifdef IP_ADD_MEMBERSHIP
    struct ip_mreq mreq;
#endif

    if (recvfd != -1) {
	Tk_DeleteFileHandler(recvfd);
	close(recvfd);
    }

    rmtaddr.sin_family = AF_INET;
    rmtaddr.sin_addr.s_addr = inet_addr(address);
    if (rmtaddr.sin_addr.s_addr == -1) {
	hp = gethostbyname(address);
	if (hp == NULL)
	    fprintf(stderr, "Can't resolve host %s\n", address);
	else
	    bcopy(hp->h_addr, (char *) &rmtaddr.sin_addr, hp->h_length);
    }
    rmtaddr.sin_port = port;

    if (!recv_active) return;

    if ((recvfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	perror("socket");

    if (setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1)
	perror("reuseaddr");

    lcladdr = rmtaddr;
#ifdef IP_ADD_MEMBERSHIP
    if (IN_MULTICAST(ntohl(rmtaddr.sin_addr.s_addr))) {
	if (bind(recvfd, &lcladdr, sizeof(lcladdr)) == -1) {
	    lcladdr.sin_addr.s_addr = INADDR_ANY;
	    err = bind(recvfd, &lcladdr, sizeof(lcladdr));
	}
    } else
#endif
    {
	lcladdr.sin_addr.s_addr = INADDR_ANY;
	err = bind(recvfd, &lcladdr, sizeof(lcladdr));
    }

    if (err == -1)
	perror("recvfd: bind");

#ifdef IP_ADD_MEMBERSHIP
    if (IN_MULTICAST(ntohl(rmtaddr.sin_addr.s_addr))) {
	mreq.imr_multiaddr = rmtaddr.sin_addr;
	mreq.imr_interface.s_addr = INADDR_ANY;
	if (setsockopt(recvfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
		       sizeof(mreq)) == -1)
	    perror("recvfd: ip_add_membership");
    }
#endif

    lcladdr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    fcntl(recvfd, F_SETFL, fcntl(recvfd, F_GETFL)|O_NDELAY);

    Tk_CreateFileHandler(recvfd, TK_READABLE, RecvVideoData, NULL);
}

/*ARGSUSED*/
static int ShutdownCmd(ClientData clientData, Tcl_Interp *interp, int argc,
		       char *argv[])
{
    int i;

    for (i=0; i<=max_source; i++) {
	if (source[i].image == NULL) continue;
	if (source[i].active) {
	    source[i].active = 0;
	    SendNotify(i);
	}
    }

    Cleanup();
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

    sendrate = atof(argv[1]);
    send_now = 1;
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
    OpenRecvFD();
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
    OpenRecvFD();
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
    u_char ttl;

    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " ttl\"", NULL);
	return TCL_ERROR;
    }

#ifdef IP_ADD_MEMBERSHIP
    ttl = atoi(argv[1]);
    if (IN_MULTICAST(ntohl(rmtaddr.sin_addr.s_addr))) {
	if (setsockopt(xmitfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
		       sizeof(ttl)) == -1) {
	    perror("ip_multicast_ttl");
	    exit(1);
	}
    }
#endif

    return TCL_OK;
}

/*ARGSUSED*/
static int SendColorCmd(ClientData clientData, Tcl_Interp *interp, int argc,
			char *argv[])
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " {0|1}\"", NULL);
	return TCL_ERROR;
    }

    if (atoi(argv[1]) && (curr_uv_data == 0)) {
	curr_uv_data = (signed char *) malloc(grab_height*grab_width);
	(void) Tcl_VarEval(interp,
	    ".sendControls.color.grey config -state normal", NULL);
	(void) Tcl_VarEval(interp,
	    ".sendControls.color.color config -state disabled", NULL);
    } else if ((atoi(argv[1]) == 0) && curr_uv_data) {
	free(curr_uv_data);
	curr_uv_data = 0;
	(void) Tcl_VarEval(interp,
	    ".sendControls.color.grey config -state disabled", NULL);
	(void) Tcl_VarEval(interp,
	    ".sendControls.color.color config -state normal", NULL);
    }

    return TCL_OK;
}

/*ARGSUSED*/
static int SendVideoCmd(ClientData clientData, Tcl_Interp *interp, int argc,
			char *argv[])
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " {0|1}\"", NULL);
	return TCL_ERROR;
    }

    send_now = send_active = atoi(argv[1]);
    if (send_active) {
	VidEncode_Reset();
	gettimeofday(&last_frame_time, NULL);
    }

    return TCL_OK;
}

/*ARGSUSED*/
static int ReceiveVideoCmd(ClientData clientData, Tcl_Interp *interp, int argc,
			   char *argv[])
{
    int i, active;

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

    active = atoi(argv[2]);
    if (source[i].active != active) {
	source[i].active = active;
	SendNotify(i);
    }
    return TCL_OK;
}

int main(int argc, char *argv[])
{
    char buf[256];
    struct hostent *hp;
    struct passwd *pw;

    interp = Tcl_CreateInterp();
    (void) Tk_ParseArgv(interp, NULL, &argc, argv, dispArgTable,
	TK_ARGV_NO_DEFAULTS);

    tkMainWin = Tk_CreateMainWindow(interp, display, "nv");
    if (tkMainWin == NULL) {
	fprintf(stderr, "%s\n", interp->result);
	exit(1);
    }

    Tk_SetClass(tkMainWin, "Nv");
    Tk_CreateEventHandler(tkMainWin, StructureNotifyMask, StructureProc, NULL);

    VidWidget_Init();
    Tk_SetWindowVisual(tkMainWin, visual, screenDepth, colmap);

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

    (void) Tcl_VarEval(interp, "option get . maxFrameRate Nv", NULL);
    framerate = atoi(interp->result);

    if (!recvOnly)
	send_allowed = GrabImage_Init(framerate, &grab_width, &grab_height);

    if (send_allowed) {
	curr_y_data = (u_char *) malloc(grab_height*grab_width);
	curr_uv_data = (signed char *) malloc(grab_height*grab_width);
	VidEncode_Init(grab_width, grab_height);
    }

    (void) Tcl_VarEval(interp, "option add Nv.address \"", NV_DEFAULT_ADDR,
	"\" startupFile", NULL);
    (void) Tcl_VarEval(interp, "option add Nv.port \"", NV_DEFAULT_PORT,
	"\" startupFile", NULL);

    sprintf(buf, "%d", send_allowed);
    Tcl_SetVar(interp, "send_allowed", buf, TCL_GLOBAL_ONLY);

    if (argc == 2) {
	(void) Tcl_VarEval(interp, "option add Nv.address \"", argv[1],
	    "\"", NULL);
    } else if (argc == 3) {
	(void) Tcl_VarEval(interp, "option add Nv.address \"", argv[1],
	    "\"", NULL);
	(void) Tcl_VarEval(interp, "option add Nv.port \"", argv[2],
	    "\"", NULL);
    }

    gethostname(buf, sizeof(buf)-1);
    if ((hp = gethostbyname(buf)) != NULL)
	    strncpy(buf, hp->h_name, sizeof(buf)-1);
    (void) Tcl_VarEval(interp, "option add Nv.interface \"", buf,
	"\" widgetDefault", NULL);

    if ((pw = getpwuid(getuid())) == NULL) {
	strcpy(myname, "nobody");
    } else {
	strcpy(myname, pw->pw_name);
    }

    strcat(myname, "@");

    (void) Tcl_VarEval(interp, "option get . interface Nv", NULL);
    strncpy(myname+strlen(myname), interp->result,
	sizeof(myname)-strlen(myname)-1);
    myname[sizeof(myname)-1] = '\0';

    OpenXmitFD();

    Tcl_CreateCommand(interp, "shutdown", ShutdownCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeBrightness", ChangeBrightnessCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeContrast", ChangeContrastCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeMaxBandwidth", ChangeMaxBandwidthCmd, 0,
		      NULL);
    Tcl_CreateCommand(interp, "changeAddress", ChangeAddressCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changePort", ChangePortCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeName", ChangeNameCmd, 0, NULL);
    Tcl_CreateCommand(interp, "changeTTL", ChangeTTLCmd, 0, NULL);
    Tcl_CreateCommand(interp, "sendColor", SendColorCmd, 0, NULL);
    Tcl_CreateCommand(interp, "sendVideo", SendVideoCmd, 0, NULL);
    Tcl_CreateCommand(interp, "receiveVideo", ReceiveVideoCmd, 0, NULL);

    (void) Tcl_VarEval(interp, "option add Nv.name \"", myname,
	"\" widgetDefault", NULL);

    if (Tcl_VarEval(interp, TK_Init, NV_Subr, NV_UIProcs, NV_API, NV_Init, NULL)
	    != TCL_OK) {
	fprintf(stderr, "NV init failed:\n");
	fprintf(stderr, "%s\n", interp->result);
	exit(1);
    }

    if (tkMainWin != NULL) Tk_MapWindow(tkMainWin);
    (void) Tcl_VarEval(interp, "update", NULL);

    signal(SIGHUP, Cleanup);
    signal(SIGINT, Cleanup);
    signal(SIGQUIT, Cleanup);
    signal(SIGPIPE, Cleanup);
    signal(SIGTERM, Cleanup);

    Tk_CreateTimerHandler(IDLEPOLL_TIME*1000, CheckIdle, NULL);

    recv_active = 1;
    OpenRecvFD();

    while (tk_NumMainWindows > 0) {
	if (send_now) {
	    SendVideoFrame();
	    while (tk_NumMainWindows > 0)
		if (Tk_DoOneEvent(1) == 0) break;
	} else {
	    (void) Tk_DoOneEvent(0);
	}
    }

    Tcl_VarEval(interp, "shutdown", NULL);
    exit(0);
    /*NOTREACHED*/
}
