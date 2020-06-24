/*
	Netvideo version 2.7
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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <tkInt.h>
#include "video.h"
#include "vidcode.h"
#include "vidgrab.h"
#include "nv.h"
#ifdef BOLTER
#include "bolter_decode.h"
#endif BOLTER

#define MAX_PACKLEN	8192

Tcl_Interp *interp;
Tk_Window w;			/* NULL means window has been deleted. */

static int xmitfd = -1, recvfd = -1;
static int send_allowed, send_active=0, recv_active=0;
static unsigned char ttl;
static struct sockaddr_in myaddr, lcladdr, rmtaddr;
static unsigned char *curr_image;
static struct timeval last_frame_time;
static char myname[MAX_NAMELEN], address[32];
static unsigned short port;
static double sendrate;

typedef struct {
    struct sockaddr_in	srcaddr;
    char		name[MAX_NAMELEN];
    struct timeval	lastrecv;
    unsigned long	lastnotify;
    vidimage_t		*image;
    unsigned long	frames[8], time[8], bytes[8];
    unsigned long	totframes, tottime, totbytes;
    unsigned short	avgpos;
} source_t;

static int max_source=0;
static source_t source[MAX_SOURCES];

static int num_receivers=0;
static struct sockaddr_in receiver[MAX_RECEIVERS];

extern double atof();

extern char InitTK[], InitNVProcs[], InitNV[];

static char *display=NULL;
static int bolter=0, detach=0;

Tk_ArgvInfo dispArgTable[] = {
    {"-display", TK_ARGV_STRING, NULL, (char *) &display, "Display to use"},
    {NULL, TK_ARGV_END, NULL, NULL, NULL}
};

Tk_ArgvInfo argTable[] = {
#ifdef BOLTER
    {"-bolter", TK_ARGV_CONSTANT, (char *) 1, (char *) &bolter,
	"Enable Bolter format decoder" },
#endif BOLTER
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
    {"-recvSize", TK_ARGV_OPTION_VALUE, NULL, "Nv.recvSize",
	"Default video receive window size"},
    {"-ttl", TK_ARGV_OPTION_VALUE, NULL, "Nv.ttl",
	"Video transmit multicast TTL"},
    {NULL, TK_ARGV_END, NULL, NULL, NULL}
};

static void StructureProc(clientData, eventPtr)
    ClientData clientData;
    XEvent *eventPtr;
{
    if (eventPtr->type == DestroyNotify) w = NULL;
}

static void Cleanup()
{
    Tcl_VarEval(interp, "destroy .", NULL);
    GrabImage_Cleanup();
    exit(0);
}

static int source_open(i)
    int i;
{
    static char buf[32];

    sprintf(buf, "source%d", i);
    return atoi(Tcl_GetVar(interp, buf, TCL_GLOBAL_ONLY));
}

static int MatchNetAddr(addr1, addr2)
    struct sockaddr_in *addr1, *addr2;
{
    return ((addr1->sin_addr.s_addr == addr2->sin_addr.s_addr) &&
	    (addr1->sin_port == addr2->sin_port));
}

static int NewSource(addr, fmt)
    struct sockaddr_in *addr;
    int fmt;
{
    int i, width, height;
    char pathname[32], cmd[256];
 
    if (MatchNetAddr(addr, &myaddr)) {
	i = 0;
    } else {
	for (i=1; i<=max_source; i++)
	    if (source[i].image == NULL) break;
    }
 
    if (i > max_source) max_source = i;
 
    bzero(&source[i], sizeof(source[i]));
    source[i].srcaddr = *addr;
    strcpy(source[i].name, inet_ntoa(addr->sin_addr));

    if (!bolter) {
	switch (fmt) {
	case VIDCODE_NTSC:
	    width = NTSC_WIDTH;
	    height = NTSC_HEIGHT;
	    break;
	case VIDCODE_PAL:
	    width = PAL_WIDTH;
	    height = PAL_HEIGHT;
	    break;
	}
    } else {
	width = BOLTER_WIDTH;
	height = BOLTER_HEIGHT;
    }
    source[i].image = VidImage_Create(width, height);

    sprintf(cmd, "toplevel .nvsource%d", i);
    (void) Tcl_VarEval(interp, cmd, NULL);
    sprintf(pathname, ".nvsource%d.video", i);
    (void) VidWidget_Create(interp, pathname, False, source[i].image);
    sprintf(cmd, "addSource %d \"%s\"", i, source[i].name);
    (void) Tcl_VarEval(interp, cmd, NULL);

    return i;
}

static void RecvNotifies(data, mask)
    ClientData data;
    int mask;
{
    int i, len, srcaddrlen;
    struct sockaddr_in srcaddr;
    unsigned char packet[MAX_PACKLEN], cmd[256];
    vidcode_hdr_t *hdr = (vidcode_hdr_t *)packet;
 
    srcaddrlen = sizeof(srcaddr);
    while ((len = recvfrom(xmitfd, packet, sizeof(packet), 0, &srcaddr,
			   &srcaddrlen)) != -1) {
	for (i=0; i<num_receivers; i++)
	    if (MatchNetAddr(&receiver[i], &srcaddr)) break;

	if (i == num_receivers) {
	    if (num_receivers == MAX_RECEIVERS) continue;
	    receiver[i] = srcaddr;
	    num_receivers++;
	    Tcl_VarEval(interp, ".receivers insert end \"\"", NULL);
	}

	sprintf(cmd, ".receivers.list delete %d", i);
	Tcl_VarEval(interp, cmd, NULL);
	if (packet[0]) {
	    sprintf(cmd, ".receivers.list insert %d \"%s (%s,%d)\"", i,
		packet+1, inet_ntoa(srcaddr.sin_addr), srcaddr.sin_port);
	} else {
	    sprintf(cmd, ".receivers.list insert %d \"%s (%s,%d) (closed)\"",
		i, packet+1, inet_ntoa(srcaddr.sin_addr), srcaddr.sin_port);
	}
	Tcl_VarEval(interp, cmd, NULL);
    }
}

void RecvVideoData(data, mask)
    ClientData data;
    int mask;
{
    int i, j, end, len, maxnamelen, srcaddrlen;
    struct sockaddr_in srcaddr;
    unsigned char packet[MAX_PACKLEN], buf[16], *packp;
    struct timeval t;
    unsigned long tdiff, avgpos;
    static char cmd[256];
 
    srcaddrlen = sizeof(srcaddr);
    while ((len = recvfrom(recvfd, packet, sizeof(packet), 0, &srcaddr,
			   &srcaddrlen)) != -1) {
	for (i=0; i<=max_source; i++) {
	    if (source[i].image == NULL) continue;
	    if (MatchNetAddr(&srcaddr, &source[i].srcaddr)) break;
	}
 
	if (i > max_source) {
	    int fmt = ((vidcode_hdr_t *) packet)->fmt;
	    i = NewSource(&srcaddr, fmt);
	}
 
	if (source[i].lastrecv.tv_sec == 0) {
	    sprintf(buf, "%d", i);
	    (void) Tcl_VarEval(interp, "setSourceName ", buf, " \"",
		source[i].name, "\"", NULL);
	}
 
	gettimeofday(&t, NULL);
	tdiff = (t.tv_sec-source[i].lastrecv.tv_sec)*1000000 +
	    t.tv_usec-source[i].lastrecv.tv_usec;
	source[i].lastrecv = t;

	if (!bolter) {
	    /* HACK: This will be replaced by RTP options parsing */
	    packp = packet;
	    if (((vidcode_hdr_t *)packet)->type == VIDCODE_NAME) {
		int maxnamelen = (len > MAX_NAMELEN) ? MAX_NAMELEN : len;

		packp += VIDCODE_HDR_LEN;
		len -= VIDCODE_HDR_LEN;
		for (j=0; j<maxnamelen; j++)
		    if (packp[j] == '\0') break;
		if (j == maxnamelen) continue;
		j++;
		if (j & 3) j += 4-(j & 3);

		if (strcmp(source[i].name, packp) != 0) {
		    strcpy(source[i].name, packp);
		    sprintf(cmd, "setSourceName %d \"%s\"", i, source[i].name);
		    (void) Tcl_VarEval(interp, cmd, NULL);
		}

		packp += j;
		len -= j;
	    }

	    if (source[i].image == NULL) continue;
	    end = VidDecode(source[i].image, packp, len);
	} else {
	    if (source[i].image == NULL) continue;
#ifdef BOLTER
	    (void) Bolter_Decode(source[i].image, packet+8, len-8);
	    (void) VidImage_Redraw(source[i].image);
#endif BOLTER
	    end = True;
	}
 
	avgpos = source[i].avgpos;
	if (end) source[i].frames[avgpos]++;
	source[i].time[avgpos] += tdiff;
	source[i].bytes[avgpos] += len;

	if (source[i].time[avgpos] >= 250000) {
	    source[i].totframes += source[i].frames[avgpos];
	    source[i].tottime += source[i].time[avgpos];
	    source[i].totbytes += source[i].bytes[avgpos];
	    source[i].avgpos = avgpos = (avgpos+1) % 8;

	    if (source[i].time[avgpos] != 0) {
		sprintf(cmd, "setFPSandBPS %d %.1lf %.1lf", i,
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

static void SendNotify(i, onoff)
    int i, onoff;
{
    char packet[MAX_PACKLEN];

    if (bolter) return;

    packet[0] = onoff;
    strcpy(&packet[1], myname);
    sendto(xmitfd, packet, strlen(myname)+2, 0, &source[i].srcaddr,
	sizeof(source[i].srcaddr));
    source[i].lastnotify = time(0);
}

static void SendVideoData(packet, len)
    char *packet;
    int len;
{
    sendto(xmitfd, packet, len, 0, &rmtaddr, sizeof(rmtaddr));

#ifdef IP_ADD_MEMBERSHIP
    if (!IN_MULTICAST(rmtaddr.sin_addr.s_addr))
#endif IP_ADD_MEMBERSHIP
	sendto(xmitfd, packet, len, 0, &lcladdr, sizeof(lcladdr));

    RecvVideoData(0, 0);
}

static void SendVideoFrame()
{
    register unsigned long tdiff;
    int max_bytes;
    unsigned char *xmitted_image;
    struct timeval frame_time;
    static char packet[MAX_PACKLEN];
    static int frame_count=NAME_XMITFREQ;
 
    if (GrabImage(curr_image)) {
	gettimeofday(&frame_time, NULL);
	tdiff = (frame_time.tv_sec-last_frame_time.tv_sec)*1000000 +
	    frame_time.tv_usec-last_frame_time.tv_usec;
	last_frame_time = frame_time;

	/* HACK: This will be replaced by the addition of an RTP option */
	if (frame_count++ == NAME_XMITFREQ) {
	    vidcode_hdr_t *hdr=(vidcode_hdr_t *)packet;
	    extern int vidcode_fmt;
	    int len;

	    frame_count = 0;
	    hdr->protvers = VIDCODE_VERSION;
	    hdr->fmt = vidcode_fmt;
	    hdr->type = VIDCODE_NAME;
	    hdr->x = hdr->y = hdr->initpix = 0;
	    strcpy(packet+VIDCODE_HDR_LEN, myname);
	    len = VIDCODE_HDR_LEN+strlen(myname)+1;
	    if (len & 3) len += 4-(len & 3);
	    SendVideoData(packet, len);
	}

	xmitted_image = source[0].image? source[0].image->data : NULL;
	max_bytes = sendrate*tdiff/8000.;
	(void) VidEncode(xmitted_image, curr_image, packet, MAX_VID_PACKLEN,
			 max_bytes, max_bytes/8, SendVideoData);
    }
}

static void CheckIdle(data)
    ClientData data;
{
    int i;
    unsigned long t=time(0);
    char cmd[256];

    for (i=0; i<=max_source; i++) {
	if (source[i].image == NULL) continue;

	if (source_open(i)) {
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
	    if (source_open(i)) SendNotify(i, True);
	}
    }

    Tk_CreateTimerHandler(IDLEPOLL_TIME*1000, CheckIdle, (ClientData) NULL);
}

static void OpenXmitFD()
{
    int one=1, addrlen;
    struct hostent *hp;

    if ((xmitfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	perror("xmitfd: socket");

    myaddr.sin_family = AF_INET;
    (void) Tcl_VarEval(interp, "option get . interface Nv", NULL);
    hp = gethostbyname(interp->result);
    if (hp == NULL) {
	fprintf(stderr, "Can't resolve host %s\n", interp->result);
    } else {
	bcopy(hp->h_addr, (char *) &myaddr.sin_addr, hp->h_length);
    }
    myaddr.sin_port = 0;

    if (bind(xmitfd, &myaddr, sizeof(myaddr)) == -1)
	perror("xmitfd: bind");

    addrlen = sizeof(myaddr);
    getsockname(xmitfd, &myaddr, &addrlen);

    ioctl(xmitfd, FIONBIO, &one);
    Tk_CreateFileHandler(xmitfd, TK_READABLE, RecvNotifies, (ClientData) NULL);
}

static void OpenRecvFD()
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
    if (IN_MULTICAST(rmtaddr.sin_addr.s_addr)) {
	if (bind(recvfd, &lcladdr, sizeof(lcladdr)) == -1) {
	    lcladdr.sin_addr.s_addr = INADDR_ANY;
	    err = bind(recvfd, &lcladdr, sizeof(lcladdr));
	}
    } else
#endif IP_ADD_MEMBERSHIP
    {
	lcladdr.sin_addr.s_addr = INADDR_ANY;
	err = bind(recvfd, &lcladdr, sizeof(lcladdr));
    }

    if (err == -1)
	perror("recvfd: bind");

#ifdef IP_ADD_MEMBERSHIP
    if (IN_MULTICAST(rmtaddr.sin_addr.s_addr)) {
	mreq.imr_multiaddr = rmtaddr.sin_addr;
	mreq.imr_interface.s_addr = INADDR_ANY;
	if (setsockopt(recvfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
		       sizeof(mreq)) == -1)
	    perror("recvfd: ip_add_membership");
    }
#endif

    lcladdr.sin_addr.s_addr = INADDR_LOOPBACK;
    ioctl(recvfd, FIONBIO, &one);

    Tk_CreateFileHandler(recvfd, TK_READABLE, RecvVideoData, (ClientData) NULL);
}

static int ShutdownCmd(tkwin, interp, argc, argv)
    Tk_Window tkwin;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    int i;
    char buf[32];

    for (i=0; i<=max_source; i++) {
	if (source[i].image == NULL) continue;
	if (source_open(i)) SendNotify(i, False);
    }

    Cleanup();
}

static int SetBrightnessCmd(tkwin, interp, argc, argv)
    Tk_Window tkwin;
    Tcl_Interp *interp;
    int argc;
    char **argv;
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

static int SetContrastCmd(tkwin, interp, argc, argv)
    Tk_Window tkwin;
    Tcl_Interp *interp;
    int argc;
    char **argv;
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

static int SetBandwidthCmd(tkwin, interp, argc, argv)
    Tk_Window tkwin;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " bandwidth\"", NULL);
	return TCL_ERROR;
    }

    sendrate = atof(argv[1]);
    return TCL_OK;
}

static int SetAddressCmd(tkwin, interp, argc, argv)
    Tk_Window tkwin;
    Tcl_Interp *interp;
    int argc;
    char **argv;
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

static int SetPortCmd(tkwin, interp, argc, argv)
    Tk_Window tkwin;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " port\"", NULL);
	return TCL_ERROR;
    }

    port = atoi(argv[1]);
    OpenRecvFD();
    return TCL_OK;
}

static int SetNameCmd(tkwin, interp, argc, argv)
    Tk_Window tkwin;
    Tcl_Interp *interp;
    int argc;
    char **argv;
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

static int SetTTLCmd(tkwin, interp, argc, argv)
    Tk_Window tkwin;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " ttl\"", NULL);
	return TCL_ERROR;
    }

#ifdef IP_ADD_MEMBERSHIP
    ttl = atoi(argv[1]);
    if (IN_MULTICAST(rmtaddr.sin_addr.s_addr)) {
	if (setsockopt(xmitfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
		       sizeof(ttl)) == -1) {
	    perror("ip_multicast_ttl");
	    exit(1);
	}
    }
#endif

    return TCL_OK;
}

static int SendVideoCmd(tkwin, interp, argc, argv)
    Tk_Window tkwin;
    Tcl_Interp *interp;
    int argc;
    char **argv;
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " {0|1}\"", NULL);
	return TCL_ERROR;
    }

    send_active = atoi(argv[1]);
    gettimeofday(&last_frame_time, NULL);
    last_frame_time.tv_usec -= EST_FRAME_TIME;
    return TCL_OK;
}

static int ReceiveVideoCmd(tkwin, interp, argc, argv)
    Tk_Window tkwin;
    Tcl_Interp *interp;
    int argc;
    char **argv;
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

    SendNotify(i, atoi(argv[2]));
    return TCL_OK;
}

int main(argc, argv)
    int argc;
    char **argv;
{
    char *args, *p;
    char buf[256];
    int framerate, width, height, result;
    struct hostent *hp;
    struct passwd *pw;

    interp = Tcl_CreateInterp();
    (void) Tk_ParseArgv(interp, NULL, &argc, argv, dispArgTable,
	TK_ARGV_NO_DEFAULTS);

    w = Tk_CreateMainWindow(interp, display, "nv");
    if (w == NULL) {
	fprintf(stderr, "%s\n", interp->result);
	exit(1);
    }

    Tk_SetClass(w, "Nv");
    Tk_CreateEventHandler(w, StructureNotifyMask, StructureProc,
	    (ClientData) NULL);

    if (Tk_ParseArgv(interp, w, &argc, argv, argTable, 0) != TCL_OK) {
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

    if (!bolter) {
	(void) Tcl_VarEval(interp, "option get . maxFrameRate Nv", NULL);
	framerate = atoi(interp->result);

	send_allowed = GrabImage_Init(framerate, &width, &height);
	if (send_allowed) {
	    curr_image = (unsigned char *) malloc(height*width);
	    VidEncode_Init(width, height);
	}
	(void) Tcl_VarEval(interp, "option add Nv.address \"", NV_DEFAULT_ADDR,
	    "\" startupFile", NULL);
	(void) Tcl_VarEval(interp, "option add Nv.port \"", NV_DEFAULT_PORT,
	    "\" startupFile", NULL);
    } else {
	send_allowed = 0;
	(void) Tcl_VarEval(interp, "option add Nv.address \"", NV_BOLTER_ADDR,
	    "\" startupFile", NULL);
	(void) Tcl_VarEval(interp, "option add Nv.port \"", NV_BOLTER_PORT,
	    "\" startupFile", NULL);
    }

    VidDecode_Init();
    VidImage_Init();
    VidWidget_Init(w);

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

    Tcl_CreateCommand(interp, "shutdown", ShutdownCmd, (ClientData) 0,
	(void (*)()) NULL);
    Tcl_CreateCommand(interp, "setBrightness", SetBrightnessCmd, (ClientData) 0,
	(void (*)()) NULL);
    Tcl_CreateCommand(interp, "setContrast", SetContrastCmd, (ClientData) 0,
	(void (*)()) NULL);
    Tcl_CreateCommand(interp, "setBandwidth", SetBandwidthCmd, (ClientData) 0,
	(void (*)()) NULL);
    Tcl_CreateCommand(interp, "setAddress", SetAddressCmd, (ClientData) 0,
	(void (*)()) NULL);
    Tcl_CreateCommand(interp, "setPort", SetPortCmd, (ClientData) 0,
	(void (*)()) NULL);
    Tcl_CreateCommand(interp, "setName", SetNameCmd, (ClientData) 0,
	(void (*)()) NULL);
    Tcl_CreateCommand(interp, "setTTL", SetTTLCmd, (ClientData) 0,
	(void (*)()) NULL);
    Tcl_CreateCommand(interp, "sendVideo", SendVideoCmd, (ClientData) 0,
	(void (*)()) NULL);
    Tcl_CreateCommand(interp, "receiveVideo", ReceiveVideoCmd, (ClientData) 0,
	(void (*)()) NULL);

    (void) Tcl_VarEval(interp, "option add Nv.name \"", myname,
	"\" widgetDefault", NULL);

    if (Tcl_VarEval(interp, InitTK, InitNVProcs, InitNV, NULL) != TCL_OK) {
	fprintf(stderr, "NV init failed:\n");
	fprintf(stderr, "%s\n", interp->result);
	exit(1);
    }

    if (w != NULL) Tk_MapWindow(w);
    (void) Tcl_VarEval(interp, "update", NULL);

    signal(SIGHUP, Cleanup);
    signal(SIGINT, Cleanup);
    signal(SIGQUIT, Cleanup);
    signal(SIGPIPE, Cleanup);
    signal(SIGTERM, Cleanup);

    Tk_CreateTimerHandler(IDLEPOLL_TIME*1000, CheckIdle, (ClientData) NULL);

    recv_active = 1;
    OpenRecvFD();

    while (tk_NumMainWindows > 0) {
	if (send_active) {
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
