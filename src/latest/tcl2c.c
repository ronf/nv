/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	Simple hack to translate a Tcl/Tk init file into a C string constant
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
#include <stdlib.h>

int main(int argc, char **argv)
{
    int c, count=0;

    if (argc < 2) {
	fprintf(stderr, "Usage: %s stringname\n", argv[0]);
	exit(1);
    }

    printf("char %s[] = {\n    ", argv[1]);
    while ((c = getchar()) != EOF) {
	switch (c) {
	case '\'':
	    printf("'\\'', ");
	    break;
	case '\\':
	    printf("'\\\\', ");
	    break;
	case '\t':
	    printf("'\\t', ");
	    break;
	case '\n':
	    printf("'\\n', ");
	    count = 10;
	    break;
	default:
	    printf("'%c',  ", c);
	    break;
	}

	if (count++ == 10) {
	    printf("\n    ");
	    count = 0;
	}
    }
    printf("'\\0' };\n");
    exit(0);
    /*NOTREACHED*/
}
