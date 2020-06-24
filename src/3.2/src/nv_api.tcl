#
# nv_api.tcl - TK procedures which define the remote NV API
#

set recvgeom ""

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

proc openSource { name } {
    set source [findSource $name]
    global source$source
    set source$source 1
    receiveVideo $source 1
    wm deiconify .nvsource$source
}

proc closeSource { name } {
    set source [findSource $name]
    global source$source
    set source$source 0
    receiveVideo $source 0
    wm withdraw .nvsource$source
}

proc setSourceBrightness { name brightness } {
    set source [findSource $name]
    .nvsource$source.ctl.brightness set $brightness
}

proc setSourceContrast { name contrast } {
    set source [findSource $name]
    .nvsource$source.ctl.contrast set $contrast
}

proc setSourceSize { name size } {
    set source [findSource $name]
    changeSourceSize $source $size
}

proc captureSource { name } {
    set source [findSource $name]
    doSourceCapture $source
}

proc setAddress { addr } {
    .confControls.netinfo.e1 delete 0 end
    .confControls.netinfo.e1 insert 0 $addr
    changeAddress $addr
}

proc setPort { port } {
    .confControls.netinfo.e2 delete 0 end
    .confControls.netinfo.e2 insert 0 $port
    changePort $port
}

proc setTTL { ttl } {
    .confControls.netinfo.e3 delete 0 end
    .confControls.netinfo.e3 insert 0 $ttl
    changeTTL $ttl
}

proc setName { name } {
    .confControls.name.e1 delete 0 end
    .confControls.name.e1 insert 0 $name
    changeName $name
}

proc setMaxBandwidth { bw } {
    .sendControls.bandwidth set $bw
}

proc startSending { } {
    sendVideo 1
    .sendControls.buttons.enable config -text "Stop sending"
}

proc stopSending { } {
    sendVideo 0
    .sendControls.buttons.enable config -text "Start sending"
}

proc showReceivers { } {
    global recvgeom
    wm geometry .receivers $recvgeom
    wm deiconify .receivers
    .sendControls.buttons.receivers config -text "Hide receivers"
}

proc hideReceivers { } {
    global recvgeom
    wm withdraw .receivers
    regexp "\[+-\].*" [wm geometry .receivers] recvgeom
    .sendControls.buttons.receivers config -text "Show receivers"
}
