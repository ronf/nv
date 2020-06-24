#
# nv_api.tcl - TK procedures which define the remote NV API
#

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
