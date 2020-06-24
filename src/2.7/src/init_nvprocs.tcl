#
# nvprocs.tk - TK procedures used by the netvideo application
#

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

proc recursiveBind { w event action } {
    bind $w $event $action
    foreach child [winfo children $w] { recursiveBind $child $event $action }
}

proc pressSend { } {
    set state [lindex [ .sendControls.enable config -text] 4]
    if { $state == "Start sending" } {
	sendVideo 1
	.sendControls.enable config -text "Stop sending"
    } else {
	sendVideo 0
	.sendControls.enable config -text "Start sending"
    }
}

proc pressReceivers { } {
    set state [lindex [ .sendControls.receivers config -text] 4]
    if { $state == "Show receivers" } {
	wm deiconify .receivers
	.sendControls.receivers config -text "Hide receivers"
    } else {
	wm withdraw .receivers
	.sendControls.receivers config -text "Show receivers"
    }
}

proc showRecvPanel { source } {
    pack append .nvsource$source .nvsource$source.ctl { top expand fill }
    bind .nvsource$source <1> "hideRecvPanel $source"
    bind .nvsource$source.video <1> "hideRecvPanel $source"
    bind .nvsource$source <2> "hideRecvPanel $source"
    bind .nvsource$source.video <2> "hideRecvPanel $source"
    bind .nvsource$source <3> "hideRecvPanel $source"
    bind .nvsource$source.video <3> "hideRecvPanel $source"
}

proc hideRecvPanel { source } {
    pack unpack .nvsource$source.ctl
    bind .nvsource$source <1> "showRecvPanel $source"
    bind .nvsource$source.video <1> "showRecvPanel $source"
    bind .nvsource$source <2> "showRecvPanel $source"
    bind .nvsource$source.video <2> "showRecvPanel $source"
    bind .nvsource$source <3> "showRecvPanel $source"
    bind .nvsource$source.video <3> "showRecvPanel $source"
}

proc checkSource { source } {
    global source$source
    set enabled [set source$source]
    set name [lindex [ .sources.source$source config -text] 4]
    receiveVideo $source $enabled
    if $enabled {
	wm deiconify .nvsource$source
    } else {
	wm withdraw .nvsource$source
    }
}

proc setRecvSize { source size } {
    case $size in {
	half	{ set scaler 2 }
	default	{ set scaler 1 }
	double	{ set scaler 0 }
    }
    .nvsource$source.ctl.sizebuttons.half config -state normal
    .nvsource$source.ctl.sizebuttons.normal config -state normal
    .nvsource$source.ctl.sizebuttons.double config -state normal
    .nvsource$source.ctl.sizebuttons.$size config -state disabled
    .nvsource$source.video scale $scaler
}

proc copySource { source } {
    global copy
    set name [lindex [ .sources.source$source config -text] 4]
    toplevel .nvcopy$copy
    wm title .nvcopy$copy "$name (captured)"
    .nvsource$source.video copy .nvcopy$copy
    bind .nvcopy$copy <1> "destroy .nvcopy$copy"
    bind .nvcopy$copy <2> "destroy .nvcopy$copy"
    bind .nvcopy$copy <3> "destroy .nvcopy$copy"
    set copy [expr "$copy + 1"]
}

proc addSource { source name } {
    pack before .sources.fill \
	[ checkbutton .sources.source$source -anchor w \
	    -command "checkSource $source" -padx 5 -relief flat \
	    -text $name ] { top fillx }

    frame .nvsource$source.ctl -borderwidth 2 -relief raised

    frame .nvsource$source.ctl.sizebuttons
    pack append .nvsource$source.ctl.sizebuttons \
	[ button .nvsource$source.ctl.sizebuttons.half -text "Half" \
	    -command "setRecvSize $source half" ] { left expand fill } \
	[ button .nvsource$source.ctl.sizebuttons.normal -text "Normal" \
	    -command "setRecvSize $source normal" ] { left expand fill } \
	[ button .nvsource$source.ctl.sizebuttons.double -text "Double" \
	    -command "setRecvSize $source double" ] { left expand fill }

    scale .nvsource$source.ctl.brightness -command "setBrightness $source" \
	-label "Brightness" -from 0 -to 100 -orient horizontal

    scale .nvsource$source.ctl.contrast -command "setContrast $source" \
	-label "Contrast" -from 0 -to 100 -orient horizontal

    frame .nvsource$source.ctl.frames -relief flat
    pack append .nvsource$source.ctl.frames \
	[ label .nvsource$source.ctl.frames.label -anchor w -width 10 \
	    -text "Frame rate:" ] { left filly } \
	[ label .nvsource$source.ctl.frames.fps -anchor e -width 9 ] \
	    { left filly } \
	[ frame .nvsource$source.ctl.frames.fill ] { left expand fill }

    frame .nvsource$source.ctl.bytes -relief flat
    pack append .nvsource$source.ctl.bytes \
	[ label .nvsource$source.ctl.bytes.label -anchor w -width 10 \
	    -text "Bandwidth:" ] { left filly } \
	[ label .nvsource$source.ctl.bytes.bps -anchor e -width 9 ] \
	    { left filly } \
	[ frame .nvsource$source.ctl.bytes.fill ] { left expand fill }

    button .nvsource$source.ctl.capture -text "Capture" \
	-command "copySource $source"

    wm title .nvsource$source $name
    wm withdraw .nvsource$source

    pack append .nvsource$source.ctl \
	.nvsource$source.ctl.brightness { top expand fillx pady 4 frame s } \
	.nvsource$source.ctl.contrast { top expand fill } \
	.nvsource$source.ctl.frames { top expand fill } \
	.nvsource$source.ctl.bytes { top expand fill } \
	.nvsource$source.ctl.sizebuttons { top expand fill } \
	.nvsource$source.ctl.capture { top expand fill }

    pack append .nvsource$source .nvsource$source.video { top expand fill }

    bind .sources.source$source <Control-c> {shutdown}
    bind .sources.source$source <q> {shutdown}
    recursiveBind .nvsource$source <Control-c> \
	"set source$source 0; checkSource $source"
    recursiveBind .nvsource$source <q> \
	"set source$source 0; checkSource $source"
    bind .nvsource$source <1> "showRecvPanel $source"
    bind .nvsource$source.video <1> "showRecvPanel $source"
    bind .nvsource$source <2> "showRecvPanel $source"
    bind .nvsource$source.video <2> "showRecvPanel $source"
    bind .nvsource$source <3> "showRecvPanel $source"
    bind .nvsource$source.video <3> "showRecvPanel $source"

    if [catch ".nvsource[set source].ctl.brightness set \
		   [option get . brightness Nv]"] {
	.nvsource$source.ctl.brightness set $nvBrightness
    }

    if [catch ".nvsource[set source].ctl.contrast set \
		   [option get . contrast Nv]"] {
	.nvsource$source.ctl.contrast set $nvContrast
    }

    setRecvSize $source [option get . recvSize Nv]
}

proc deleteSource { source } {
    global source$source
    set enabled [set source$source]
    if $enabled {
	receiveVideo $source 0
    }
    destroy .nvsource$source
    destroy .sources.source$source
}

proc setSourceName { source name } {
    .sources.source$source config -text $name
    wm title .nvsource$source $name
}

proc setFPSandBPS { source fps bps } {
    if { $fps == "" } {
	.nvsource$source.ctl.frames.fps config -text ""
	.nvsource$source.ctl.bytes.bps config -text ""
    } else {
	.nvsource$source.ctl.frames.fps config -text "$fps  fps"
	.nvsource$source.ctl.bytes.bps config -text "$bps kbps"
    }
}
