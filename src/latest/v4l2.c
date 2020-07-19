/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>

	Video for Linux 2 frame grab routines
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <linux/videodev2.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <tcl.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_code.h"

#define NUM_BUFFERS	2

extern Tcl_Interp *interp;

static int devices[32], inputs[32], num_inputs;
static int xmit_size, xmit_color, active_input;
static struct v4l2_buffer grab_buf;
static uint8 *grab_data[NUM_BUFFERS];
static size_t grab_data_len[NUM_BUFFERS];
static reconfigproc_t *reconfig;
static void *enc_state;

static int V4L2_Ctrl(int input, int ctl, void *arg)
{
    int result;

    do {
	result = ioctl(devices[input], ctl, arg);
    } while ((result == -1) && (errno == EINTR));

    return result;
}

static int V4L2_FindInputs(void)
{
    int dev, fd, index;
    struct v4l2_input input;
    char devname[16], input_names[2048], *p = input_names;

    for (dev=0; dev < 8; dev++) {
	sprintf(devname, "/dev/video%d", dev);

	if ((fd = open(devname, O_RDWR)) < 0)
	    continue;

	for (index=0; index < 4; index++) {
	    devices[num_inputs] = fd;
	    inputs[num_inputs] = index;
	    input.index = index;

	    if (V4L2_Ctrl(num_inputs, VIDIOC_ENUMINPUT, &input) < 0)
		break;

	    *p++ = '{';
	    strcpy(p, input.name);
	    p += strlen(input.name);
	    *p++ = '}';
	    *p++ = ' ';

	    num_inputs++;
	}
    }

    if (num_inputs > 0) {
	*--p = '\0';

	(void) Tcl_SetVar(interp, "v4l2Ports", input_names, TCL_GLOBAL_ONLY);
    }

    return num_inputs;
}

static int V4L2_ReqBufs(int count)
{
    struct v4l2_requestbuffers req;

    memset(&req, 0, sizeof(req));
    req.count = count;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    return V4L2_Ctrl(active_input, VIDIOC_REQBUFS, &req);
}

static void V4L2_FreeBufs(void)
{
    int i;

    memset(&grab_buf, 0, sizeof(grab_buf));

    for (i=0; i < NUM_BUFFERS; i++) {
	if (grab_data[i] != NULL) {
	    (void) munmap(grab_data[i], grab_data_len[i]);
	    grab_data[i] = NULL;
	    grab_data_len[i] = 0;
	}
    }

    (void) V4L2_ReqBufs(0);
}

static int V4L2_AllocBufs(int count)
{
    int i;
    struct v4l2_buffer buf;

    if (V4L2_ReqBufs(NUM_BUFFERS) < 0)
	return -1;

    for (i=0; i < NUM_BUFFERS; i++) {
	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = i;

	if (V4L2_Ctrl(active_input, VIDIOC_QUERYBUF, &buf) < 0)
	    goto fail;

	grab_data[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
			    MAP_SHARED, devices[active_input], buf.m.offset);
	grab_data_len[i] = buf.length;

	if (grab_data[i] == MAP_FAILED)
	    goto fail;

	if (V4L2_Ctrl(active_input, VIDIOC_QBUF, &buf) < 0)
	    goto fail;
    }

    return 0;

fail:
    V4L2_FreeBufs();
    return -1;
}

static int V4L2_Grab(uint8 **datap, int *lenp)
{
    if (grab_buf.length > 0)
	V4L2_Ctrl(active_input, VIDIOC_QBUF, &grab_buf);

    memset(&grab_buf, 0, sizeof(grab_buf));
    grab_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    grab_buf.memory = V4L2_MEMORY_MMAP;

    if (V4L2_Ctrl(active_input, VIDIOC_DQBUF, &grab_buf) < 0)
	return 0;

    *datap = grab_data[grab_buf.index];
    *lenp = grab_data_len[grab_buf.index];
    return 1;
}

static void V4L2_GrabStop(void)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    V4L2_Ctrl(active_input, VIDIOC_STREAMOFF, &type);
    V4L2_FreeBufs();
}

static grabproc_t *V4L2_GrabStart(void)
{
    int width, height;
    struct v4l2_format format;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    switch (xmit_size) {
    case VID_SMALL:
	width = 160;
	height = 120;
	break;
    case VID_MEDIUM:
	width = 320;
	height = 240;
	break;
    case VID_LARGE:
	width = 640;
	height = 480;
	break;
    }

    if (enc_state != NULL)
	V4L2_GrabStop();

    if (V4L2_Ctrl(active_input, VIDIOC_S_INPUT, &inputs[active_input]) < 0)
	return NULL;

    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = width;
    format.fmt.pix.height = height;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

    if (V4L2_Ctrl(active_input, VIDIOC_S_FMT, &format) < 0)
	return NULL;

    if (format.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV)
	return NULL;

    if (V4L2_AllocBufs(NUM_BUFFERS) < 0)
	return NULL;

    if (V4L2_Ctrl(active_input, VIDIOC_STREAMON, &type) < 0)
	return NULL;

    if (reconfig) (*reconfig)(enc_state, width, height);

    return V4L2_Grab;
}

/*ARGSUSED*/
static char *V4L2_TracePort(ClientData clientData, Tcl_Interp *interp,
			       char *name1, char *name2, int flags)
{
    if (enc_state != NULL)
	(void) V4L2_GrabStop();
 
    active_input = atoi(Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY));

    if (enc_state != NULL)
	(void) V4L2_GrabStart();
 
    return NULL;
}
 
int V4L2_Probe(void)
{
    if (!V4L2_FindInputs())
	return 0;

    Tcl_TraceVar(interp, "v4l2Port", TCL_TRACE_WRITES, V4L2_TracePort, NULL);

    return VID_GREYSCALE|VID_COLOR|VID_SMALL|VID_MEDIUM|VID_LARGE;
}

char *V4L2_Attach(void)
{
    return ".grabControls.v4l2";
}

void V4L2_Detach(void)
{
}

/*ARGSUSED*/
grabproc_t *V4L2_Start(int grabtype, int min_framespacing, int config,
		       reconfigproc_t *r, void *e)
{
    if (grabtype != VIDIMAGE_YUYV)
	return NULL;

    xmit_size = config & VID_SIZEMASK;
    xmit_color = 1;
    reconfig = r;
    enc_state = e;

    return V4L2_GrabStart();
}

void V4L2_Stop(void)
{
    if (enc_state != NULL) {
	V4L2_GrabStop();
	reconfig = NULL;
	enc_state = NULL;
    }
}
