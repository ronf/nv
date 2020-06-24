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
 */

#ifndef lint
static char rcsid[] =
    "@(#) $Header: jvcodec.cc,v 1.1 93/05/26 19:46:55 mccanne Exp $ (LBL)";
#endif

#include <osfcn.h>

#include  "JV_vcd.h"
#include  "jv_reg.h"
#include  "JV_err.h"
#include  "err.h"

extern "C" {
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int JvInit(const char* device, JvRegs* jvr);
int JvtSelCompVideoQuality(JvJpegDesc* desc, JvTvStandard format, 
			   JvVideoQual vqual);
int JvLoadCompJpegParams(JvJpegDesc* desc, jv_comp_types mode);
int JvLoadDcmpJpegParams(JvJpegDesc* desc);
int JvLoadStillJpegParams(JvJpegDesc* desc);
int JvLoadCompHuffTables(JvJpegDesc* jpeg);
int JvLoadCompQuantTables(JvJpegDesc* jpeg);
int JvLoadDcmpHuffTables(JvJpegDesc* descp);
int JvLoadDcmpQuantTables(JvJpegDesc* descp);
int JvtSetupVideoIn(JvTvStandard TVstandard, u_char SAA7191array[0x18]);
int JvtScale(int XInSize, int* XOutSize, int YInSize, int* YOutSize);

int JvCompField(int cob_id, int* NumBytes, JvTimeStamp* stamp);
int JvDcmpBlockField(int dib_id, int dob_id);
int JvDcmpField(int in_id, int out_id);
int jv_MakeQTables(int factor, u_int compCoef2[], u_int decompCoef2[]);
int jv_LoadDefaultHuffmanTables(volatile u_int* reg_base, u_short table[]);
int jv_LoadQTables(volatile u_int* reg_base, u_int q_table[]);
int JvUnLockDmaBuf(int buf_id);
int JvLockDmaBuf(u_char* vm, int num_bytes, int* BufID);
int JvtDitherSetup(DitherVals* DV);

int JvStillCompBlock(int comp_id, int dcmp_id, int* NumCompBytes);

int JvBlock(int, int);

/*
 * The double's here should really be floats, but conventional C
 * coerces to double.
 */
int JvtLuminanceAdjust(double contrast, double brightness, DitherVals* DV);
int JvtChromaAdjust(double saturation, DitherVals* DV);

int JvCompBlockField(int cob_id, int *NumBytes, JvTimeStamp *stamp);

#ifdef mips
#include <mips/cachectl.h>
extern int cacheflush(char* addr, int len, int cache);
#endif
};

#include "jvcodec.h"

/* The J-Video library code references this global. */
extern "C" {
JvRegs dsp_jvr;         /* Jvideo registers */
};

JVideoCodec::JVideoCodec()
{
	initparams();

	char* device = getenv("JVDEV");
	if (device == 0)
		device = "/dev/jv0";

	int status = JvInit(device, &dsp_jvr);
	valid_ = (status >= 0);

	Init();
}

JVideoCodec::~JVideoCodec()
{
	freedma(cb0);
	freedma(cb1);
}

u_char* JVideoCodec::grab(int& len)
{
	int buflen = capbuf->len;
	int status = JvCompBlockField(capbuf->dma_id, &buflen, 0);
	ck(status, "JvCompBlockField");

	u_char* addr = capbuf->bp;
#ifdef mips
	if (cacheflush((char*)addr, buflen, DCACHE) < 0) {
		perror("cacheflush");
		exit(1);
	}
#endif
	len = buflen;
	return (addr);
}

void JVideoCodec::start_grab()
{
	/*
	 * Swap buffers and start up the next capture.
	 */
	capbuf = (capbuf == cb0) ? cb1 : cb0;
	int status = JvCompField(capbuf->dma_id, &capbuf->len, 0);
	ck(status, "start_grab (JvCompField)");
}

u_char* JVideoCodec::wait_grab(int& length)
{
	int status;
	for (int i = 0; i < 3; ++i) {
		status = JvBlock(JVOP_COMP, FIELD_MSEC_TIMEOUT);
		/*
		 * Intervening calls to jv can eat the compression done
		 * callback, so ignore timeout errors.
		 */
		if (status == BadTimeOut)
			continue;
	}
	ck(status, "wait_grab (JvBlock)");
#ifdef mips
	/* XXX this won't work if the cache is write-back. */
	if (cacheflush((char*)capbuf->bp, capbuf->len, DCACHE) < 0) {
		perror("cacheflush");
		exit(1);
	}
#endif
	length = capbuf->len;
	return (capbuf->bp);
}

void JVideoCodec::decompress(int in_id, int out_id)
{
	int status = JvDcmpBlockField(in_id, out_id);
	ck(status, "decompress (JvDcmpBlockField)");
}

void JVideoCodec::start_decompress(int in_id, int out_id)
{
	int status = JvDcmpField(in_id, out_id);
	ck(status, "start_decompress (JvDcmpField)");
}

void JVideoCodec::wait_decompress()
{
	int status = JvBlock(JVOP_DCMP, FIELD_MSEC_TIMEOUT);
	ck(status, "wait_decompress (JvBlock)");
}

void JVideoCodec::compress(int in_id, int out_id, int& len)
{
	int cc;
	int status = JvStillCompBlock(out_id, in_id, &cc);
	ck(status, "compress (JvStillCompBlock)");
	len = cc;
}

void JVideoCodec::setQ(u_int q_factor) const
{
#define JVR		_jvrgs
#define JV550		_jvcl550b
	u_int cmpQTable[256];
	u_int dcmpQTable[256];

	jv_MakeQTables(q_factor, cmpQTable, dcmpQTable);
	jv_LoadDefaultHuffmanTables(JVR.comp.base, JV550.cmp_huffman_table);
	jv_LoadQTables(JVR.comp.base, cmpQTable);
	jv_LoadDefaultHuffmanTables(JVR.dcmp.base, JV550.dec_huffman_table);
	jv_LoadQTables(JVR.dcmp.base, dcmpQTable);
#undef JVR
#undef JV550
}

void JVideoCodec::freedma(dmabuf* db)
{
	(void)JvUnLockDmaBuf(db->dma_id);
	if (db->shm_id >= 0) {
		if (shmdt(db->bp) < 0) {
			perror("shmdt");
			exit(1);
		}
		if (shmctl(db->shm_id, IPC_RMID, 0) < 0) {
			perror("shmctl");
			exit(1);
		}
	} else
		delete db->base;
	delete db;
}

JVideoCodec::dmabuf* JVideoCodec::allocdma(int size)
{
	dmabuf* db = new dmabuf;
	u_char* p = new u_char[size + NBPG];
	db->base = p;
	/*
	 * Align address to a page boundary.
	 */
	int off = NBPG - ((int)p & (NBPG-1));
	p += off;

	int status = JvLockDmaBuf(p, size, &db->dma_id);
	ck(status, "allocdma (JvLockDmaBuf)");

	db->shm_id = -1;
	db->bp = p;

	return (db);
}

JVideoCodec::dmabuf* JVideoCodec::allocshdma(int size)
{
	dmabuf* db = new dmabuf;
	int shmid = shmget(IPC_PRIVATE, size, IPC_CREAT|0777);
	if (shmid < 0) {
		perror("shmget");
		exit(1);
	}
	u_char* p = (u_char *)shmat(shmid, 0, 0);
	if (p == (u_char*)-1) {
		perror("shmat");
		exit(1);
	}
	int status = JvLockDmaBuf(p, size, &db->dma_id);
	ck(status, "allocshdma (JvLockDmaBuf)");

	db->shm_id = shmid;
	db->bp = p;

	return (db);
}

void JVideoCodec::initparams()
{
	jd.video_format = NTSC;
	jd.frame_rate = 15;
	jd.x_active = 640;
	jd.y_active = 240;
	jd.x_decimate = jd.y_decimate = 1;
	jd.x_offset = 10;
	/*XXX*/
	jd.q_factor = 100;

	cw = ch = 0;
}

inline void JVideoCodec::ck(int st, char* s)
{
	if (st < 0) {
		fprintf(stderr, "jvideo (%s): error %d\n", s, st);
		exit(1);
	}
}

void JVideoCodec::dithersetup()
{
	dv.MakeNewLUTs = TRUE;
	dv.LoadLUTs = TRUE;
	dv.ColorSpace = YUV;
	dv.OutColors = 256;
	dv.Mono = 0;
	dv.Interlace = 0;
  
	for (int i = 0; i < 256; ++i)
		dv.IndexMap[i] = i;
	int status = JvtDitherSetup(&dv);
	ck(status, "dithersetup (JvtDitherSetup)");

	dv.AdjDef[1].negIn = FALSE;
	dv.AdjDef[2].negIn = FALSE;
	dv.AdjDef[3].negIn = FALSE;

	status = JvtLuminanceAdjust(double(0), double(10), &dv);
	ck(status, "dithersetup (JvtLuminanceAdjust)");

	/*XXX need to check for color */
	status = JvtChromaAdjust(double(0), &dv);
	ck(status, "dithersetup (JvtChromaAdjust)");
}

int JVideoCodec::dimensions(int& width, int& height)
{
	int h = height;
	int w = width + 12;
#ifdef __alpha
	w = (w  + 7) & ~7;
#else
	w = (w  + 3) & ~3;
#endif
	int n = w * h;
	w -= 12;
	while (1) {
		int status = JvtScale(jd.x_size, &w, jd.y_size, &h);
		ck(status, "dimensions (JvtScale)");
#ifdef __alpha
		if ((12 + w) % 8 == 0)
			break;
#else
		if ((12 + w) % 4 == 0)
			break;
#endif
		--w;
	}
	width = w + 12;
	height = h;

	return (n);
}

/*
 * Change the dithering output size.  E.g., the jpeg frames are still
 * 320x240, but the hardware can scale the output image up.
 * Width and height must be values that were returned by
 * dimensions().
 */
void JVideoCodec::resize(int width, int height)
{
	width -= 12;
	if (cw != width || ch != height) {
		int w = width;
		int h = height;
		int status = JvtScale(jd.x_size, &w, jd.y_size, &h);
		ck(status, "resize (JvtScale)");
		/*XXX*/
		if (w != width || h != height)
			abort();
		cw = width;
		ch = height;
	}
}

void JVideoCodec::Init()
{
	int status = JvtSelCompVideoQuality(&jd, NTSC, VQUAL_VERY_HIGH);
	ck(status, "JvtSelCompVideoQuality");
	
	status = JvLoadCompJpegParams(&jd, COMP_LIVE_NTSC);
	ck(status, "JvLoadCompJpegParams");
	
	status = JvLoadDcmpJpegParams(&jd);
	ck(status, "JvLoadDcmpJpegParams");
	
	/*
	 * The above statement did not establish tables. These must be handled 
	 * separately:  Load compression and decompression tables
	 */
	setQ(jd.q_factor);

	status = JvLoadCompHuffTables(&jd);
	ck(status, "JvLoadCompHuffTables");

	status = JvLoadCompQuantTables(&jd);
	ck(status, "JvLoadCompQuantTables");

	status = JvLoadDcmpHuffTables(&jd);
	ck(status, "JvLoadDcmpHuffTables");

	status = JvLoadDcmpQuantTables(&jd);
	ck(status, "JvLoadDcmpQuantTables");
	
	dithersetup();

	static u_char DmsdArray[0x20];
	status = JvtSetupVideoIn(NTSC, DmsdArray);
	ck(status, "JvtSetupVideoIn");

	int w = 320; int h = 240;
	(void)dimensions(w, h);
	resize(w, h);
	
	/*XXX is 64K enough? */
	const int NumCompBytes = 64 * 1024;
	cb0 = allocdma(NumCompBytes);
	cb1 = allocdma(NumCompBytes);
	capbuf = cb0;
}

