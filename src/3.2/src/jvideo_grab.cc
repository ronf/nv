/*
 * Copyright (c) 1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Netvideo version 3.2
 * Written by Ron Frederick <frederick@parc.xerox.com>
 *
 * J-Video frame grabber for decstation.
 * Written by Steven McCanne <mccanne@ee.lbl.gov>, May 1993.
 */

#ifndef lint
static char rcsid[] =
    "@(#) $Header: jvideo_grab.cc,v 1.2 93/05/26 19:42:46 mccanne Exp $ (LBL)";
#endif

#ifdef JVIDEO_GRABBER

#include <osfcn.h>

extern "C" {
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <mips/cachectl.h>

extern int cacheflush(char* addr, int nbytes, int cache);
};

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <tk.h>

#include "jvcodec.h"

struct YUV {
	u_char y;
	u_char u;
	u_char v;
	u_char pad;
};
static struct YUV YUVmap[256];

JVideoCodec* codec;
JVideoCodec::dmabuf* db;

int
catchframe(u_char *yin, s_char *uvin)
{
	int len;
	static int first = 1;

	if (first) {
		first = 0;
		codec->start_grab();
		codec->wait_grab(len);
		codec->start_decompress(codec->capid(), db->dma_id);
	}
	/*
	 * Wait for last decompression to finish, and start up the
	 * next frame capture.  This allows us to overlap frame
	 * capture with the YUV tranlation loop below.
	 */
	codec->wait_decompress();
	codec->start_grab();

	register u_long* yp = (u_long*)yin;
	register u_long* uvp = (u_long*)uvin;
	register u_long* sp = (u_long*)db->bp;
	register u_long* ep = yp + (320 * 240)/4;/*XXX*/
	int cnt = 320;
	while (yp < ep) {
		register u_long s = *sp++;
		register u_long y;
		register struct YUV *p = YUVmap;

		y = p[s & 0xff].y;
		y |= p[(s >> 8) & 0xff].y << 8;
		y |= p[(s >> 16) & 0xff].y << 16;
		y |= p[s >> 24].y << 24;
		*yp++ = y;
		if (uvp != 0)
			*uvp++ = p[s & 0xff].u |
				(p[(s >> 8) & 0xff].v << 8) |
				(p[(s >> 16) & 0xff].u << 16) |
				(p[(s >> 24) & 0xff].v << 24);

		/* Don't ask */
		cnt -= 4;
		if (cnt <= 0) {
			cnt = 320;
			sp += 3;
		}
	}
	/*
	 * Wait for the last frame capture to finish, then start
	 * up the next decompression.  This allows us to overlap
	 * the hardware decompression with nv processing.
	 *
	 * We flush the cache here, since the J-Video card
	 * will DMA in a fresh image underneath us.
	 */
	(void)codec->wait_grab(len);
	if (cacheflush((char*)db->bp, 332 * 240, DCACHE) < 0) {
		perror("cacheflush");
		exit(1);
	}
	codec->start_decompress(codec->capid(), db->dma_id);

	return (1);
}

int
pixround(double p)
{
	int pixel = (int)p;

	if (pixel < -128)
		pixel = -128;
	else if (pixel > 127)
		pixel = 127;

	return (pixel);
}
		
void
init_colortab(void)
{
	const JvDitherVals& dv = codec->getDitherVals();

	for (int i = 0; i < 256; ++i) {
		double r = dv.ColorLUT[i][0] / 256.;
		double g = dv.ColorLUT[i][1] / 256.;
		double b = dv.ColorLUT[i][2] / 256.;

		double y = 0.299 * r + 0.587 * g + 0.114 * b;
		double u = 126. * (b - y);
		double v = 160. * (r - y);

		struct YUV* p = &YUVmap[i];
		p->y = (int)(y * 255.);
		p->u = pixround(u);
		p->v = pixround(v);
	}
}

int
jvideo_init()
{
	codec = new JVideoCodec();
	if (!codec->valid())
		return (-1);
	/*XXX*/
	db = codec->allocdma(332 * 240);

	init_colortab();

	return (0);
}

extern "C" {

int (*GrabImage)(u_char *y_data, s_char *uv_data);

int
GrabImage_Init(int framerate, int *widthp, int *heightp)
{
	if (jvideo_init() < 0)
		return (0);

	*widthp = 320;
	*heightp = 240;

	GrabImage = catchframe;

	return (1);
}

void
GrabImage_Cleanup(void)
{
	codec->freedma(db);
	delete codec;
}
};

#else
int (*GrabImage)(u_char *y_data, u_char *uv_data);

int
GrabImage_Init(framerate, widthp, heightp)
	int framerate;
	int *widthp, *heightp;
{
}

void
GrabImage_Cleanup()
{
}
#endif
