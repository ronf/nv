#
# nv_init.tcl - TK interface definition for NV
#

set version "3.2"
set copy 0

set screenDepth [winfo screendepth .]
case $screenDepth in {
    1		{
		  set activeBgColor	black
		  set activeFgColor	white
		  set disabledFgColor	""
		  set selectBgColor	black
		  set BgColor		white
		  set FgColor		black
		}
    default	{
		  set activeBgColor	#dfdfdf
		  set activeFgColor	black
		  set disabledFgColor	#777777
		  set selectBgColor	#bfdfff
		  set BgColor		#bfbfbf
		  set FgColor		black
		}
}

set nvBrightness			50
set nvContrast				50
set nvMaxBandwidth			128
set nvRecvColor				color
set nvRecvSize				normal
set nvXmitColor				color
set nvTTL				16

option add Nv.brightness		$nvBrightness		startupFile
option add Nv.contrast			$nvContrast		startupFile
option add Nv.maxBandwidth		$nvMaxBandwidth		startupFile
option add Nv.recvColor			$nvRecvColor		startupFile
option add Nv.recvSize			$nvRecvSize		startupFile
option add Nv.xmitColor			$nvXmitColor		startupFile
option add Nv.title			"nv v$version"		startupFile
option add Nv.ttl			$nvTTL			startupFile

option add *activeBackground		$activeBgColor		startupFile
option add *activeForeground		$activeFgColor		startupFile
option add *CheckButton.selector	$FgColor		startupFile
option add *background			$BgColor		startupFile
option add *foreground			$FgColor		startupFile
option add *selectBackground		$selectBgColor		startupFile
option add *Scale.activeForeground	$activeBgColor		startupFile
option add *Scale.sliderForeground	$BgColor		startupFile
option add *Scrollbar.foreground	$BgColor		startupFile
option add *Scrollbar.activeForeground	$activeBgColor		startupFile
option add *Button.disabledForeground	$disabledFgColor	startupFile

proc pressSend {} {
    set state [lindex [ .sendControls.buttons.enable config -text] 4]
    if {$state == "Start sending"} {
	startSending
    } else {
	stopSending
    }
}
 
proc pressReceivers {} {
    set state [lindex [ .sendControls.buttons.receivers config -text] 4]
    if {$state == "Show receivers"} {
	showReceivers
    } else {
	hideReceivers
    }
}

loadAppDefaults {Nv NV} userDefault
if {[envVal XENVIRONMENT] != {}} {
    option readfile [envVal XENVIRONMENT] userDefault
}

frame .sources
pack append .sources \
    [scrollbar .sources.sb -relief raised -command ".sources.canvas yview"] \
	{left filly} \
    [canvas .sources.canvas -height 95 -width 300 -relief raised \
	-scrollincrement 95 -scrollregion { 0 0 100 95 } \
	-yscroll ".sources.sb set"] {left expand fill}

frame .confControls -borderwidth 2 -relief raised

label .confControls.title -text "Conference info"

frame .confControls.netinfo -relief flat
pack append .confControls.netinfo \
    [label .confControls.netinfo.l1 -text "Address:"] {left filly} \
    [entry .confControls.netinfo.e1 -relief flat -width 14] {left expand fill} \
    [label .confControls.netinfo.l2 -text "Port:"] {left filly} \
    [entry .confControls.netinfo.e2 -relief flat -width 4] {left filly} \
    [label .confControls.netinfo.l3 -text "TTL:"] {left filly} \
    [entry .confControls.netinfo.e3 -relief flat -width 1] {left filly}

frame .confControls.name -relief flat
pack append .confControls.name \
    [label .confControls.name.l1 -text "Name:"] {left filly} \
    [entry .confControls.name.e1 -relief flat -width 1] {left expand fill}

pack append .confControls \
    .confControls.title {top fillx} \
    .confControls.netinfo {top fill} \
    .confControls.name {top fill}

frame .sendControls -borderwidth 2 -relief raised

label  .sendControls.title -text "Video transmit options"

scale  .sendControls.bandwidth -command "changeMaxBandwidth" \
    -label "Max Bandwidth (kbps)" -from 1 -to 1024 -orient horizontal

frame .sendControls.color
pack append .sendControls.color \
    [button .sendControls.color.grey -text "Send greyscale" -width 20 \
	-command "sendColor 0"] {left expand fill} \
    [button .sendControls.color.color -text "Send color" -width 20 \
	-state disabled -command "sendColor 1"] {left expand fill}

frame .sendControls.buttons
pack append .sendControls.buttons \
    [button .sendControls.buttons.enable -text "Start sending" -width 20 \
	-command "pressSend"] {left expand fill} \
    [button .sendControls.buttons.receivers -text "Show receivers" -width 20 \
	-command "pressReceivers"] {left expand fill}

bind .sources.canvas <Configure> "packSources"

bind .confControls.netinfo.e1 <Return> {
    focus .
    changeAddress [.confControls.netinfo.e1 get]
    .confControls.netinfo.e1 select clear
}

bind .confControls.netinfo.e2 <Return> {
    focus .
    changePort [.confControls.netinfo.e2 get]
    .confControls.netinfo.e2 select clear
}

bind .confControls.netinfo.e3 <Return> {
    focus .
    changeTTL [.confControls.netinfo.e3 get]
    .confControls.netinfo.e3 select clear
}

bind .confControls.name.e1 <Return> {
    focus .
    changeName [.confControls.name.e1 get]
    .confControls.name.e1 select clear
}

if [catch {.sendControls.bandwidth set [option get . maxBandwidth Nv]}] {
    .sendControls.maxBandwidth set $nvMaxBandwidth
}

.confControls.netinfo.e1 insert 0 [option get . address Nv]
changeAddress [.confControls.netinfo.e1 get]
.confControls.netinfo.e2 insert 0 [option get . port Nv]
changePort [.confControls.netinfo.e2 get]
.confControls.netinfo.e3 insert 0 [option get . ttl Nv]
changeTTL [.confControls.netinfo.e3 get]
.confControls.name.e1 insert 0 [option get . name Nv]
changeName [.confControls.name.e1 get]
sendVideo 0

pack append .sendControls \
    .sendControls.title {top expand fillx} \
    .sendControls.bandwidth {top expand fillx} \
    .sendControls.color {top fillx} \
    .sendControls.buttons {top fillx}

toplevel .receivers
pack append .receivers \
    [scrollbar .receivers.scroll -relief raised \
	-command ".receivers.list yview"] { left filly } \
    [listbox .receivers.list -geometry 60x10 -yscroll ".receivers.scroll set" \
	-relief raised] { left expand fill }
wm minsize .receivers 0 0
wm title .receivers "nv receivers"
wm withdraw .receivers

pack append . .sources {top expand fill} .confControls {top fill}

global send_allowed
if $send_allowed { pack append . .sendControls { top fillx } }

update
set reqw [winfo reqwidth .]
set reqh [winfo reqheight .]

wm minsize . $reqw $reqh
wm geometry . ${reqw}x$reqh
wm title . [option get . title Nv]

wm protocol . WM_TAKE_FOCUS {focus .}
wm protocol .receivers WM_TAKE_FOCUS {focus .receivers}

bind . <Control-c> {shutdown}
bind . <Escape> {shutdown}
bind . <q> {shutdown}

bind .receivers <Control-c> {pressReceivers}
bind .receivers <Escape> {pressReceivers}
bind .receivers <q> {pressReceivers}
wm protocol .receivers WM_DELETE_WINDOW {pressReceivers}
