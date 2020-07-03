#
# Uncomment and edit these lines on a Sun running SunOS 4.x
#
#CC	= gcc
#INCS	= -I/usr/local/include -I/usr/openwin/include
#NVLIBS	= -L/usr/local/lib -lXvid -lvfc -ltk -ltcl \
#	  -L/usr/openwin/lib -lXext -lX11 -lm
#CFLAGS	= -ansi -g -O2 -DPARALLAX -DVIDEOPIX -DX11GRAB $(INCS)

#
# Uncomment and edit these lines on a Sun running SunOS 5.x
#
#CC	= gcc
#INCS	= -I/opt/SUNWits/Graphics-sw/xil/include \
#	  -I/opt/VideoPix/lib -I/opt/VideoPix/sys \
#	  -I/opt/tcl/include -I/usr/openwin/include
#NVLIBS	= -L/opt/SUNWits/Graphics-sw/xil/lib \
#	  -R/opt/SUNWits/Graphics-sw/xil/lib -lxil \
#	  -L/opt/VideoPix/lib -lvfc \
#	  -L/opt/tcl/lib -ltk -ltcl \
#	  -L/usr/openwin/lib -R/usr/openwin/lib  -lXext -lX11 \
#	  -lthread -lsocket -lnsl -lm 
#CFLAGS	= -g -O2 -DSUNOS_5 -DSUNVIDEO -DVIDEOPIX -DX11GRAB $(INCS)

#
# Uncomment and edit these lines on an SGI
#
#INCS	= -I/usr/local/include
#NVLIBS	= -L/usr/local/lib -ltk -ltcl -lvl -ldmedia -lXext -lX11 -lm
#CFLAGS	= -g3 -O2 -DINDIGO -DSGIVL -DX11GRAB $(INCS)

#
# Uncomment and edit these lines on a DECstation 5000
#
#INCS	= -I/usr/local/include
#NVLIBS	= -L/usr/local/lib -ltk -ltcl -lXv -lXext -lX11 -lm
#CFLAGS	= -g3 -O2 -DLITTLE_BITFIELDS -DPIP -DX11GRAB $(INCS)

#
# Uncomment and edit these lines on a DEC Alpha with MME installed
#
#INCS	= -I/usr/local/include
#NVLIBS	= -L/usr/local/lib -ltk -ltcl -lmme -lXext -lX11 -lm
#CFLAGS	= -migrate -g3 -O2 -DLITTLE_BITFIELDS -DJ300 -DJ300_MME \
#	  -DMME_FRAME -DX11GRAB $(INCS)

#
# Uncomment and edit these lines on a DEC Alpha without MME installed
#
#INCS	= -I/usr/local/include
#NVLIBS	= -L/usr/local/lib -ltk -ltcl -lXext -lX11 -lm
#CFLAGS	= -migrate -g3 -O2 -DLITTLE_BITFIELDS -DJ300 -DX11GRAB $(INCS)

#
# Uncomment and edit these lines on an 80x86 running BSDI
#
#INCS	= -I/usr/X11/include
#NVLIBS	= -L/usr/local/lib -ltk -ltcl -L/usr/X11/lib -lX11 -lm
#CFLAGS	= -ansi -O2 -DLITTLE_BITFIELDS -DX11GRAB -DNO_SHM $(INCS)

#
# Uncomment and edit these lines on an HP9000/7xx
#
#INCS	= -I/usr/local/include
#NVLIBS	= -L/usr/local/lib -ltk -ltcl -lXv -lXext -lX11 -lm
#CFLAGS	= -g3 -O2 -DVIDEOLIVE -DX11GRAB $(INCS)

OBJS	= nv.o nv_init.o nv_decode.o nv_encode.o nv_transform.o \
	  nvdct_transform.o cellb_decode.o cellb_encode.o cellb_tables.o \
	  ibmvca_grab.o indigo_grab.o j300_grab.o jvdriverint.o \
	  parallax_grab.o parcvid_grab.o pip_grab.o sgivl_grab.o \
	  sunvideo_grab.o videolive_grab.o videopix_grab.o x11_grab.o \
	  vid_color.o vid_grey.o vid_image.o vid_util.o vid_widget.o

all: nv

nv: $(OBJS)
	$(CC) $(CFLAGS) -o nv $(OBJS) $(NVLIBS)

nv_init.c: tcl2c tk_init.tcl nv_init.tcl nv_grabpanels.tcl
	./tcl2c TK_Init < tk_init.tcl > nv_init.c
	./tcl2c NV_Init < nv_init.tcl >> nv_init.c
	./tcl2c NV_GrabPanels < nv_grabpanels.tcl >> nv_init.c

tcl2c: tcl2c.c
	$(CC) $(CFLAGS) -o tcl2c tcl2c.c

clobber: clean
	-rm nv

clean:
	-rm -f $(OBJS) tcl2c nv_init.c
