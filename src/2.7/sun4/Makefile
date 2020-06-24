INCS	= -I/import/VideoPix/lib -I/import/VideoPix/sys \
	  -I/project/tcl/src/tk2.3 -I/project/tcl/src/tcl6.4 \
	  -I/import/X11R4/include -I/tilde/frederic/kernels-4.1.3
LDLIBS	= /project/tcl/lib/libtk2.3.a /project/tcl/lib/libtcl6.4.a \
	  -L/import/VideoPix/lib -lvfc -L/import/X11R4/usr/lib -lXext -lX11 -lm 
CFLAGS	= -O2 $(INCS)

OBJS	= nv.o nv_init.o vfc_grab.o \
	  vid_decode.o vid_encode.o vid_image.o vid_widget.o

all: nv

nv: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

nv_init.c: tcl2c init_tk.tcl init_nvprocs.tcl init_nv.tcl
	tcl2c InitTK < init_tk.tcl > nv_init.c
	tcl2c InitNVProcs < init_nvprocs.tcl >> nv_init.c
	tcl2c InitNV < init_nv.tcl >> nv_init.c

clobber: clean
	-rm nv

clean:
	-rm -f $(OBJS) tcl2c nv_init.c
