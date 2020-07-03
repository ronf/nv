/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>

	Mac video frame grab headers
*/

#ifdef MAC
#import <AVFoundation/AVFoundation.h>

#include <pthread.h>
#include <tcl.h>
#include "sized_types.h"
#include "vid_image.h"
#include "vid_code.h"
#include "mac_grab.h"

extern Tcl_Interp *interp;

static pthread_mutex_t grab_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t grab_cond = PTHREAD_COND_INITIALIZER;
static CMSampleBufferRef grab_frame = nil;
static uint8 *grab_data = NULL;
static size_t grab_data_len = 0;
static reconfigproc_t *reconfig = NULL;
static void *enc_state = NULL;
static int xmit_size = 0, xmit_color = 0, port = 0, width = 0, height = 0;

@interface VideoGrabber: NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

@property (strong) AVCaptureSession *captureSession;

+ (NSArray *) devices;

+ (BOOL) startFromPort: (int) port width: (int) width height: (int) height;

+ (void) stop;

+ (BOOL) running;

- (void) captureOutput: (AVCaptureOutput *) captureOutput
 didOutputSampleBuffer: (CMSampleBufferRef) sampleBuffer
        fromConnection: (AVCaptureConnection *) connection;

@end

@implementation VideoGrabber

static VideoGrabber *grabber = nil;

+ (NSArray *) devices {
    return [AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo];
}

+ (BOOL) startFromPort: (int) port width: (int) width height: (int) height {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
	grabber = [self new];
    });

    if ([self running])
	[self stop];

    AVCaptureDevice *videoDevice = [[self devices] objectAtIndex: port];
    AVCaptureDeviceFormat *format = nil;

    for (format in [videoDevice formats]) {
	CMFormatDescriptionRef desc = [format formatDescription];
	CMVideoDimensions dim = CMVideoFormatDescriptionGetDimensions(desc);

	if (width == dim.width && height == dim.height)
	    break;
    }

    if (format == nil) {
	NSLog(@"Can't find matching frame size");
	return NO;
    }

    if ([videoDevice lockForConfiguration: NULL] != YES) {
	NSLog(@"Unable to lock camera for configuration");
	return NO;
    }

    videoDevice.activeFormat = format;

    NSError *error = nil;
    AVCaptureDeviceInput *input = \
	[AVCaptureDeviceInput deviceInputWithDevice: videoDevice
					      error: &error];

    AVCaptureVideoDataOutput *output = [AVCaptureVideoDataOutput new];

    OSType pixelFormat = \
	xmit_color? kCVPixelFormatType_422YpCbCr8
		  : kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;

    NSDictionary *videoSettings = @{
	(id) kCVPixelBufferPixelFormatTypeKey: @(pixelFormat)};

    [output setVideoSettings: videoSettings];

    dispatch_queue_t queue = dispatch_queue_create("video_queue", NULL);
    [output setSampleBufferDelegate: grabber queue: queue];
    dispatch_release(queue);

    grabber.captureSession = [AVCaptureSession new];
    [grabber.captureSession addInput: input];
    [grabber.captureSession addOutput: output];
    [grabber.captureSession startRunning];
    [videoDevice unlockForConfiguration];

    return YES;
}

+ (void) stop {
    [grabber.captureSession stopRunning];
    [grabber.captureSession release];
    grabber.captureSession = nil;
}

+ (BOOL) running {
    return grabber.captureSession != nil;
}

- (void)  captureOutput: (AVCaptureOutput *) captureOutput
 didOutputSampleBuffer: (CMSampleBufferRef) sampleBuffer
        fromConnection: (AVCaptureConnection *) connection {
    pthread_mutex_lock(&grab_mutex);

    if (grab_frame != nil)
	CFRelease(grab_frame);

    grab_frame = (CMSampleBufferRef) CFRetain(sampleBuffer);

    pthread_cond_signal(&grab_cond);
    pthread_mutex_unlock(&grab_mutex);
}

@end

static int Mac_Grab(uint8 **datap, int *lenp)
{
    CVImageBufferRef image_buffer;
    size_t image_buffer_size;
    uint8 *data;

    pthread_mutex_lock(&grab_mutex);

    while (grab_frame == nil)
	pthread_cond_wait(&grab_cond, &grab_mutex);

    image_buffer = CMSampleBufferGetImageBuffer(grab_frame);

    CVPixelBufferLockBaseAddress(image_buffer, 0);

    if (xmit_color)
	data = CVPixelBufferGetBaseAddress(image_buffer);
    else
	data = CVPixelBufferGetBaseAddressOfPlane(image_buffer, 0);

    memcpy(grab_data, data, grab_data_len);
    CVPixelBufferUnlockBaseAddress(image_buffer, 0);

    CFRelease(grab_frame);
    grab_frame = nil;

    pthread_mutex_unlock(&grab_mutex);

    *datap = grab_data;
    *lenp = grab_data_len;
    return 1;
}

/*ARGSUSED*/
static char *Mac_TracePort(ClientData clientData, Tcl_Interp *interp,
			       char *name1, char *name2, int flags)
{
    port = atoi(Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY));

    if ([VideoGrabber running])
	[VideoGrabber startFromPort: port width: width height: height];

    return NULL;
}
 
int Mac_Probe(void)
{
    NSArray *devices = [VideoGrabber devices];

    if ([devices count] == 0)
	return 0;

    NSMutableArray *deviceNames = [NSMutableArray new];

    [devices enumerateObjectsUsingBlock: ^(id obj, NSUInteger idx, BOOL *stop) {
	NSString *name = [obj localizedName];
	[deviceNames addObject: [NSString stringWithFormat: @"{%@}", name]];
    }];

    NSString *deviceList = [deviceNames componentsJoinedByString: @" "];

    Tcl_SetVar(interp, "macPorts", (char *) [deviceList UTF8String],
	        TCL_GLOBAL_ONLY);
    Tcl_TraceVar(interp, "macPort", TCL_TRACE_WRITES, Mac_TracePort, NULL);

    return VID_GREYSCALE|VID_COLOR|VID_SMALL|VID_MEDIUM|VID_LARGE;
}

char *Mac_Attach(void)
{
    return ".grabControls.mac";
}

void Mac_Detach(void)
{
}

/*ARGSUSED*/
grabproc_t *Mac_Start(int grabtype, int min_framespacing, int config,
			  reconfigproc_t *reconfig, void *enc_state)
{

    if (grabtype != VIDIMAGE_UYVY && grabtype != VIDIMAGE_GREY)
	return NULL;

    xmit_size = config & VID_SIZEMASK;
    xmit_color = grabtype != VIDIMAGE_GREY;

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

    grab_data_len = xmit_color? width * height * 2 : width * height;

    if (grab_data != NULL)
	free(grab_data);

    grab_data = (uint8 *)malloc(grab_data_len);

    if (reconfig)
	(*reconfig)(enc_state, width, height);

    return [VideoGrabber startFromPort: port
				 width: width
				height: height] ? Mac_Grab : NULL;
}

void Mac_Stop(void)
{

    [VideoGrabber stop];

    if (grab_frame != nil) {
	CFRelease(grab_frame);
	grab_frame = nil;
    }

    if (grab_data != NULL) {
	free(grab_data); 
	grab_data = NULL;
    }
}
#endif /* MAC */
