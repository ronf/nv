#
# nv_init.tcl - TK interface definition for NV
#

set copy 0
set screenDepth [winfo screendepth .]
set source_list ""
set version "4.0beta"

proc max {args} {
    set m [lindex $args 0]
    foreach i $args {
	if {$i > $m} {set m $i}
    }
    return $m
}

proc envVal { envValName } {
    global env
    if [info exists env($envValName)] {
	return $env($envValName)
    } else {
	return {}
    }
}
 
proc doSubst { s pat rep } {
    while { 1 } {
	set a [expr "[string first $pat $s] - 1"]
	set b [expr "$a + [string length $pat] + 1"]
	if { $a == -2 } break
	set s "[string range $s 0 $a]${rep}[string range $s $b end]"
    }
    return $s
}
 
proc loadAppDefaults { classNameList {priority startupFile} } {
    set filepath "[split [envVal XUSERFILESEARCHPATH] :] \
		  [envVal XAPPLRESDIR]/%N \
		  [split [envVal XFILESEARCHPATH] :] \
		  /usr/lib/X11/%T/%N%S"
    foreach i $classNameList {
        foreach j $filepath {
	    set f [doSubst [doSubst [doSubst $j  %T app-defaults] %S .ad] %N $i]
	    if {[file exists $f]} {
		catch "option readfile $f $priority"
		break
	    }
	}
    }
}

proc checkSourceColor {source} {
    global color$source
    case [set color$source] in {
	grey  {set wantcolor 0}
	color {set wantcolor 1}
    }
    .nvsource$source.video color $wantcolor
}

proc checkSourceSize {source} {
    global size$source
    case [set size$source] in {
	half    {set scaler  1}
	default {set scaler  0}
	double  {set scaler -1}
    }
    .nvsource$source.video scale $scaler
}

proc doSourceCapture {source} {
    global copy
    set name [wm title .nvsource$source]
    toplevel .nvcopy$copy
    wm title .nvcopy$copy "$name (captured)"
    wm maxsize .nvcopy$copy [winfo reqwidth .nvsource$source.video] 2000
    wm minsize .nvcopy$copy [winfo reqwidth .nvsource$source.video] \
	[winfo reqheight .nvsource$source.video]
    wm protocol .nvcopy$copy WM_TAKE_FOCUS "focus .nvcopy$copy.text"
    pack append .nvcopy$copy \
	[frame .nvcopy$copy.video] {top fill} \
	[scrollbar .nvcopy$copy.sb -relief raised \
	    -command ".nvcopy$copy.text yview"] {left filly} \
	[text .nvcopy$copy.text -width 1 -height 3 -relief raised \
	    -yscrollcommand ".nvcopy$copy.sb set"] {left expand fill}
    .nvsource$source.video copy .nvcopy$copy.video
    bind .nvcopy$copy.text <Control-c> "destroy .nvcopy$copy"
    bind .nvcopy$copy.text <Escape> "destroy .nvcopy$copy"
    set copy [expr $copy+1]
}

proc setSourceName {source name} {
    wm title .nvsource$source $name
    wm iconname .nvsource$source $name
    if [regexp "(.*) \[<(\].*\[>)\]" $name fullname shortname] {
	set name $shortname
    }
    .sources.canvas itemconfig title$source -text $name
    while 1 {
	set bbox [.sources.canvas bbox title$source]
	set w [expr [lindex $bbox 2]-[lindex $bbox 0]]
	if {($w > 100) && ([string length $name] > 1)} {
	    set name [string range $name 0 [expr [string length $name]-2]]
	    .sources.canvas itemconfig title$source -text ${name}...
	} else {
	    break
	}
    }
}

proc setRecvStats {source fps shown bps loss} {
    if {$fps == -1} {
	.nvsource$source.ctl.frames.fps config -text ""
	.nvsource$source.ctl.frames.shown config -text ""
	.nvsource$source.ctl.bytes.bps config -text ""
	.nvsource$source.ctl.bytes.loss config -text ""
    } else {
	.nvsource$source.ctl.frames.fps config -text "$fps  fps"
	.nvsource$source.ctl.frames.shown config -text "($shown shown)"
	.nvsource$source.ctl.bytes.bps config -text "$bps kbps"
	.nvsource$source.ctl.bytes.loss config -text "([set loss]% loss)"
    }
}

proc addGrabber {grabber name state} {
    .menu.grabbers.m add radio -label $name -state $state -variable nvGrabber \
	-value $grabber -command {global nvGrabber; changeGrabber $nvGrabber}
}

proc addEncoding {encoding name state} {
    .menu.encodings.m add radio -label $name -state $state \
	-variable nvEncoding -value $encoding \
	-command {global nvEncoding; changeEncoding $nvEncoding}
}

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
    bind .nvsource$source.video <1> "hideRecvControls $source"
    bind .nvsource$source.video <2> "hideRecvControls $source"
    bind .nvsource$source.video <3> "hideRecvControls $source"
}
 
proc hideRecvControls {source} {
    pack unpack .nvsource$source.ctl
    bind .nvsource$source.video <1> "showRecvControls $source"
    bind .nvsource$source.video <2> "showRecvControls $source"
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

    bind .nvsource$source.video <1> "showRecvControls $source"
    bind .nvsource$source.video <2> "showRecvControls $source"
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

proc checkPanels {} {
    global startup grabber_found nvConfInfo nvGrabPanel nvXmitPanel nvRecvPanel

    set w [winfo width .sources]
    set h [winfo height .sources]

    .sources config -width $w -height $h
    wm sizefrom . ""
    wm geometry . ""
    wm minsize . 1 1

    pack unpack .confInfo
    pack unpack .grabControls
    pack unpack .sendControls
    pack unpack .recvDefaults

    if {$nvConfInfo == "true"} {
	pack append . .confInfo {top fillx}
    }
    if {$grabber_found && ($nvGrabPanel == "true")} {
	pack append . .grabControls {top fillx}
    }
    if {$grabber_found && ($nvXmitPanel == "true")} {
	pack append . .sendControls {top fillx}
    }
    if {$nvRecvPanel == "true"} {
	pack append . .recvDefaults {top fillx}
    }

    update idletasks
    set minw [max 320 [winfo reqwidth .confInfo] \
		      [winfo reqwidth .grabControls] \
		      [winfo reqwidth .sendControls] \
		      [winfo reqwidth .recvDefaults]]
    set minh [expr [winfo height .]-[max 99 [winfo height .sources]]+99]
    wm minsize . $minw $minh
}

proc findSource {name} {
    set match ""
    foreach child [winfo children .sources] {
	if [string match ".sources.source*" $child] {
	    set sourcename [lindex [$child config -text] 4]
	    if [string match $name $sourcename] {
		set match $child
		break
	    }
	}
    }
 
    if [string length $match] {
	return [string range $match 15 end]
    } else {
	error "Source not found: $name"
    }
}

proc openSource {name} {
    set source [findSource $name]
    global source$source
    set source$source 1
    receiveVideo $source 1
    wm deiconify .nvsource$source
}

proc closeSource {name} {
    set source [findSource $name]
    global source$source
    set source$source 0
    receiveVideo $source 0
    wm withdraw .nvsource$source
}

proc setSourceBrightness {name brightness} {
    set source [findSource $name]
    .nvsource$source.ctl.row1.brightness set $brightness
    changeBrightness $source $brightness
}

proc setSourceContrast {name contrast} {
    set source [findSource $name]
    .nvsource$source.ctl.row1.contrast set $contrast
    changeContrast $source $contrast
}

proc setSourceSize {name size} {
    set source [findSource $name]
    set size$source $size
    checkSourceSize $source
}

proc setSourceColor {name color} {
    set source [findSource $name]
    set color$source $color
    checkSourceColor $source
}

proc captureSource {name} {
    set source [findSource $name]
    doSourceCapture $source
}

proc setAddress {addr} {
    .confControls.netinfo.e1 delete 0 end
    .confControls.netinfo.e1 insert 0 $addr
    changeAddress $addr
}

proc setPort {port} {
    .confControls.netinfo.e2 delete 0 end
    .confControls.netinfo.e2 insert 0 $port
    changePort $port
}

proc setTTL {ttl} {
    .confControls.netinfo.e3 delete 0 end
    .confControls.netinfo.e3 insert 0 $ttl
    changeTTL $ttl
}

proc setName {name} {
    .confControls.name.e1 delete 0 end
    .confControls.name.e1 insert 0 $name
    changeName $name
}

proc setMaxBandwidth {bw} {
    .sendControls.bandwidth set $bw
}

proc startSending {} {
    global grabber_found

    if {! $grabber_found} {
	error "No grabber available"
    }

    if {![catch "sendVideo 1" errmsg]} {
	.sendControls.enable config -text "Stop sending"
    } else {
	tk_dialog .errorbox "Uh oh!" $errmsg "" 0 OK
    }
}

proc stopSending {} {
    sendVideo 0
    .sendControls.enable config -text "Start sending"
}

proc pressSend {} {
    set state [lindex [ .sendControls.enable config -text] 4]
    if {$state == "Start sending"} {
	startSending
    } else {
	stopSending
    }
}

case $screenDepth in {
    1		{
		  set activeBgColor     black
		  set activeFgColor     white
		  set disabledFgColor   ""
		  set selectBgColor     black
		  set BgColor           white
		  set FgColor           black
		}
    default	{
		  set activeBgColor     #dfdfdf
		  set activeFgColor     black
		  set disabledFgColor   #777777
		  set selectBgColor     #bfdfff
		  set BgColor           #bfbfbf
		  set FgColor           black
		}
}

option add *activeBackground			$activeBgColor    startupFile
option add *activeForeground			$activeFgColor    startupFile
option add *background				$BgColor          startupFile
option add *foreground				$FgColor          startupFile
option add *selectBackground			$selectBgColor    startupFile
option add *Scale.activeForeground		$activeBgColor    startupFile
option add *Scale.sliderForeground		$BgColor          startupFile
option add *Scrollbar.foreground		$BgColor          startupFile
option add *Scrollbar.activeForeground		$activeBgColor    startupFile
option add *Button.disabledForeground		$disabledFgColor  startupFile
option add *Button.disabledForeground		$disabledFgColor  startupFile
option add *Radiobutton.disabledForeground	$disabledFgColor  startupFile
option add *Menu.disabledForeground		$disabledFgColor  startupFile
option add *highlightThickness			0		  startupFile
option add *Button.padY				1		  startupFile
option add *Menu.padY				2		  startupFile

set nvBrightness	60
set nvConfInfo		true
set nvContrast		50
set nvEncoding		""
set nvGrabber		""
set nvGrabPanel		false
set nvMaxBandwidth	128
set nvMaxBWLimit	1024
set nvMaxFrameRate	0
set nvPort		4444
set nvRecvColor		color
set nvRecvPanel		false
set nvRecvSize		normal
set nvXmitColor		color
set nvXmitPanel		true
set nvXmitSize		medium
set nvTTL		16

option add Nv.brightness			$nvBrightness	  startupFile
option add Nv.confInfo				$nvConfInfo	  startupFile
option add Nv.contrast				$nvContrast	  startupFile
option add Nv.encoding				$nvEncoding	  startupFile
option add Nv.grabber				$nvGrabber	  startupFile
option add Nv.grabPanel				$nvGrabPanel	  startupFile
option add Nv.maxBandwidth			$nvMaxBandwidth	  startupFile
option add Nv.maxBandwidthLimit			$nvMaxBWLimit	  startupFile
option add Nv.maxFrameRate			$nvMaxFrameRate	  startupFile
option add Nv.port				$nvPort		  startupFile
option add Nv.recvColor				$nvRecvColor	  startupFile
option add Nv.recvPanel				$nvRecvPanel	  startupFile
option add Nv.recvSize				$nvRecvSize	  startupFile
option add Nv.xmitColor				$nvXmitColor	  startupFile
option add Nv.xmitPanel				$nvXmitPanel	  startupFile
option add Nv.xmitSize				$nvXmitSize	  startupFile
option add Nv.title				"nv v$version"	  startupFile
option add Nv.ttl				$nvTTL		  startupFile

loadAppDefaults {Nv NV} userDefault
if {[envVal XENVIRONMENT] != {}} {
    catch "option readfile [envVal XENVIRONMENT] userDefault"
}

frame .menu -borderwidth 2 -relief raised
menubutton .menu.info -text "Info...  " -menu .menu.info.m
menubutton .menu.grabbers -text "Grabbers...  " -menu .menu.grabbers.m
menubutton .menu.encodings -text "Encodings...  " -menu .menu.encodings.m
menubutton .menu.panels -text "Panels...  " -menu .menu.panels.m

menu .menu.info.m
.menu.info.m add command -label "About nv..." \
  -command {tk_dialog .aboutnv "About nv" "nv Network Video Tool, version $version\nby Ron Frederick <frederick@parc.xerox.com>" "" 0 OK}

menu .menu.grabbers.m
menu .menu.encodings.m

menu .menu.panels.m
.menu.panels.m add check -label "Show conference info" -variable nvConfInfo \
    -onvalue "true" -offvalue "false" -command {checkPanels}
if {$grabber_found} {
    .menu.panels.m add check -label "Show grabber controls" -variable \
	nvGrabPanel -onvalue "true" -offvalue "false" -command {checkPanels}
    .menu.panels.m add check -label "Show transmit options" -variable \
	nvXmitPanel -onvalue "true" -offvalue "false" -command {checkPanels}
}
.menu.panels.m add check -label "Show receive defaults" -variable nvRecvPanel \
    -onvalue "true" -offvalue "false" -command {checkPanels}

pack append .menu .menu.info {left}
pack append .menu .menu.grabbers {left}
pack append .menu .menu.encodings {left}
pack append .menu .menu.panels {left}

frame .sources
pack append .sources \
    [scrollbar .sources.sb -relief raised -command ".sources.canvas yview"] \
	{left filly} \
    [canvas .sources.canvas -height 95 -width 300 -borderwidth 2 \
	-relief raised -yscrollincrement 95 -scrollregion { 0 0 100 95 } \
	-yscrollcommand ".sources.sb set"] {left expand fill}

frame .confInfo -borderwidth 2 -relief raised

label .confInfo.title -text "Conference info"

frame .confInfo.netinfo -relief flat
pack append .confInfo.netinfo \
    [label .confInfo.netinfo.l1 -text "Address:"] {left filly} \
    [entry .confInfo.netinfo.e1 -relief flat -width 16] {left expand fill} \
    [label .confInfo.netinfo.l2 -text "Port:"] {left filly} \
    [entry .confInfo.netinfo.e2 -relief flat -width 6] {left filly}

frame .confInfo.namettl -relief flat
pack append .confInfo.namettl \
    [label .confInfo.namettl.l1 -text "Name:"] {left filly} \
    [entry .confInfo.namettl.e1 -relief flat -width 1] {left expand fill} \
    [label .confInfo.namettl.l2 -text "TTL:"] {left filly} \
    [entry .confInfo.namettl.e2 -relief flat -width 5] {left filly}

pack append .confInfo \
    .confInfo.title {top fillx} \
    .confInfo.netinfo {top fill} \
    .confInfo.namettl {top fill}

frame .grabControls -borderwidth 2 -relief raised

frame .sendControls -borderwidth 2 -relief raised

label  .sendControls.title -text "Video transmit options"

set bwLimit [option get . maxBandwidthLimit Nv]
if {[catch {expr $bwLimit+0}] || ($bwLimit < 16)} {
    set bwLimit $nvMaxBWLimit
}

scale  .sendControls.bandwidth -label "Max Bandwidth (kbps)" \
    -from 1 -to $bwLimit -bigincrement [expr $bwLimit/16] -orient horizontal \
    -command "changeMaxBandwidth"

frame .sendControls.radios

frame .sendControls.radios.size
pack append .sendControls.radios.size \
    [radiobutton .sendControls.radios.size.small -text "Small" \
	-relief flat -width 12 -variable nvXmitSize -value small -anchor w] \
	{top fill} \
    [radiobutton .sendControls.radios.size.medium -text "Medium" \
	-relief flat -width 12 -variable nvXmitSize -value medium -anchor w] \
	{top fill} \
    [radiobutton .sendControls.radios.size.large -text "Large" \
	-relief flat -width 12 -variable nvXmitSize -value large -anchor w] \
	{top fill}

frame .sendControls.radios.color
pack append .sendControls.radios.color \
    [radiobutton .sendControls.radios.color.grey  -text "Greyscale" \
	-relief flat -width 12 -variable nvXmitColor -value grey -anchor w] \
	{top fill} \
    [radiobutton .sendControls.radios.color.color -text "Color" \
	-relief flat -width 12 -variable nvXmitColor -value color -anchor w] \
	{top fill}

pack append .sendControls.radios \
    [frame .sendControls.radios.fill1] {left expand fill} \
    .sendControls.radios.size  {left fill} \
    [frame .sendControls.radios.fill2] {left expand fill} \
    .sendControls.radios.color {left fill} \
    [frame .sendControls.radios.fill3] {left expand fill}

button .sendControls.enable -text "Start sending" -command "pressSend"

pack append .sendControls \
    .sendControls.title {top fillx} \
    .sendControls.bandwidth {top fillx} \
    .sendControls.radios {top fillx} \
    .sendControls.enable {top fillx}

bind .sources.canvas <Configure> "packSources"

bind .confInfo.netinfo.e1 <Return> {
    focus .
    .confInfo.netinfo.e1 select clear
}

bind .confInfo.netinfo.e2 <Return> {
    focus .
    .confInfo.netinfo.e2 select clear
}

bind .confInfo.namettl.e1 <Return> {
    focus .
    .confInfo.namettl.e1 select clear
}

bind .confInfo.namettl.e2 <Return> {
    focus .
    .confInfo.namettl.e2 select clear
}

bind .confInfo.netinfo.e1 <FocusOut> {changeAddress [.confInfo.netinfo.e1 get]}
bind .confInfo.netinfo.e2 <FocusOut> {changePort [.confInfo.netinfo.e2 get]}
bind .confInfo.namettl.e1 <FocusOut> {changeName [.confInfo.namettl.e1 get]}
bind .confInfo.namettl.e2 <FocusOut> {changeTTL [.confInfo.namettl.e2 get]}

if [catch {.sendControls.bandwidth set [option get . maxBandwidth Nv]}] {
    .sendControls.bandwidth set $nvMaxBandwidth
}
changeMaxBandwidth [.sendControls.bandwidth get]

.confInfo.netinfo.e1 insert 0 [option get . address Nv]
changeAddress [.confInfo.netinfo.e1 get]
.confInfo.netinfo.e2 insert 0 [option get . port Nv]
changePort [.confInfo.netinfo.e2 get]
.confInfo.namettl.e1 insert 0 [option get . name Nv]
changeName [.confInfo.namettl.e1 get]
.confInfo.namettl.e2 insert 0 [option get . ttl Nv]
changeTTL [.confInfo.namettl.e2 get]
sendVideo 0

frame .recvDefaults -borderwidth 2 -relief raised

frame .recvDefaults.row1 -borderwidth 0 -relief flat

scale .recvDefaults.row1.brightness -label "Brightness" \
    -from 0 -to 100 -orient horizontal -command "set nvBrightness"

scale .recvDefaults.row1.contrast -label "Contrast" \
    -from 0 -to 100 -orient horizontal -command "set nvContrast"

pack append .recvDefaults.row1 \
    .recvDefaults.row1.brightness {left expand fill} \
    [frame .recvDefaults.row1.fill -width 10 -height 1] {left fill} \
    .recvDefaults.row1.contrast {left expand fill}

frame .recvDefaults.row2 -borderwidth 0 -relief flat

frame .recvDefaults.row2.size
pack append .recvDefaults.row2.size \
    [radiobutton .recvDefaults.row2.size.half   -text "Half Size" \
	-relief flat -width 12 -variable nvRecvSize -value half -anchor w] \
	{top fill} \
    [radiobutton .recvDefaults.row2.size.normal -text "Normal Size" \
	-relief flat -width 12 -variable nvRecvSize -value normal -anchor w] \
	{top fill} \
    [radiobutton .recvDefaults.row2.size.double -text "Double Size" \
	-relief flat -width 12 -variable nvRecvSize -value double -anchor w] \
	{top fill}

frame .recvDefaults.row2.color
pack append .recvDefaults.row2.color \
    [radiobutton .recvDefaults.row2.color.grey  -text "Greyscale" \
	-relief flat -width 12 -variable nvRecvColor -value grey -anchor w] \
	{top fill} \
    [radiobutton .recvDefaults.row2.color.color -text "Color" \
	-relief flat -width 12 -variable nvRecvColor -value color -anchor w] \
	{top fill}

pack append .recvDefaults.row2 \
    [frame .recvDefaults.fill1] {left expand fill} \
    .recvDefaults.row2.size  {left fill} \
    [frame .recvDefaults.fill2] {left expand fill} \
    .recvDefaults.row2.color {left fill} \
    [frame .recvDefaults.fill3] {left expand fill}

pack append .recvDefaults \
    [label .recvDefaults.title -text "Video receive defaults"] {top fill} \
    .recvDefaults.row1 {top fill} \
    .recvDefaults.row2 {top fill}

wm protocol . WM_TAKE_FOCUS {focus .}

bind . <Control-c> {exit}
bind . <Escape> {exit}
bind . <q> {exit}

pack append . .menu {top fill} .sources {top expand fill}

wm title . [option get . title Nv]

set nvEncoding [option get . encoding Nv]
set nvGrabber [option get . grabber Nv]

if [catch ".recvDefaults.row1.brightness set [option get . brightness Nv]"] {
    .recvDefaults.row1.brightness set $nvBrightness
}
set nvBrightness [.recvDefaults.row1.brightness get]

if [catch ".recvDefaults.row1.contrast set [option get . contrast Nv]"] {
    .recvDefaults.row1.contrast set $nvContrast
}
set nvContrast [.recvDefaults.row1.contrast get]

set nvConfInfo [option get . confInfo Nv]
set nvRecvColor [option get . recvColor Nv]
set nvRecvSize [option get . recvSize Nv]
set nvXmitColor [option get . xmitColor Nv]
set nvXmitPanel [option get . xmitPanel Nv]
set nvXmitSize [option get . xmitSize Nv]
checkPanels
