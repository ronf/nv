CC	= gcc
INCS	= -I/import/VideoPix/lib -I/import/VideoPix/sys \
	  -I/project/tcl/src/tk3.2 -I/project/tcl/src/tcl6.7 \
	  -I/import/X11R4/include -I/import/mcast/include
LDLIBS	= /project/tcl/lib/libtk3.2.a /project/tcl/lib/libtcl6.7.a \
	  -L/import/VideoPix/lib -lvfc -L/import/X11R4/usr/lib -lXext -lX11 -lm
CFLAGS	= -ansi -O2 -static $(INCS)

OBJS	= nv.o nv_init.o vfc_grab.o cuseeme_decode.o \
	  vid_color.o vid_decode.o vid_encode.o vid_grey.o vid_image.o \
	  vid_transform.o vid_widget.o

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

vid_widget.s:
	$(CC) -S $(CFLAGS) vid_widget.c

vid_encode.s:
	$(CC) -S $(CFLAGS) vid_encode.c
