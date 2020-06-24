TKDIR   = /project/tk
INCS	= -I$(TKDIR) -I$(TKDIR)/tcl
LDLIBS	= -L$(TKDIR) -ltk -L$(TKDIR)/tcl -ltcl -lXext -lX11 -lm
CFLAGS	= -O2 $(INCS)

OBJS	= nv.o nv_init.o sony_grab.o \
	  vid_decode.o vid_encode.o vid_image.o \
	  vid_transform.o vid_update.o vid_widget.o

all: nv

nv: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

nv_init.c: tcl2c tk_init.tcl nv_subr.tcl nv_uiprocs.tcl nv_api.tcl nv_init.tcl
	./tcl2c TK_Init < tk_init.tcl > nv_init.c
	./tcl2c NV_Subr < nv_subr.tcl >> nv_init.c
	./tcl2c NV_UIProcs < nv_uiprocs.tcl >> nv_init.c
	./tcl2c NV_API < nv_api.tcl >> nv_init.c
	./tcl2c NV_Init < nv_init.tcl >> nv_init.c

# NEWS cpp can't handle a long token. Use gcc-cpp instead.
nv_init.o: nv_init.c
	gcc -E -P $(CFLAGS) nv_init.c > nv_init.i
	$(CC) -c -o nv_init.o nv_init.i
	/bin/rm -f nv_init.i

clobber: clean
	-rm nv

clean:
	-rm -f $(OBJS) tcl2c nv_init.c
