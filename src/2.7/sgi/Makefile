INCS	= -I/iris-import/contrib/tk2.3 -I/iris-import/contrib/tk2.3/tcl
LDLIBS	= -L/iris-import/lib -ltk -ltcl -lsvideo -lXext -lX11 -lm 
CFLAGS	= -O $(INCS) -D_BSD_SIGNALS

OBJS	= nv.o nv_init.o indigo_grab.o \
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
