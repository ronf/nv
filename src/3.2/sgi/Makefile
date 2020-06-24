INCS	= -I/iris-import/contrib/tk3.2 -I/iris-import/contrib/tk3.2/tcl
LDLIBS	= -L/iris-import/lib -ltk -ltcl -lsvideo -lXext -lX11 -lm 
CFLAGS	= -O $(INCS)

OBJS	= nv.o nv_init.o cuseeme_decode.o indigo_grab.o \
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
