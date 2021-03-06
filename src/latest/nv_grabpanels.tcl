#
# nv_grabpanels.tcl - TK interface descriptions for video grabber controls
#
set macPort 0

frame .grabControls.mac

label .grabControls.mac.title -text "Mac Video controls"

frame .grabControls.mac.radios

frame .grabControls.mac.radios.port

set idx 0
if {[info exists macPorts]} {
    foreach port $macPorts {
	pack append .grabControls.mac.radios.port \
	    [radiobutton .grabControls.mac.radios.port.port$idx -text $port \
		-relief flat -variable macPort -value $idx \
		-anchor w] {top fill}
	incr idx
    }
}

pack append .grabControls.mac.radios \
    [frame .grabControls.mac.radios.fill1] {left expand fill} \
    .grabControls.mac.radios.port {left fill} \
    [frame .grabControls.mac.radios.fill2] {left expand fill}

pack append .grabControls.mac \
    .grabControls.mac.title {top fill} \
    .grabControls.mac.radios {top fill}

set v4l2Port 0

frame .grabControls.v4l2

label .grabControls.v4l2.title -text "Video for Linux 2 controls"

frame .grabControls.v4l2.radios

frame .grabControls.v4l2.radios.port

set idx 0
if {[info exists v4l2Ports]} {
    foreach port $v4l2Ports {
	pack append .grabControls.v4l2.radios.port \
	    [radiobutton .grabControls.v4l2.radios.port.port$idx -text $port \
		-relief flat -variable v4l2Port -value $idx \
		-anchor w] {top fill}
	incr idx
    }
}

pack append .grabControls.v4l2.radios \
    [frame .grabControls.v4l2.radios.fill1] {left expand fill} \
    .grabControls.v4l2.radios.port {left fill} \
    [frame .grabControls.v4l2.radios.fill2] {left expand fill}

pack append .grabControls.v4l2 \
    .grabControls.v4l2.title {top fill} \
    .grabControls.v4l2.radios {top fill}

set pvidPort 2

frame .grabControls.parcvid

label .grabControls.parcvid.title -text "PARC Video controls"

frame .grabControls.parcvid.radios

frame .grabControls.parcvid.radios.port
pack append .grabControls.parcvid.radios.port \
    [radiobutton .grabControls.parcvid.radios.port.port1 -text "SVideo 1" \
	-relief flat -width 10 -variable pvidPort -value 0 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.parcvid.radios.port.port2 -text "SVideo 2" \
	-relief flat -width 10 -variable pvidPort -value 1 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.parcvid.radios.port.svideo -text "Composite" \
	-relief flat -width 10 -variable pvidPort -value 2 -anchor w] \
	{top fill}

pack append .grabControls.parcvid.radios \
    [frame .grabControls.parcvid.radios.fill1] {left expand fill} \
    .grabControls.parcvid.radios.port {left fill} \
    [frame .grabControls.parcvid.radios.fill2] {left expand fill}

pack append .grabControls.parcvid \
    .grabControls.parcvid.title {top fill} \
    .grabControls.parcvid.radios {top fill}

set vpixPort 1
set vpixFmt 2

frame .grabControls.videopix

label .grabControls.videopix.title -text "VideoPix controls"

frame .grabControls.videopix.radios

frame .grabControls.videopix.radios.port
pack append .grabControls.videopix.radios.port \
    [radiobutton .grabControls.videopix.radios.port.port1 -text "Port 1" \
	-relief flat -width 10 -variable vpixPort -value 1 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.videopix.radios.port.port2 -text "Port 2" \
	-relief flat -width 10 -variable vpixPort -value 2 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.videopix.radios.port.svideo -text "S-Video" \
	-relief flat -width 10 -variable vpixPort -value 3 -anchor w] \
	{top fill}

frame .grabControls.videopix.radios.format
pack append .grabControls.videopix.radios.format \
    [radiobutton .grabControls.videopix.radios.format.ntsc -text "NTSC" \
	-relief flat -width 10 -variable vpixFmt -value 0 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.videopix.radios.format.pal -text "PAL" \
	-relief flat -width 10 -variable vpixFmt -value 1 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.videopix.radios.format.auto -text "Auto" \
	-relief flat -width 10 -variable vpixFmt -value 2 -anchor w] \
	{top fill}

pack append .grabControls.videopix.radios \
    [frame .grabControls.videopix.radios.fill1] {left expand fill} \
    .grabControls.videopix.radios.port {left fill} \
    [frame .grabControls.videopix.radios.fill2] {left expand fill} \
    .grabControls.videopix.radios.format {left fill} \
    [frame .grabControls.videopix.radios.fill3] {left expand fill}

pack append .grabControls.videopix \
    .grabControls.videopix.title {top fill} \
    .grabControls.videopix.radios {top fill}

#
# Parallax grabber controls
#
set plxBrightness 128
set plxContrast 128
set plxHue 128
set plxSaturation 128
set plxChan 0
set plxStd 0
set plxFmt 0

frame .grabControls.parallax

label .grabControls.parallax.title -text "Parallax controls"

frame .grabControls.parallax.row1
pack append .grabControls.parallax.row1 \
    [scale .grabControls.parallax.row1.brightness -label "Brightness" \
	-from 0 -to 255 -orient horizontal -command "set plxBrightness"] \
	{left expand fill} \
    [frame .grabControls.parallax.row1.fill -height 1 -width 10] {left fill} \
    [scale .grabControls.parallax.row1.contrast -label "Contrast" \
	-from 0 -to 255 -orient horizontal -command "set plxContrast"] \
	{left expand fill}

frame .grabControls.parallax.row2
pack append .grabControls.parallax.row2 \
    [scale .grabControls.parallax.row2.hue -label "Hue" \
	-from 0 -to 255 -orient horizontal -command "set plxHue"] \
	{left expand fill} \
    [frame .grabControls.parallax.row2.fill -height 1 -width 10] {left fill} \
    [scale .grabControls.parallax.row2.saturation -label "Saturation" \
	-from 0 -to 255 -orient horizontal -command "set plxSaturation"] \
	{left expand fill}

frame .grabControls.parallax.row3

frame .grabControls.parallax.row3.input
pack append .grabControls.parallax.row3.input \
    [label .grabControls.parallax.row3.input.label -text "Video input:" \
	-anchor w] {top fill} \
    [radiobutton .grabControls.parallax.row3.input.chan0 -text "Channel 0" \
	-relief flat -width 10 -anchor w -value 0 -variable plxChan] \
	{top fill} \
    [radiobutton .grabControls.parallax.row3.input.chan1 -text "Channel 1" \
	-relief flat -width 10 -anchor w -value 1 -variable plxChan] \
	{top fill}

frame .grabControls.parallax.row3.std
pack append .grabControls.parallax.row3.std \
    [label .grabControls.parallax.row3.std.label -text "Video standard:" \
	-anchor w] {top fill} \
    [radiobutton .grabControls.parallax.row3.std.ntsc -text "NTSC" \
	-relief flat -width 10 -anchor w -value 0 -variable plxStd] \
	{top fill} \
    [radiobutton .grabControls.parallax.row3.std.pal -text "PAL" \
	-relief flat -width 10 -anchor w -value 1 -variable plxStd] \
	{top fill} \
    [radiobutton .grabControls.parallax.row3.std.secam -text "SECAM" \
	-relief flat -width 10 -anchor w -value 2 -variable plxStd] \
	{top fill}

frame .grabControls.parallax.row3.fmt
pack append .grabControls.parallax.row3.fmt \
    [label .grabControls.parallax.row3.fmt.label -text "Input format:" \
	-anchor w] {top fill} \
    [radiobutton .grabControls.parallax.row3.fmt.comp -text "Composite" \
	-relief flat -width 10 -anchor w -value 0 -variable plxFmt] \
	{top fill} \
    [radiobutton .grabControls.parallax.row3.fmt.yc -text "Y/C" \
	-relief flat -width 10 -anchor w -value 1 -variable plxFmt] \
	{top fill} \
    [radiobutton .grabControls.parallax.row3.fmt.rgb -text "RGB" \
	-relief flat -width 10 -anchor w -value 2 -variable plxFmt]\
	{top fill} \
    [radiobutton .grabControls.parallax.row3.fmt.yuv -text "YUV" \
	-relief flat -width 10 -anchor w -value 3 -variable plxFmt]\
	{top fill}

pack append .grabControls.parallax.row3 \
    .grabControls.parallax.row3.input {left fill} \
    [frame .grabControls.parallax.row3.fill1] {left expand fill} \
    .grabControls.parallax.row3.std {left fill} \
    [frame .grabControls.parallax.row3.fill2] {left expand fill} \
    .grabControls.parallax.row3.fmt {left fill}

pack append .grabControls.parallax \
    .grabControls.parallax.title {top fill} \
    .grabControls.parallax.row1 {top fill} \
    .grabControls.parallax.row2 {top fill} \
    .grabControls.parallax.row3 {top fillx frame s pady 5}

#
# SunVideo (rtvc).  Same input ports as a VideoPix but numbered
#   differently (SVideo is 0, not 3).  The format (PAL/NTSC/UNKNOWN)
#   can be read but not set;  we don't have anything on the panel for
#   it at present.  We could also add panel items for the IMAGE_SKIP
#   and MAX_BUFFERS properties and for the do/don't rescale colours
#   choice, but for now there's just the port selector.
#
set sunvidPort 1

frame .grabControls.sunvideo

label .grabControls.sunvideo.title -text "SunVideo controls"

frame .grabControls.sunvideo.radios

frame .grabControls.sunvideo.radios.port
pack append .grabControls.sunvideo.radios.port \
    [radiobutton .grabControls.sunvideo.radios.port.port1 -text "Port 1" \
	-relief flat -width 10 -variable sunvidPort -value 1 -anchor w \
	-command {global sunvidPort; \
		xilSetDevAttrInt rtvc0 PORT_V $sunvidPort}] {top fill} \
    [radiobutton .grabControls.sunvideo.radios.port.port2 -text "Port 2" \
	-relief flat -width 10 -variable sunvidPort -value 2 -anchor w \
	-command {global sunvidPort; \
		xilSetDevAttrInt rtvc0 PORT_V $sunvidPort}] {top fill} \
    [radiobutton .grabControls.sunvideo.radios.port.svideo -text "S-Video" \
	-relief flat -width 10 -variable sunvidPort -value 0 -anchor w \
	-command {global sunvidPort; \
		xilSetDevAttrInt rtvc0 PORT_V $sunvidPort}] {top fill}

pack append .grabControls.sunvideo.radios \
    [frame .grabControls.sunvideo.radios.fill1] {left expand fill} \
    .grabControls.sunvideo.radios.port {left fill} \
    [frame .grabControls.sunvideo.radios.fill2] {left expand fill}

pack append .grabControls.sunvideo \
    .grabControls.sunvideo.title {top fill} \
    .grabControls.sunvideo.radios {top fill}

#
# SGI Video Library input source controls
#
#  Currently supports Indy VINO only
#  To do: support Galileo family

set vinoInput default

frame .grabControls.sgiIndyVINO

label .grabControls.sgiIndyVINO.title -text "SGIVL controls"

frame .grabControls.sgiIndyVINO.radios
frame .grabControls.sgiIndyVINO.radios.input1
frame .grabControls.sgiIndyVINO.radios.input2

pack append .grabControls.sgiIndyVINO.radios.input1 \
    [radiobutton .grabControls.sgiIndyVINO.radios.input1.indycam \
	-text "IndyCam" -state normal \
	-relief flat -anchor w -value indycam -variable vinoInput] \
	{top fill} 

pack append .grabControls.sgiIndyVINO.radios.input2 \
    [radiobutton .grabControls.sgiIndyVINO.radios.input2.analogSvideo \
	-text "Analog S-Video" -state normal \
	-relief flat -anchor w -value svideo -variable vinoInput] \
	{top fill} \
    [radiobutton .grabControls.sgiIndyVINO.radios.input2.analogComp \
	-text "Analog Composite" -state normal \
	-relief flat -anchor w -value comp -variable vinoInput] \
	{top fill}

pack append .grabControls.sgiIndyVINO.radios \
    [frame .grabControls.sgiIndyVINO.radios.fill1] {left expand fill} \
    .grabControls.sgiIndyVINO.radios.input1 {left fill} \
    [frame .grabControls.sgiIndyVINO.radios.fill2] {left expand fill} \
    .grabControls.sgiIndyVINO.radios.input2 {left fill} \
    [frame .grabControls.sgiIndyVINO.radios.fill3] {left expand fill}

pack append .grabControls.sgiIndyVINO \
    .grabControls.sgiIndyVINO.title {top fill} \
    .grabControls.sgiIndyVINO.radios {top fill}

proc sgivlEnableControls {} {
    .grabControls.sgiIndyVINO.radios.input1.indycam      config -state normal
    .grabControls.sgiIndyVINO.radios.input2.analogSvideo config -state normal
    .grabControls.sgiIndyVINO.radios.input2.analogComp   config -state normal
}

proc sgivlDisableControls {} {
    .grabControls.sgiIndyVINO.radios.input1.indycam      config -state disabled
    .grabControls.sgiIndyVINO.radios.input2.analogSvideo config -state disabled
    .grabControls.sgiIndyVINO.radios.input2.analogComp   config -state disabled
}

#
# HP VideoLive grabber controls
#
frame .grabControls.videolive

label .grabControls.videolive.title -text "Videolive controls"

frame .grabControls.videolive.row1
pack append .grabControls.videolive.row1 \
    [scale .grabControls.videolive.row1.brightness -label "Brightness" -state disabled \
	-from -100 -to 100 -orient horizontal -command "set vliveBrightness"] \
	{left expand fill} \
    [frame .grabControls.videolive.row1.fill -height 1 -width 10] {left fill} \
    [scale .grabControls.videolive.row1.contrast -label "Contrast"  -state disabled\
	-from -100 -to 100 -orient horizontal -command "set vliveContrast"] \
	{left expand fill}

frame .grabControls.videolive.row2
pack append .grabControls.videolive.row2 \
    [scale .grabControls.videolive.row2.hue -label "Hue" -state disabled \
	-from -100 -to 100 -orient horizontal -command "set vliveHue"] \
	{left expand fill} \
    [frame .grabControls.videolive.row2.fill -height 1 -width 10] {left fill} \
    [scale .grabControls.videolive.row2.saturation -label "Saturation" -state disabled \
	-from -100 -to 100 -orient horizontal -command "set vliveSaturation"] \
	{left expand fill}

frame .grabControls.videolive.row3

frame .grabControls.videolive.row3.std
pack append .grabControls.videolive.row3.std \
    [label .grabControls.videolive.row3.std.label -text "Video standard:" \
	-anchor w] {top fill} \
    [radiobutton .grabControls.videolive.row3.std.ntsc -text "NTSC" -state disabled \
	-relief flat -width 10 -anchor w -value ntsc -variable vliveStd] \
	{top fill} \
    [radiobutton .grabControls.videolive.row3.std.pal -text "PAL" -state disabled \
	-relief flat -width 10 -anchor w -value pal -variable vliveStd] \
	{top fill} \
    [radiobutton .grabControls.videolive.row3.std.secam -text "SECAM" -state disabled \
	-relief flat -width 10 -anchor w -value secam -variable vliveStd] \
	{top fill}

frame .grabControls.videolive.row3.input
pack append .grabControls.videolive.row3.input \
    [label .grabControls.videolive.row3.input.label -text "Input:" \
	-anchor w] {top fill} \
    [radiobutton .grabControls.videolive.row3.input.comp -text "Composite 1" -state disabled \
	-relief flat -width 12 -anchor w -value composite \
	-variable vliveInput] {top fill} \
    [radiobutton .grabControls.videolive.row3.input.comp2 -text "Composite 2" -state disabled \
	-relief flat -width 12 -anchor w -value composite2 \
	-variable vliveInput] {top fill} \
    [radiobutton .grabControls.videolive.row3.input.rgb -text "RGB" -state disabled \
	-relief flat -width 12 -anchor w -value rgb \
	-variable vliveInput] {top fill} \
    [radiobutton .grabControls.videolive.row3.input.svideo -text "S-Video" -state disabled \
	-relief flat -width 12 -anchor w -value svideo \
	-variable vliveInput] {top fill}

pack append .grabControls.videolive.row3 \
    [frame .grabControls.videolive.row3.fill1] {left expand fill} \
    .grabControls.videolive.row3.std {left fill} \
    [frame .grabControls.videolive.row3.fill2] {left expand fill} \
    .grabControls.videolive.row3.input {left fill} \
    [frame .grabControls.videolive.row3.fill3] {left expand fill}

pack append .grabControls.videolive \
    .grabControls.videolive.title {top fill} \
    .grabControls.videolive.row1 {top fill} \
    .grabControls.videolive.row2 {top fill} \
    .grabControls.videolive.row3 {top fillx frame s pady 5}

proc videoliveGrabEnableControls {} {
    .grabControls.videolive.row1.brightness config -state normal
    .grabControls.videolive.row1.contrast config -state normal
    .grabControls.videolive.row2.hue config -state normal
    .grabControls.videolive.row2.saturation config -state normal
    .grabControls.videolive.row3.std.ntsc config -state normal
    .grabControls.videolive.row3.std.pal config -state normal
    .grabControls.videolive.row3.std.secam config -state normal
    .grabControls.videolive.row3.input.comp config -state normal
    .grabControls.videolive.row3.input.comp2 config -state normal
    .grabControls.videolive.row3.input.rgb config -state normal
    .grabControls.videolive.row3.input.svideo config -state normal
}

proc videoliveGrabDisableControls {} {
    .grabControls.videolive.row1.brightness config -state disabled
    .grabControls.videolive.row1.contrast config -state disabled
    .grabControls.videolive.row2.hue config -state disabled
    .grabControls.videolive.row2.saturation config -state disabled
    .grabControls.videolive.row3.std.ntsc config -state disabled
    .grabControls.videolive.row3.std.pal config -state disabled
    .grabControls.videolive.row3.std.secam config -state disabled
    .grabControls.videolive.row3.input.comp config -state disabled
    .grabControls.videolive.row3.input.comp2 config -state disabled
    .grabControls.videolive.row3.input.rgb config -state disabled
    .grabControls.videolive.row3.input.svideo config -state disabled
}

#
# IBM VCA Grabber controls
#
set ibmvcaPort 0

frame .grabControls.ibmvca

label .grabControls.ibmvca.title -text "IBM VCA controls"

frame .grabControls.ibmvca.radios

frame .grabControls.ibmvca.radios.port
pack append .grabControls.ibmvca.radios.port \
    [radiobutton .grabControls.ibmvca.radios.port.port1 -text "Composite" \
	-relief flat -width 10 -variable ibmvcaPort -value 0 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.ibmvca.radios.port.port2 -text "SVideo" \
	-relief flat -width 10 -variable ibmvcaPort -value 1 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.ibmvca.radios.port.svideo -text "RGB" \
	-relief flat -width 10 -variable ibmvcaPort -value 2 -anchor w] \
	{top fill}

pack append .grabControls.ibmvca.radios \
    [frame .grabControls.ibmvca.radios.fill1] {left expand fill} \
    .grabControls.ibmvca.radios.port {left fill} \
    [frame .grabControls.ibmvca.radios.fill2] {left expand fill}

pack append .grabControls.ibmvca \
    .grabControls.ibmvca.title {top fill} \
    .grabControls.ibmvca.radios {top fill}

#
# J300 grabber controls
#
proc j300FormatSVideo {} {
    .grabControls.j300.radios.format.ntsc config -text "SVideo 525"
    .grabControls.j300.radios.format.pal config -text "SVideo 625"
    .grabControls.j300.radios.format.secam config -state disabled -text ""
}

proc j300FormatComposite {} {
    .grabControls.j300.radios.format.ntsc config -text "NTSC"
    .grabControls.j300.radios.format.pal config -text "PAL"
    .grabControls.j300.radios.format.secam config -state normal -text "SECAM"
}

proc j300EnableControls {} {
    global j300Port

    .grabControls.j300.radios.port.svideo config -state normal
    .grabControls.j300.radios.port.port1 config -state normal
    .grabControls.j300.radios.port.port2 config -state normal
    .grabControls.j300.radios.format.ntsc config -state normal
    .grabControls.j300.radios.format.pal config -state normal
    if {$j300Port != 0} {
	.grabControls.j300.radios.format.secam config -state normal
    }
}

proc j300DisableControls {} {
    .grabControls.j300.radios.port.svideo config -state disabled
    .grabControls.j300.radios.port.port1 config -state disabled
    .grabControls.j300.radios.port.port2 config -state disabled
    .grabControls.j300.radios.format.ntsc config -state disabled
    .grabControls.j300.radios.format.pal config -state disabled
    .grabControls.j300.radios.format.secam config -state disabled
}

frame .grabControls.j300

label .grabControls.j300.title -text "J300 Video Controls"

frame .grabControls.j300.radios

frame .grabControls.j300.radios.port
pack append .grabControls.j300.radios.port \
    [radiobutton .grabControls.j300.radios.port.svideo -text "S-Video" \
	-relief flat -width 12 -variable j300Port -value 0 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.j300.radios.port.port1 -text "Composite 1" \
	-relief flat -width 12 -variable j300Port -value 1 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.j300.radios.port.port2 -text "Composite 2" \
	-relief flat -width 12 -variable j300Port -value 2 -anchor w] \
	{top fill}

frame .grabControls.j300.radios.format
pack append .grabControls.j300.radios.format \
    [radiobutton .grabControls.j300.radios.format.ntsc -text "NTSC" \
	-relief flat -width 12 -variable j300Format -value 1 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.j300.radios.format.pal -text "PAL" \
	-relief flat -width 12 -variable j300Format -value 2 -anchor w] \
	{top fill} \
    [radiobutton .grabControls.j300.radios.format.secam -text "SECAM" \
	-relief flat -width 12 -variable j300Format -value 3 -anchor w] \
	{top fill}

pack append .grabControls.j300.radios \
    [frame .grabControls.j300.radios.fill1] {left expand fill} \
    .grabControls.j300.radios.port {left fill} \
    [frame .grabControls.j300.radios.fill2] {left expand fill} \
    .grabControls.j300.radios.format {left fill} \
    [frame .grabControls.j300.radios.fill3] {left expand fill}

pack append .grabControls.j300 \
    .grabControls.j300.title {top fill} \
    .grabControls.j300.radios {top fill}

#
# X11 Grabber controls
#
set x11grabMode pointer

proc x11grabUpdatePos {x y w h} {
    if {[string compare $x [.grabControls.x11grab.row1.pos.x.e get]] != 0} {
	.grabControls.x11grab.row1.pos.x.e delete 0 end
	.grabControls.x11grab.row1.pos.x.e insert 0 $x
    }
    if {[string compare $y [.grabControls.x11grab.row1.pos.y.e get]] != 0} {
	.grabControls.x11grab.row1.pos.y.e delete 0 end
	.grabControls.x11grab.row1.pos.y.e insert 0 $y
    }
    if {[string compare $w [.grabControls.x11grab.row1.pos.w.e get]] != 0} {
	.grabControls.x11grab.row1.pos.w.e delete 0 end
	.grabControls.x11grab.row1.pos.w.e insert 0 $w
    }
    if {[string compare $h [.grabControls.x11grab.row1.pos.h.e get]] != 0} {
	.grabControls.x11grab.row1.pos.h.e delete 0 end
	.grabControls.x11grab.row1.pos.h.e insert 0 $h
    }
}

frame .grabControls.x11grab

label .grabControls.x11grab.title -text "X11 Grabber controls"

frame .grabControls.x11grab.row1

frame .grabControls.x11grab.row1.mode
pack append .grabControls.x11grab.row1.mode \
    [radiobutton .grabControls.x11grab.row1.mode.fixed -text "Fixed" \
	-relief flat -width 10 -anchor w -value fixed -variable x11grabMode] \
	{top fill} \
    [radiobutton .grabControls.x11grab.row1.mode.pointer -text "Pointer" \
	-relief flat -width 10 -anchor w -value pointer -variable x11grabMode] \
	{top fill} \
    [radiobutton .grabControls.x11grab.row1.mode.window -text "Window" \
	-relief flat -width 10 -anchor w -value window -variable x11grabMode] \
	{top fill}

frame .grabControls.x11grab.row1.pos

frame .grabControls.x11grab.row1.pos.x
pack append .grabControls.x11grab.row1.pos.x \
    [label .grabControls.x11grab.row1.pos.x.l -text "X:" -width 7 \
	-anchor e] {left filly} \
    [entry .grabControls.x11grab.row1.pos.x.e -relief flat -width 5] \
	{left filly}

frame .grabControls.x11grab.row1.pos.y
pack append .grabControls.x11grab.row1.pos.y \
    [label .grabControls.x11grab.row1.pos.y.l -text "Y:" -width 7 \
	-anchor e] {left filly} \
    [entry .grabControls.x11grab.row1.pos.y.e -relief flat -width 5] \
	{left filly}

frame .grabControls.x11grab.row1.pos.w
pack append .grabControls.x11grab.row1.pos.w \
    [label .grabControls.x11grab.row1.pos.w.l -text "Width:" -width 7 \
	-anchor e] {left filly} \
    [entry .grabControls.x11grab.row1.pos.w.e -relief flat -width 5] \
	{left filly}

frame .grabControls.x11grab.row1.pos.h
pack append .grabControls.x11grab.row1.pos.h \
    [label .grabControls.x11grab.row1.pos.h.l -text "Height:" -width 7 \
	-anchor e] {left filly} \
    [entry .grabControls.x11grab.row1.pos.h.e -relief flat -width 5] \
	{left filly}

pack append .grabControls.x11grab.row1.pos \
    .grabControls.x11grab.row1.pos.x {top fill} \
    .grabControls.x11grab.row1.pos.y {top fill} \
    .grabControls.x11grab.row1.pos.w {top fill} \
    .grabControls.x11grab.row1.pos.h {top fill}

bind .grabControls.x11grab.row1.pos.x.e <Return> {
    focus .
    .grabControls.x11grab.row1.pos.x.e select clear
}

bind .grabControls.x11grab.row1.pos.y.e <Return> {
    focus .
    .grabControls.x11grab.row1.pos.y.e select clear
}

bind .grabControls.x11grab.row1.pos.w.e <Return> {
    focus .
    .grabControls.x11grab.row1.pos.w.e select clear
}

bind .grabControls.x11grab.row1.pos.h.e <Return> {
    focus .
    .grabControls.x11grab.row1.pos.h.e select clear
}

bind .grabControls.x11grab.row1.pos.x.e <FocusIn> \
    {global x11grabMode; set x11grabMode fixed}
bind .grabControls.x11grab.row1.pos.y.e <FocusIn> \
    {global x11grabMode; set x11grabMode fixed}
bind .grabControls.x11grab.row1.pos.w.e <FocusIn> \
    {global x11grabMode; set x11grabMode fixed}
bind .grabControls.x11grab.row1.pos.h.e <FocusIn> \
    {global x11grabMode; set x11grabMode fixed}

bind .grabControls.x11grab.row1.pos.x.e <FocusOut> \
    {x11grabSetX [.grabControls.x11grab.row1.pos.x.e get]}
bind .grabControls.x11grab.row1.pos.y.e <FocusOut> \
    {x11grabSetY [.grabControls.x11grab.row1.pos.y.e get]}
bind .grabControls.x11grab.row1.pos.w.e <FocusOut> \
    {x11grabSetW [.grabControls.x11grab.row1.pos.w.e get]}
bind .grabControls.x11grab.row1.pos.h.e <FocusOut> \
    {x11grabSetH [.grabControls.x11grab.row1.pos.h.e get]}

pack append .grabControls.x11grab.row1 \
    [frame .grabControls.x11grab.row1.fill1] {left expand fill} \
    .grabControls.x11grab.row1.mode {left fill} \
    [frame .grabControls.x11grab.row1.fill2] {left expand fill} \
    .grabControls.x11grab.row1.pos {left fill} \
    [frame .grabControls.x11grab.row1.fill3] {left expand fill}

frame .grabControls.x11grab.row2
pack append .grabControls.x11grab.row2 \
    [button .grabControls.x11grab.row2.region -text "Select Region" \
	-command {after 0 x11grabSetRegion}] {left expand fill} \
    [button .grabControls.x11grab.row2.window -text "Select Window" \
	-command {after 0 x11grabSetWindow}] {left expand fill}

pack append .grabControls.x11grab \
    .grabControls.x11grab.title {top fill} \
    .grabControls.x11grab.row1 {top fill} \
    .grabControls.x11grab.row2 {top fill}
