#
# nv_subr.tcl - TK utility subroutines used by NV
#

set source_list ""
set copy 0

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

proc findSource { name } {
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

proc changeSourceColor {source colormode} {
    case $colormode in {
	grey  {set wantcolor 0}
	color {set wantcolor 1}
    }
    .nvsource$source.ctl.colorbuttons.grey config -state normal
    .nvsource$source.ctl.colorbuttons.color config -state normal
    .nvsource$source.ctl.colorbuttons.$colormode config -state disabled
    .nvsource$source.video color $wantcolor
}

proc changeSourceSize {source size} {
    case $size in {
	half    {set scaler  1}
	default {set scaler  0}
	double  {set scaler -1}
    }
    .nvsource$source.ctl.sizebuttons.half config -state normal
    .nvsource$source.ctl.sizebuttons.normal config -state normal
    .nvsource$source.ctl.sizebuttons.double config -state normal
    .nvsource$source.ctl.sizebuttons.$size config -state disabled
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

proc setFPSandBPS {source fps bps} {
    if {$fps == ""} {
	.nvsource$source.ctl.frames.fps config -text ""
	.nvsource$source.ctl.bytes.bps config -text ""
    } else {
	.nvsource$source.ctl.frames.fps config -text "$fps  fps"
	.nvsource$source.ctl.bytes.bps config -text "$bps kbps"
    }
}
