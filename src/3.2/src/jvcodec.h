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
 * @(#) $Header: jvcodec.h,v 1.1 93/05/26 19:46:56 mccanne Exp $ (LBL)
 */

#ifndef nv_jvideocodec_h
#define nv_jvideocodec_h

extern "C" {
#include  "JV.h"
};

class JVideoCodec {
public:
	JVideoCodec();
	~JVideoCodec();
	/*
	 * Synchronous frame grabbing.
	 */
	u_char* grab(int& len);
	/*
	 * Asynchronous frame grabbing.
	 */
	void start_grab();
	u_char* wait_grab(int&);

	/*
	 * Synchronous decompression.
	 */
	void decompress(int in_id, int out_id);
	/*
	 * Asynchronous decompression.
	 */
	void start_decompress(int in_id, int out_id);
	void wait_decompress();

	void compress(int in_id, int out_id, int& length);
	int dimensions(int& width, int& height);
	void resize(int width, int height);

	struct dmabuf {
		u_char* bp;
		u_char* base;
		int dma_id;
		int shm_id;
		int len;	/* length of captured frame (not of buffer) */
	};
	dmabuf* allocdma(int size);
	dmabuf* allocshdma(int size);
	void freedma(dmabuf*);
	int valid() { return (valid_); }

	inline int capid() { return (capbuf->dma_id); }
	inline const JvDitherVals& getDitherVals() { return (dv); }
private:
	void Init();
	int valid_;
	void setQ(u_int q_factor) const;
	void initparams();
	void ck(int st, char* s);
	void dithersetup();

	/*
	 * Frame capture buffer.
	 */
	dmabuf* cb0;
	dmabuf* cb1;
	dmabuf* capbuf;

	JvDitherVals dv;
	JvJpegDesc jd;
	int cw;
	int ch;
};
#endif

