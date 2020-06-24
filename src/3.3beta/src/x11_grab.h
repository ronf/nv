/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	X11 screen grab headers
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

extern int X11Grab_Probe(void);
extern char *X11Grab_Attach(void);
extern void X11Grab_Detach(void);
extern grabproc_t *X11Grab_Start(int framerate, int config,
				 reconfigproc_t *reconfig);
extern void X11Grab_Stop(void);
