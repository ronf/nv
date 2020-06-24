/*
 * rtp.h
 *
 * Constants and structures based on the 12/15/92 draft of the RTP protocol
 * Internet Draft. This information is still subject to change.
 *
 */

#ifndef _rtp_h
#define _rtp_h

/* Offset from UNIX's epoch to the NTP epoch in seconds (NTP's JAN_1970) */
#define RTP_EPOCH_OFFSET	2208988800UL

#define RTP_V2		0x8000

#define RTP_TYPEMASK	0xc000
#define RTP_P		0x2000
#define RTP_X		0x1000
#define RTP_CCMASK	0x0f00
#define RTP_M		0x0080
#define RTP_PTMASK	0x007f

#define RTP_CCSHIFT	8

#define RTCP_CNTMASK	0x1f00
#define RTCP_PTMASK	0x00ff

#define RTCP_CNTSHIFT	8

/* Basic RTP header */
typedef struct {
    uint16	flags;		/* T:2 P:1 X:1 CC:4 M:1 PT:7 */
    uint16	seq;		/* sequence number */
    uint32	ts;		/* time stamp */
    uint32	ssrc;		/* synchronization src */
} rtphdr_t;

/* RTP header extension */
typedef struct {
    uint16	rsvd;		/* reserved */
    uint16	len;		/* length */
} rtphdrx_t;

/* Basic RTCP header */
typedef struct {
    uint16	flags;		/* T:2 P:1 CNT:5 PT:8 */
    uint16	len;		/* length */
} rtcphdr_t;

/* RTCP SR header */
typedef struct {
    rtcphdr_t	h;		/* common header */
    uint32	ssrc;		/* synchronization src */
    uint32	ntp_hi;		/* NTP timestamp, high word */
    uint32	ntp_lo;		/* NTP timestamp, low word */
    uint32	rtp_ts;		/* RTP timestamp */
    uint32	pkts;		/* packets sent */
    uint32	bytes;		/* bytes sent */
} rtcp_sr_t;

/* RTCP RR item */
typedef struct {
    uint32	ssrc;		/* syncrhronization src */
    uint32	received;	/* packets received */
    uint32	expected;	/* pakcets expected */
    uint32	jitter;		/* interarrival jitter */
    uint32	lsr;		/* timestamp of last SR received */
    uint32	dlsr;		/* time since last SR received */
} rtcp_rritem_t;

/* RTCP RR header */
typedef struct {
    rtcphdr_t	h;		/* common header */
    uint32	ssrc;		/* synchronization src */
} rtcp_rr_t;

/* RTP standard payload types for audio */
#define RTP_PT_PCMU		0	/* 8kHz PCM mu-law mono */
#define RTP_PT_1016		1	/* 8kHz CELP (Fed Std 1016) mono */
#define RTP_PT_G721		2	/* 8kHz G.721 ADPCM mono */
#define RTP_PT_GSM		3	/* 8kHz GSM mono */
#define RTP_PT_G723		4	/* 8kHz G.723 ADPCM mono */
#define RTP_PT_DVI		5	/* 8kHz Intel DVI ADPCM mono */
#define RTP_PT_L16_16		6	/* 16kHz 16-bit linear mono */
#define RTP_PT_L16_44_2		7	/* 44.1kHz 16-bit linear stereo */

/* RTP standard payload types for video */
#define RTP_PT_CELLB		25	/* Sun CellB */
#define RTP_PT_JPEG		26	/* JPEG */
#define RTP_PT_CUSEEME		27	/* Cornell CU-SeeMe */
#define RTP_PT_NV		28	/* Xerox PARC nv */
#define RTP_PT_PICWIN		29	/* BBN Picture Window */
#define RTP_PT_CPV		30	/* Bolter CPV */
#define RTP_PT_H261		31	/* CCITT H.261 */

/* RTCP payload types */
#define RTCP_PT_SR		1	/* Sender Report */
#define RTCP_PT_RR		2	/* Reception Report */
#define RTCP_PT_SDES		3	/* Source Description */
#define RTCP_PT_BYE		4	/* Client shutdown */

/* SDES types */
#define RTCP_SDES_END		0	/* Canonical name */
#define RTCP_SDES_CNAME		1	/* Canonical name */
#define RTCP_SDES_NAME		2	/* User specified name */
#define RTCP_SDES_EMAIL		3	/* EMail address */
#define RTCP_SDES_LOC		4	/* Location */
#define RTCP_SDES_TXT		5	/* User specified text */

#endif /*_rtp_h*/
