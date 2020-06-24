#
# nv_procs.tcl - TK UI procedures used by NV
#

proc showRecvControls {source} {
    pack append .nvsource$source .nvsource$source.ctl {top expand fill}
    bind .nvsource$source <1> "hideRecvControls $source"
    bind .nvsource$source.video <1> "hideRecvControls $source"
    bind .nvsource$source <2> "hideRecvControls $source"
    bind .nvsource$source.video <2> "hideRecvControls $source"
    bind .nvsource$source <3> "hideRecvControls $source"
    bind .nvsource$source.video <3> "hideRecvControls $source"
}
 
proc hideRecvControls {source} {
    pack unpack .nvsource$source.ctl
    bind .nvsource$source <1> "showRecvControls $source"
    bind .nvsource$source.video <1> "showRecvControls $source"
    bind .nvsource$source <2> "showRecvControls $source"
    bind .nvsource$source.video <2> "showRecvControls $source"
    bind .nvsource$source <3> "showRecvControls $source"
    bind .nvsource$source.video <3> "showRecvControls $source"
}

proc enterSource {id} {
    global activeBgColor screenDepth
    .sources.canvas itemconfig $id -fill $activeBgColor
    if {$screenDepth == 1} {.sources.canvas itemconfig [expr $id+2] -fill white}
}

proc leaveSource {id} {
    global BgColor screenDepth
    .sources.canvas itemconfig $id -fill $BgColor
    if {$screenDepth == 1} {.sources.canvas itemconfig [expr $id+2] -fill black}
}

proc packSources {} {
    global source_list
    set width  [winfo width .sources.canvas]
    set height [winfo height .sources.canvas]
    if {$width == 0} {set width 1}
    set x 0
    set y 0
    foreach source $source_list {
	.sources.canvas coords rect$source $x $y [expr $x+100] [expr $y+95]
	.sources.canvas coords video$source [expr $x+50] [expr $y+38]
	.sources.canvas coords title$source [expr $x+50] [expr $y+76]
	incr x 100
	if {[expr $x+100] >= $width} {
	    set x 0
	    incr y 95
	}
    }
    if {$x != 0} {incr y 95}
    .sources.canvas config -scrollregion "0 0 $width $y"
}

proc showSource {source} {
    global nvgeom$source
    receiveVideo $source 1
    wm geometry .nvsource$source [set nvgeom$source]
    wm deiconify .nvsource$source
}

proc hideSource {source} {
    global nvgeom$source
    receiveVideo $source 0
    regexp "\[+-\].*" [wm geometry .nvsource$source] nvgeom$source
    wm withdraw .nvsource$source
}

proc toggleSource {source} {
    if [winfo ismapped .nvsource$source] {
	hideSource $source
    } else {
	showSource $source
    }
}

proc addSource {source address} {
    global source_list BgColor

    .sources.video$source scale 2

    set id [.sources.canvas create rect 0 0 0 0 -fill $BgColor -outline "" \
	-tags rect$source]
    .sources.canvas create window 0 0 -anchor center \
	-window .sources.video$source -tags video$source
    .sources.canvas create text 0 0 -anchor n -text $address -tags title$source

    lappend source_list $source
    packSources

    bind .sources.video$source <Any-Enter> "enterSource $id"
    bind .sources.video$source <Any-Leave> "leaveSource $id"
    bind .sources.video$source <1> "toggleSource $source"
    bind .sources.video$source <2> "toggleSource $source"
    bind .sources.video$source <3> "toggleSource $source"

    frame .nvsource$source.ctl -borderwidth 2 -relief raised

    frame .nvsource$source.ctl.sizebuttons
    pack append .nvsource$source.ctl.sizebuttons \
	[button .nvsource$source.ctl.sizebuttons.half   -text "Half" \
	    -command "changeSourceSize $source half"]   {left expand fill} \
	[button .nvsource$source.ctl.sizebuttons.normal -text "Normal" \
	    -command "changeSourceSize $source normal"] {left expand fill} \
	[button .nvsource$source.ctl.sizebuttons.double -text "Double" \
	    -command "changeSourceSize $source double"] {left expand fill}

    frame .nvsource$source.ctl.colorbuttons
    pack append .nvsource$source.ctl.colorbuttons \
	[button .nvsource$source.ctl.colorbuttons.grey  -text "Greyscale" \
	    -command "changeSourceColor $source grey"]  {left expand fill} \
	[button .nvsource$source.ctl.colorbuttons.color -text "Color" \
	    -command "changeSourceColor $source color"] {left expand fill}

    scale .nvsource$source.ctl.brightness -command "changeBrightness $source" \
	-label "Brightness" -from 0 -to 100 -orient horizontal

    scale .nvsource$source.ctl.contrast -command "changeContrast $source" \
	-label "Contrast" -from 0 -to 100 -orient horizontal

    frame .nvsource$source.ctl.address -relief flat
    pack append .nvsource$source.ctl.address \
	[label .nvsource$source.ctl.address.label -anchor w -width 10 \
	    -text "Address:"] {left filly} \
	[label .nvsource$source.ctl.address.value -anchor e -width 12 \
	    -text $address] {left filly} \
	[frame .nvsource$source.ctl.address.fill] {left expand fill}

    frame .nvsource$source.ctl.frames -relief flat
    pack append .nvsource$source.ctl.frames \
	[label .nvsource$source.ctl.frames.label -anchor w -width 10 \
	    -text "Frame rate:"] {left filly} \
	[label .nvsource$source.ctl.frames.fps -anchor e -width 12] \
	    {left filly} \
	[frame .nvsource$source.ctl.frames.fill] {left expand fill}

    frame .nvsource$source.ctl.bytes -relief flat
    pack append .nvsource$source.ctl.bytes \
	[label .nvsource$source.ctl.bytes.label -anchor w -width 10 \
	    -text "Bandwidth:"] {left filly} \
	[label .nvsource$source.ctl.bytes.bps -anchor e -width 12] \
	    {left filly} \
	[frame .nvsource$source.ctl.bytes.fill] {left expand fill}

    button .nvsource$source.ctl.capture -text "Capture" \
	-command "doSourceCapture $source"

    wm title .nvsource$source $address

    global nvgeom$source
    set nvgeom$source ""
    wm withdraw .nvsource$source

    pack append .nvsource$source.ctl \
	.nvsource$source.ctl.brightness {top fillx pady 4 frame s} \
	.nvsource$source.ctl.contrast {top fill} \
	.nvsource$source.ctl.address {top fill} \
	.nvsource$source.ctl.frames {top fill} \
	.nvsource$source.ctl.bytes {top fill} \
	.nvsource$source.ctl.sizebuttons {top fill} \
	.nvsource$source.ctl.colorbuttons {top fill} \
	.nvsource$source.ctl.capture {top fill}

    pack append .nvsource$source .nvsource$source.video {top frame center}

    wm protocol .nvsource$source WM_DELETE_WINDOW "hideSource $source"
    wm protocol .nvsource$source WM_TAKE_FOCUS "focus .nvsource$source"

    bind .nvsource$source <Control-c> "hideSource $source"
    bind .nvsource$source <Escape> "hideSource $source"
    bind .nvsource$source <q> "hideSource $source"

    bind .nvsource$source <h> "changeSourceSize $source half"
    bind .nvsource$source <n> "changeSourceSize $source normal"
    bind .nvsource$source <d> "changeSourceSize $source double"
    bind .nvsource$source <c> "doSourceCapture $source"

    bind .nvsource$source <Key-1> "changeSourceSize $source half"
    bind .nvsource$source <Key-2> "changeSourceSize $source normal"
    bind .nvsource$source <Key-3> "changeSourceSize $source double"

    bind .nvsource$source <1> "showRecvControls $source"
    bind .nvsource$source.video <1> "showRecvControls $source"
    bind .nvsource$source <2> "showRecvControls $source"
    bind .nvsource$source.video <2> "showRecvControls $source"
    bind .nvsource$source <3> "showRecvControls $source"
    bind .nvsource$source.video <3> "showRecvControls $source"

    if [catch ".nvsource[set source].ctl.brightness set \
		   [option get . brightness Nv]"] {
	.nvsource$source.ctl.brightness set $nvBrightness
    }

    if [catch ".nvsource[set source].ctl.contrast set \
		   [option get . contrast Nv]"] {
	.nvsource$source.ctl.contrast set $nvContrast
    }

    changeSourceSize  $source [option get . recvSize Nv]
    changeSourceColor $source [option get . recvColor Nv]
}

proc deleteSource {source} {
    global source_list

    receiveVideo $source 0
    .sources.canvas delete title$source
    .sources.canvas delete video$source
    .sources.canvas delete rect$source
    destroy .nvsource$source
    destroy .sources.video$source

    set ind [lsearch $source_list $source]
    set source_list [lreplace $source_list $ind $ind]
    packSources
}
