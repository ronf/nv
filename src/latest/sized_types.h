/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>

	Machine-specific sized integer type definitions
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

typedef signed char	int8;		/*  8 bit signed int */
typedef short		int16;		/* 16 bit signed int */
typedef int		int32;		/* 32 bit signed int */
#if defined(__alpha)
typedef long		int64;		/* 64 bit signed int */
#endif

typedef unsigned char	uint8;		/*  8 bit unsigned int */
typedef unsigned short	uint16;		/* 16 bit unsigned int */
typedef unsigned int	uint32;		/* 32 bit unsigned int */
#if defined(__alpha)
typedef unsigned long	uint64;		/* 64 bit unsigned int */
#endif
