/*
 * rtp.h
 *
 * Constants and structures based on the 12/15/92 draft of the RTP protocol
 * Internet Draft. This information is still subject to change.
 *
 */

#ifndef _rtp_h
#define _rtp_h

#define RTP_VERSION	1

/* Offset from UNIX's epoch to the NTP epoch in seconds (NTP's JAN_1970) */
#define RTP_EPOCH_OFFSET	2208988800UL

/* Basic RTP header */
struct rtphdr {
#ifndef LITTLE_BITFIELDS
	u_char	rh_vers:2;	/* version */
	u_char	rh_flow:6;	/* flow id */
	u_char	rh_opts:1;	/* options present */
	u_char	rh_sync:1;	/* end of synchronization unit */
	u_char	rh_content:6;	/* content id */
#else
	u_char	rh_flow:6;	/* flow id */
	u_char	rh_vers:2;	/* version */
	u_char	rh_content:6;	/* content id */
	u_char	rh_sync:1;	/* end of synchronization unit */
	u_char	rh_opts:1;	/* options present */
#endif
	u_short	rh_seq;		/* sequence number */
	u_long	rh_ts;		/* time stamp (middle of NTP timestamp) */
};

/* Basic RTP option header */
struct rtpopthdr {
#ifndef LITTLE_BITFIELDS
	u_char	roh_fin:1;	/* final option flag */
	u_char	roh_type:7;	/* option type */
#else
	u_char	roh_type:7;	/* option type */
	u_char	roh_fin:1;	/* final option flag */
#endif
	u_char	roh_optlen;	/* option len */
};

/* Normal RTP options */
#define RTPOPT_CSRC	0	/* Content source */
#define RTPOPT_SSRC	1	/* Synchronization source */
#define RTPOPT_BOP	2	/* Beginning of playout unit */

/* RTP source (CSRC, SSRC) option header */
struct rtpsrchdr {
#ifndef LITTLE_BITFIELDS
	u_char	rsh_fin:1;	/* final option flag */
	u_char	rsh_type:7;	/* option type */
#else
	u_char	rsh_type:7;	/* option type */
	u_char	rsh_fin:1;	/* final option flag */
#endif
	u_char	rsh_optlen;	/* option len (== 2) */
	u_short	rsh_uid;	/* unique id within host */
	u_long	rsh_addr;	/* IP address of host */
};

/* RTP BOP option header */
struct rtpbophdr {
#ifndef LITTLE_BITFIELDS
	u_char	rbh_fin:1;	/* final option flag */
	u_char	rbh_type:7;	/* option type */
#else
	u_char	rbh_type:7;	/* option type */
	u_char	rbh_fin:1;	/* final option flag */
#endif
	u_char	rbh_optlen;	/* option len (== 1) */
	u_short	rbh_seq;	/* sequence number of BOP */
};

/* RTCP forward direction options */
#define RTPOPT_CDESC	32	/* Content description */
#define RTPOPT_SDESC	33	/* Source description */
#define RTPOPT_FDESC	34	/* Flow description */
#define RTPOPT_BYE	35	/* Conference exit notification */

/* RTCP CDESC option header */
struct rtcpcdeschdr {
#ifndef LITTLE_BITFIELDS
	u_char	rtch_fin:1;	/* final option flag */
	u_char	rtch_type:7;	/* option type */
#else
	u_char	rtch_type:7;	/* option type */
	u_char	rtch_fin:1;	/* final option flag */
#endif
	u_char	rtch_optlen;	/* option len */
#ifndef LITTLE_BITFIELDS
	u_char	rtch_x1:2;	/* reserved (must be 0) */
	u_char	rtch_content:6;	/* content id */
#else
	u_char	rtch_content:6;	/* content id */
	u_char	rtch_x1:2;	/* reserved (must be 0) */
#endif
	u_char	rtch_x2;	/* reserved (must be 0) */
	u_short	rtch_rport;	/* return port */
	u_char	rtch_cqual;	/* clock quality */
	u_char	rtch_x3;	/* reserved (must be 0) */
	u_long	rtch_cdesc;	/* content descriptor */
};

/* RTCP SDESC option header */
struct rtcpsdeschdr {
#ifndef LITTLE_BITFIELDS
	u_char	rtsh_fin:1;	/* final option flag */
	u_char	rtsh_type:7;	/* option type */
#else
	u_char	rtsh_type:7;	/* option type */
	u_char	rtsh_fin:1;	/* final option flag */
#endif
	u_char	rtsh_optlen;	/* option len */
	u_short	rtsh_uid;	/* unique id within host */
	u_long	rtsh_addr;	/* IP address of host */
};

/* RTCP BYE option header */
struct rtcpbyehdr {
#ifndef LITTLE_BITFIELDS
	u_char	rtbh_fin:1;	/* final option flag */
	u_char	rtbh_type:7;	/* option type */
#else
	u_char	rtbh_type:7;	/* option type */
	u_char	rtbh_fin:1;	/* final option flag */
#endif
	u_char	rtbh_optlen;	/* option len */
	u_short	rtbh_uid;	/* unique id within host */
	u_long	rtbh_addr;	/* IP address of host */
};

/* RTCP reverse direction options */
#define RTPOPT_QOS	64	/* Quality of service */
#define RTPOPT_RAD	65	/* Raw application data */

/* Basic RTCP reverse packet header */
struct rtcprevhdr {
	u_char	rtrh_flow;	/* flow id */
	u_char	rtrh_x1;	/* reserved (must be 0) */
	u_char	rtrh_x2;	/* reserved (must be 0) */
	u_char	rtrh_x3;	/* reserved (must be 0) */
};

/* RTCP QOS option header */
struct rtcpqoshdr {
#ifndef LITTLE_BITFIELDS
	u_char	rtqh_fin:1;	/* final option flag */
	u_char	rtqh_type:7;	/* option type */
#else
	u_char	rtqh_type:7;	/* option type */
	u_char	rtqh_fin:1;	/* final option flag */
#endif
	u_char	rtqh_optlen;	/* option len (== 5) */
	u_short	rtqh_uid;	/* unique id within host */
	u_long	rtqh_addr;	/* IP address of host */
	u_short	rtqh_precv;	/* packets received */
	u_short	rtqh_seqrange;	/* sequence number range */
	u_short	rtqh_mindel;	/* minimum delay */
	u_short	rtqh_maxdel;	/* maximum delay */
	u_short	rtqh_avgdel;	/* average delay */
	u_short	rtqh_x;		/* reserved (must be 0) */
};

/* RTP standard content encodings for audio */
#define RTPCONT_PCMU		0	/* 8kHz PCM mu-law mono */
#define RTPCONT_1016		1	/* 8kHz CELP (Fed Std 1016) mono */
#define RTPCONT_G721		2	/* 8kHz G.721 ADPCM mono */
#define RTPCONT_GSM		3	/* 8kHz GSM mono */
#define RTPCONT_G723		4	/* 8kHz G.723 ADPCM mono */
#define RTPCONT_DVI		5	/* 8kHz Intel DVI ADPCM mono */
#define RTPCONT_L16_16		6	/* 16kHz 16-bit linear mono */
#define RTPCONT_L16_44_2	7	/* 44.1kHz 16-bit linear stereo */

/* RTP standard content encodings for video */
#define RTPCONT_CUSEEME		27	/* Cornell CU-SeeMe */
#define RTPCONT_NV		28	/* Xerox PARC nv */
#define RTPCONT_DVC		29	/* BBN dvc */
#define RTPCONT_BOLT		30	/* Bolter */
#define RTPCONT_H261		31	/* CCITT H.261 */

#endif /*_rtp_h*/
