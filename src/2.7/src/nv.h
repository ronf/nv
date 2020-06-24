/*
	Netvideo version 2.7
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

#define NV_DEFAULT_ADDR		"224.2.1.0"
#define NV_DEFAULT_PORT		"4444"

#define NV_BOLTER_ADDR		"224.2.0.2"
#define NV_BOLTER_PORT		"4242"

#define BOLTER_WIDTH		256
#define BOLTER_HEIGHT		200

#define MAX_NAMELEN		64
#define MAX_VID_PACKLEN		512

#define MAX_SOURCES		32
#define MAX_RECEIVERS		256

#define EST_FRAME_TIME		250000

#define NAME_XMITFREQ		8

#define IDLEPOLL_TIME		1
#define MAXIDLE_TIME		5
#define NOTIFY_TIME		60
