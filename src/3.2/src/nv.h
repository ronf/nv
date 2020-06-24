/*
	Netvideo version 3.2
	Written by Ron Frederick <frederick@parc.xerox.com>

	Common definitions
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

#define NV_DEFAULT_ADDR		"224.2.1.1"
#define NV_DEFAULT_PORT		"4444"

#define NV_ICON_WIDTH		96
#define NV_ICON_HEIGHT		72

#define NV_MIN_FLOW		32		/* Hack to distinguish new nv */

#define MAX_NAMELEN		64
#define MAX_VID_PACKLEN		(1024-sizeof(struct rtphdr))

#define MAX_SOURCES		32
#define MAX_RECEIVERS		256

#define GRAB_ERROR_TIME		100
#define EST_FRAME_TIME		250

#define IDLEPOLL_TIME		1
#define NAME_XMIT_TIME		5
#define MAXIDLE_TIME		15
#define NOTIFY_TIME		60

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK		((u_long)0x7f000001)
#endif
