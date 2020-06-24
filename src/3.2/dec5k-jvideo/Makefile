#
# Uncomment these if you have the JVideo frame grabber.
#
JVIDEO_LIB = /usr/src/local/jvideo.mips/appl/lib/libapp.a \
	/usr/src/local/jvideo.mips/jvt/libjvt.a \
	/usr/src/local/jvideo.mips/lib/jv/libjv.a
JVIDEO_CFLAGS = -DJVIDEO_GRABBER \
	-I/usr/src/local/jvideo.mips/include \
	-I/usr/src/local/jvideo.mips/appl/include \
	-I/usr/src/local/jvideo.mips/h

CC = gcc
C++ = gcc
INCS =	-I/usr/src/local/tk3.2 \
	-I/usr/src/local/tk3.2/tcl
LIBTK = /usr/src/local/tk3.2/libtk.a
LIBTCL = /usr/src/local/tk3.2/tcl/libtcl.a
LDLIBS	= $(LIBTK) $(LIBTCL) -lXext -lX11 $(JVIDEO_LIB) -lm 
CFLAGS	= -O $(INCS) -Wall -DLITTLE_BITFIELDS $(JVIDEO_CFLAGS)


OBJS	= nv.o nv_init.o cuseeme_decode.o jvideo_grab.o jvcodec.o \
	  vid_color.o vid_decode.o vid_encode.o vid_grey.o vid_image.o \
	  vid_transform.o vid_widget.o

.SUFFIXES: .cc $(.SUFFIXES)

.cc.o:
	$(C++) -c $(CFLAGS) $*.cc

all: nv

nv: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

nv_init.c: tcl2c tk_init.tcl nv_subr.tcl nv_uiprocs.tcl nv_api.tcl nv_init.tcl
	./tcl2c TK_Init < tk_init.tcl > nv_init.c
	./tcl2c NV_Subr < nv_subr.tcl >> nv_init.c
	./tcl2c NV_UIProcs < nv_uiprocs.tcl >> nv_init.c
	./tcl2c NV_API < nv_api.tcl >> nv_init.c
	./tcl2c NV_Init < nv_init.tcl >> nv_init.c

clobber: clean
	-rm nv

clean:
	-rm -f $(OBJS) tcl2c nv_init.c
