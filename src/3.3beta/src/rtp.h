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
	uint8	rh_vers:2;	/* version */
	uint8	rh_chanid:6;	/* channel id */
	uint8	rh_opts:1;	/* options present */
	uint8	rh_sync:1;	/* end of synchronization unit */
	uint8	rh_content:6;	/* content id */
#else
	uint8	rh_chanid:6;	/* channel id */
	uint8	rh_vers:2;	/* version */
	uint8	rh_content:6;	/* content id */
	uint8	rh_sync:1;	/* end of synchronization unit */
	uint8	rh_opts:1;	/* options present */
#endif
	uint16	rh_seq;		/* sequence number */
	uint32	rh_ts;		/* time stamp (middle of NTP timestamp) */
};

/* Basic RTP option header */
struct rtpopthdr {
#ifndef LITTLE_BITFIELDS
	uint8	roh_fin:1;	/* final option flag */
	uint8	roh_type:7;	/* option type */
#else
	uint8	roh_type:7;	/* option type */
	uint8	roh_fin:1;	/* final option flag */
#endif
	uint8	roh_optlen;	/* option len */
};

/* Normal RTP options */
#define RTPOPT_CSRC	0	/* Content source */
#define RTPOPT_SSRC	1	/* Synchronization source */
#define RTPOPT_BOP	2	/* Beginning of playout unit */

/* RTP synchronization source (SSRC) option header */
struct rtpssrchdr {
#ifndef LITTLE_BITFIELDS
	uint8	rsh_fin:1;	/* final option flag */
	uint8	rsh_type:7;	/* option type */
#else
	uint8	rsh_type:7;	/* option type */
	uint8	rsh_fin:1;	/* final option flag */
#endif
	uint8	rsh_optlen;	/* option len (== 1) */
	uint16	rsh_id;		/* source identifier */
};

/* RTP BOP option header */
struct rtpbophdr {
#ifndef LITTLE_BITFIELDS
	uint8	rbh_fin:1;	/* final option flag */
	uint8	rbh_type:7;	/* option type */
#else
	uint8	rbh_type:7;	/* option type */
	uint8	rbh_fin:1;	/* final option flag */
#endif
	uint8	rbh_optlen;	/* option len (== 1) */
	uint16	rbh_seq;	/* sequence number of BOP */
};

/* RTCP forward direction options */
#define RTPOPT_FMT	32	/* Format description */
#define RTPOPT_SDESC	33	/* Source description */
#define RTPOPT_BYE	35	/* Conference exit notification */

/* RTCP CDESC option header */
struct rtcpfmthdr {
#ifndef LITTLE_BITFIELDS
	uint8	rtfh_fin:1;	/* final option flag */
	uint8	rtfh_type:7;	/* option type */
#else
	uint8	rtfh_type:7;	/* option type */
	uint8	rtfh_fin:1;	/* final option flag */
#endif
	uint8	rtfh_optlen;	/* option len */
#ifndef LITTLE_BITFIELDS
	uint8	rtfh_x1:2;	/* reserved (must be 0) */
	uint8	rtfh_fmt:6;	/* format id */
#else
	uint8	rtfh_fmt:6;	/* format id */
	uint8	rtfh_x1:2;	/* reserved (must be 0) */
#endif
	uint8	rtfh_cqual;	/* clock quality */
};

/* RTCP SDESC option header */
struct rtcpsdeschdr {
#ifndef LITTLE_BITFIELDS
	uint8	rtsh_fin:1;	/* final option flag */
	uint8	rtsh_type:7;	/* option type */
#else
	uint8	rtsh_type:7;	/* option type */
	uint8	rtsh_fin:1;	/* final option flag */
#endif
	uint8	rtsh_optlen;	/* option len */
	uint16	rtsh_id;	/* content source id */
	uint32	rtsh_addr;	/* IP address of host */
};

/* RTCP BYE option header */
struct rtcpbyehdr {
#ifndef LITTLE_BITFIELDS
	uint8	rtbh_fin:1;	/* final option flag */
	uint8	rtbh_type:7;	/* option type */
#else
	uint8	rtbh_type:7;	/* option type */
	uint8	rtbh_fin:1;	/* final option flag */
#endif
	uint8	rtbh_optlen;	/* option len */
	uint16	rtbh_id;	/* content source id */
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
#define RTPCONT_CELLB		25	/* Sun CellB */
#define RTPCONT_JPEG		26	/* JPEG */
#define RTPCONT_CUSEEME		27	/* Cornell CU-SeeMe */
#define RTPCONT_NV		28	/* Xerox PARC nv */
#define RTPCONT_PICWIN		29	/* BBN Picture Window */
#define RTPCONT_CPV		30	/* Bolter CPV */
#define RTPCONT_H261		31	/* CCITT H.261 */

#endif /*_rtp_h*/
