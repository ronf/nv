#
# nv_procs.tcl - TK UI procedures used by NV
#

proc updateXmitPanel {mask} {
    global nvXmitSize nvXmitColor

    set VID_SMALL 1
    set VID_MEDIUM 2
    set VID_LARGE 4
    set VID_GREYSCALE 8
    set VID_COLOR 16

    if {$mask & $VID_SMALL} {
	.sendControls.radios.size.small config -state normal
    } else {
	.sendControls.radios.size.small config -state disabled
	if {[string compare $nvXmitSize small] == 0} {set nvXmitSize medium}
    }

    if {$mask & $VID_MEDIUM} {
	.sendControls.radios.size.medium config -state normal
    } else {
	.sendControls.radios.size.medium config -state disabled
    }

    if {$mask & $VID_LARGE} {
	.sendControls.radios.size.large config -state normal
    } else {
	.sendControls.radios.size.large config -state disabled
	if {[string compare $nvXmitSize large] == 0} {set nvXmitSize medium}
    }

    if {$mask & $VID_GREYSCALE} {
	.sendControls.radios.color.grey config -state normal
    } else {
	.sendControls.radios.color.grey config -state disabled
	if {[string compare $nvXmitColor grey] == 0} {set nvXmitColor color}
    }

    if {$mask & $VID_COLOR} {
	.sendControls.radios.color.color config -state normal
    } else {
	.sendControls.radios.color.color config -state disabled
	if {[string compare $nvXmitColor color] == 0} {set nvXmitColor grey}
    }
}

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
	if {[expr $x+100] > $width} {
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
    if [string length [set nvgeom$source]] {
	wm geometry .nvsource$source [set nvgeom$source]
    }
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

proc addSource {source info color_ok} {
    global source_list nvBrightness nvContrast nvRecvColor nvRecvSize BgColor

    set id [.sources.canvas create rect 0 0 0 0 -fill $BgColor -outline "" \
	-tags rect$source]
    .sources.canvas create window 0 0 -anchor center \
	-window .sources.video$source -tags video$source
    .sources.canvas create text 0 0 -anchor n -text $info -tags title$source

    lappend source_list $source
    packSources

    bind .sources.video$source <Any-Enter> "enterSource $id"
    bind .sources.video$source <Any-Leave> "leaveSource $id"
    bind .sources.video$source <1> "toggleSource $source"
    bind .sources.video$source <2> "toggleSource $source"
    bind .sources.video$source <3> "toggleSource $source"

    frame .nvsource$source.ctl -borderwidth 2 -relief raised

    frame .nvsource$source.ctl.row1 -borderwidth 0 -relief flat

    scale .nvsource$source.ctl.row1.brightness -label "Brightness" \
	-from 0 -to 100 -orient horizontal -command "changeBrightness $source"

    scale .nvsource$source.ctl.row1.contrast -label "Contrast" \
	-from 0 -to 100 -orient horizontal -command "changeContrast $source"

    pack append .nvsource$source.ctl.row1 \
	.nvsource$source.ctl.row1.brightness {left expand fill} \
	[frame .nvsource$source.ctl.row1.fill -width 10 -height 1] {left fill} \
	.nvsource$source.ctl.row1.contrast {left expand fill}

    frame .nvsource$source.ctl.row2 -borderwidth 0 -relief flat

    frame .nvsource$source.ctl.row2.size
    pack append .nvsource$source.ctl.row2.size \
	[radiobutton .nvsource$source.ctl.row2.size.half   -text "Half Size" \
	    -relief flat -width 12 -variable size$source -value half \
	    -anchor w -command "checkSourceSize $source"]   {top fill} \
	[radiobutton .nvsource$source.ctl.row2.size.normal -text "Normal Size" \
	    -relief flat -width 12 -variable size$source -value normal \
	    -anchor w -command "checkSourceSize $source"] {top fill} \
	[radiobutton .nvsource$source.ctl.row2.size.double -text "Double Size" \
	    -relief flat -width 12 -variable size$source -value double \
	    -anchor w -command "checkSourceSize $source"] {top fill}

    frame .nvsource$source.ctl.row2.color
    pack append .nvsource$source.ctl.row2.color \
	[radiobutton .nvsource$source.ctl.row2.color.grey  -text "Greyscale" \
	    -relief flat -width 12 -variable color$source -value grey \
	    -anchor w -command "checkSourceColor $source"]  {top fill} \
	[radiobutton .nvsource$source.ctl.row2.color.color -text "Color" \
	    -relief flat -width 12 -variable color$source -value color \
	    -anchor w -command "checkSourceColor $source"] {top fill}

    pack append .nvsource$source.ctl.row2 \
	[frame .nvsource$source.ctl.fill1] {left expand fill} \
	.nvsource$source.ctl.row2.size  {left fill} \
	[frame .nvsource$source.ctl.fill2] {left expand fill} \
	.nvsource$source.ctl.row2.color {left fill} \
	[frame .nvsource$source.ctl.fill3] {left expand fill}

    frame .nvsource$source.ctl.info -relief flat
    pack append .nvsource$source.ctl.info \
	[label .nvsource$source.ctl.info.label -anchor w -width 10 \
	    -text "Source:"] {left filly} \
	[label .nvsource$source.ctl.info.value -anchor w -text $info] \
	    {left filly} \
	[frame .nvsource$source.ctl.info.fill] {left expand fill}

    frame .nvsource$source.ctl.frames -relief flat
    pack append .nvsource$source.ctl.frames \
	[label .nvsource$source.ctl.frames.label -anchor w -width 10 \
	    -text "Frame rate:"] {left filly} \
	[label .nvsource$source.ctl.frames.fps -anchor e -width 12] \
	    {left filly} \
	[label .nvsource$source.ctl.frames.shown -anchor w -width 12] \
	    {left filly} \
	[frame .nvsource$source.ctl.frames.fill] {left expand fill}

    frame .nvsource$source.ctl.bytes -relief flat
    pack append .nvsource$source.ctl.bytes \
	[label .nvsource$source.ctl.bytes.label -anchor w -width 10 \
	    -text "Bandwidth:"] {left filly} \
	[label .nvsource$source.ctl.bytes.bps -anchor e -width 12] \
	    {left filly} \
	[label .nvsource$source.ctl.bytes.loss -anchor w -width 12] \
	    {left filly} \
	[frame .nvsource$source.ctl.bytes.fill] {left expand fill}

    button .nvsource$source.ctl.capture -text "Capture" \
	-command "doSourceCapture $source"

    wm title .nvsource$source $info

    global nvgeom$source
    set nvgeom$source ""
    wm withdraw .nvsource$source

    pack append .nvsource$source.ctl \
	.nvsource$source.ctl.row1 {top fillx pady 4 frame s} \
	.nvsource$source.ctl.row2 {top fill} \
	.nvsource$source.ctl.info {top fill} \
	.nvsource$source.ctl.frames {top fill} \
	.nvsource$source.ctl.bytes {top fill} \
	.nvsource$source.ctl.capture {top fill}

    pack append .nvsource$source .nvsource$source.video {top frame center}

    wm protocol .nvsource$source WM_DELETE_WINDOW "hideSource $source"
    wm protocol .nvsource$source WM_TAKE_FOCUS "focus .nvsource$source"

    bind .nvsource$source <Control-c> "hideSource $source"
    bind .nvsource$source <Escape> "hideSource $source"
    bind .nvsource$source <q> "hideSource $source"

    bind .nvsource$source <h> "set size$source half; checkSourceSize $source"
    bind .nvsource$source <n> "set size$source normal; checkSourceSize $source"
    bind .nvsource$source <d> "set size$source double; checkSourceSize $source"
    bind .nvsource$source <c> "doSourceCapture $source"

    bind .nvsource$source <Key-1> \
	"set size$source half; checkSourceSize $source"
    bind .nvsource$source <Key-2> \
	"set size$source normal; checkSourceSize $source"
    bind .nvsource$source <Key-3> \
	"set size$source double; checkSourceSize $source"

    bind .nvsource$source <1> "showRecvControls $source"
    bind .nvsource$source.video <1> "showRecvControls $source"
    bind .nvsource$source <2> "showRecvControls $source"
    bind .nvsource$source.video <2> "showRecvControls $source"
    bind .nvsource$source <3> "showRecvControls $source"
    bind .nvsource$source.video <3> "showRecvControls $source"

    .nvsource$source.ctl.row1.brightness set $nvBrightness
    changeBrightness $source [.nvsource$source.ctl.row1.brightness get]

    .nvsource$source.ctl.row1.contrast set $nvContrast
    changeContrast $source [.nvsource$source.ctl.row1.contrast get]

    global size$source color$source
    set size$source $nvRecvSize
    checkSourceSize  $source
    if {$color_ok} {
	set color$source $nvRecvColor
    } else {
	set color$source grey
	.nvsource$source.ctl.row2.color.grey config -state disabled
	.nvsource$source.ctl.row2.color.color config -state disabled
    }
    checkSourceColor $source
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

proc setSourceInfo {source info} {
    .nvsource$source.ctl.info.value config -text $info
}
