/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	SunOS 5.x get_multicast_interface support
*/

/*
 * Copyright (c) 1993 by Sun Microsystems Inc.
 */

#ifdef	SUNOS_5
#include <fcntl.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#include <netinet/in.h>
#include <inet/common.h>
#include <inet/ip.h>
#include <inet/mib2.h>

/*
 * get_multicast_interface()
 *
 * We want to know what source IP address the kernel will put in outgoing
 *   multicast packets.  To do this we interrogate the kernel's IP MIB,
 *   extract the routing table, find the routing entry for multicast
 *   (the entry labelled "224.0.0.0" when you run "netstat -r"), and then
 *   get the next-hop address in that entry.  You probably don't want to
 *   recalculate this too often.
 *
 * We get at the IP MIB using a glorious mishmash of streams and socket
 *   interfaces:  it's basically a streams optmgmt request, but the structure
 *   carried within the request (and acknowledgement) is a struct opthdr,
 *   defined in <sys/socket.h>.  See <inet/mib2.h> for gory details.
 *
 * Returns 0 for failure, or 1 (and sets *ifaddr) for success
 */

struct req_mishmash {
    struct T_optmgmt_req	mish;
    struct opthdr		mash;
};

struct ack_mishmash {
    struct T_optmgmt_ack	mish;
    struct opthdr		mash;
};

int
get_multicast_interface(struct in_addr *ifaddr)
{
    struct req_mishmash req;
    struct ack_mishmash ack;
    struct strbuf ctrl, data;
    int fd, flags, succeeded=0;

    if ((fd = open(IP_DEV_NAME, O_RDWR)) == -1) return 0;

    req.mish.PRIM_type = T_OPTMGMT_REQ;
    req.mish.OPT_offset = ((char *)&req.mash) - ((char *)&req.mish);
    req.mish.OPT_length = sizeof(req.mash);
    req.mish.MGMT_flags = MI_T_CURRENT;

    req.mash.level = MIB2_IP;		/* See <inet/mib2.h> */
    req.mash.name = 0;
    req.mash.len = 0;

    ctrl.buf = (char *) &req;
    ctrl.len = sizeof(req);

    if (putmsg(fd, &ctrl, 0, 0) == -1) return 0;

    /*
     * What we get back is a whole slew of messages.  The control part of
     *   each message looks very much like the request we sent;  the level
     *   and name fields tell us what sort of MIB info it is, and the len
     *   field tells how big the data part of the message will be.  The
     *   kernel seems to give us back a whole bunch of levels, not just
     *   the MIB2_IP we asked for.
     *
     * We blithely assume that the kernel lays out the ack in the same
     *   way that we laid out the request.
     *
     * The data buffer is grown on demand within the while-loop.
     */

    data.buf = 0;
    data.maxlen = 0;

    ctrl.buf = (char *) &ack;
    ctrl.maxlen = sizeof(ack);

    while ((flags=0, (getmsg(fd, &ctrl, 0, &flags) == MOREDATA)) &&
	   (ctrl.len >= sizeof(ack)) && (ack.mish.PRIM_type == T_OPTMGMT_ACK) &&
	   (ack.mish.MGMT_flags == T_SUCCESS)) {
	struct opthdr *ackmash=&ack.mash;
	mib2_ipRouteEntry_t *ire;

	if (ackmash->len > data.maxlen) {
	    if (data.buf != 0) free(data.buf);
	    data.maxlen = ackmash->len;
	    if ((data.buf = malloc(data.maxlen)) == 0) break;
	}

	/*
	 * Get the data part of the message.  What we'd really like
	 *   to do is skip it unless it's the routing info, but is
	 *   there streams magic to toss single data parts?
	 */
	flags = 0;
	if (getmsg(fd, 0, &data, &flags) != 0) break; /* Error of some sort */

	if (ackmash->level != MIB2_IP || ackmash->name != MIB2_IP_21) continue;

	for (ire = (mib2_ipRouteEntry_t *) data.buf;
	     (char *)(ire+1) <= data.buf+data.len; ire++) {
	    if ((ire->ipRouteInfo.re_ire_type == IRE_RESOLVER) &&
		(ire->ipRouteMask == htonl(IN_CLASSD_NET))) {
		/* Found the needle in the haystack */
		ifaddr->s_addr = ire->ipRouteNextHop;
		succeeded = 1;
		break;
	    }
	}

	if (succeeded) break;
    }

    /*
     * We end up here on success or on all sorts of failures -- about
     *   most of which we should probably complain loudly, but don't.
     */
    if (data.buf != 0) free(data.buf);
    close(fd); /* Any remaining messages should get flushed */
    return succeeded;
}

#else

/*
 * Not SunOS 5.x.  Other OSes on which 'nv' runs at present appear to have
 * networking code with a strong BSD heritage, so we don't need to do this.
 */

struct in_addr;

/*ARGSUSED*/
int
get_multicast_interface(struct in_addr *ifaddr)
{
    return 0;
}

#endif
