#
# nv.tk - A TK interface to the netvideo application
#

set version "2.7"
set copy 0

set screenDepth [winfo screendepth .]
case $screenDepth in {
    1		{
		  set activeBgColor	black
		  set activeFgColor	white
		  set disabledFgColor	""
		  set BgColor		white
		  set FgColor		black
		}
    default	{
		  set activeBgColor	#dddddd
		  set activeFgColor	black
		  set disabledFgColor	#777777
		  set BgColor		#bbbbbb
		  set FgColor		black
		}
}

set nvBrightness			60
set nvContrast				60
set nvMaxBandwidth			128
set nvRecvSize				normal
set nvTTL				16

option add Nv.brightness		$nvBrightness		startupFile
option add Nv.contrast			$nvContrast		startupFile
option add Nv.maxBandwidth		$nvMaxBandwidth		startupFile
option add Nv.recvSize			$nvRecvSize		startupFile
option add Nv.ttl			$nvTTL			startupFile

option add *activeBackground		$activeBgColor		startupFile
option add *activeForeground		$activeFgColor		startupFile
option add *CheckButton.selector	$FgColor		startupFile
option add *background			$BgColor		startupFile
option add *foreground			$FgColor		startupFile
option add *Scale.activeForeground	$activeBgColor		startupFile
option add *Scale.sliderForeground	$BgColor		startupFile
option add *Button.disabledForeground	$disabledFgColor	startupFile

loadAppDefaults {Nv NV} userDefault
if { [envVal XENVIRONMENT] != {} } {
    option readfile [envVal XENVIRONMENT] userDefault
}

frame .sources -borderwidth 2 -relief raised
pack append .sources \
	[ label .sources.title -text "Network video sources" ] { top fillx } \
	[ frame .sources.fill ] { top expand fill }

frame .confControls -borderwidth 2 -relief raised

label .confControls.title -text "Conference info"

frame .confControls.netinfo -relief flat
pack append .confControls.netinfo \
	[ label .confControls.netinfo.l1 -text "Address:" ] { left filly } \
	[ entry .confControls.netinfo.e1 -relief flat -width 14 ] \
		{ left expand fill } \
	[ label .confControls.netinfo.l2 -text "Port:" ] { left filly } \
	[ entry .confControls.netinfo.e2 -relief flat -width 4 ] \
		{ left filly } \
	[ label .confControls.netinfo.l3 -text "TTL:" ] { left filly } \
	[ entry .confControls.netinfo.e3 -relief flat -width 1 ] \
		{ left filly }

frame .confControls.name -relief flat
pack append .confControls.name \
	[ label .confControls.name.l1 -text "Name:" ] { left filly } \
	[ entry .confControls.name.e1 -relief flat -width 1 ] \
		{ left expand fill }

pack append .confControls \
	.confControls.title { top fillx } \
	.confControls.netinfo { top fill } \
	.confControls.name { top fill }

frame .sendControls -borderwidth 2 -relief raised

label  .sendControls.title -text "Video transmit options"

scale  .sendControls.bandwidth -command "setBandwidth" \
	-label "Max Bandwidth (kbps)" -from 0 -to 512 -orient horizontal

button .sendControls.enable -text "Start sending" -command "pressSend"
button .sendControls.receivers -text "Show receivers" -command "pressReceivers"

bind .confControls.netinfo.e1 <Return> {
    focus .
    setAddress [.confControls.netinfo.e1 get]
    .confControls.netinfo.e1 select clear
}

bind .confControls.netinfo.e2 <Return> {
    focus .
    setPort [.confControls.netinfo.e2 get]
    .confControls.netinfo.e2 select clear
}

bind .confControls.netinfo.e3 <Return> {
    focus .
    setTTL [.confControls.netinfo.e3 get]
    .confControls.netinfo.e3 select clear
}

bind .confControls.name.e1 <Return> {
    focus .
    setName [.confControls.name.e1 get]
    .confControls.name.e1 select clear
}

if [catch {.sendControls.bandwidth set [option get . maxBandwidth Nv]}] {
    .sendControls.maxBandwidth set $nvMaxBandwidth
}

.confControls.netinfo.e1 insert 0 [option get . address Nv]
setAddress [.confControls.netinfo.e1 get]
.confControls.netinfo.e2 insert 0 [option get . port Nv]
setPort [.confControls.netinfo.e2 get]
.confControls.netinfo.e3 insert 0 [option get . ttl Nv]
setTTL [.confControls.netinfo.e3 get]
.confControls.name.e1 insert 0 [option get . name Nv]
setName [.confControls.name.e1 get]
sendVideo 0

pack append .sendControls \
	.sendControls.title { top expand fillx pady 2 } \
	.sendControls.bandwidth { top expand fillx pady 2 } \
	.sendControls.enable { top expand fillx pady 2 } \
	.sendControls.receivers { top expand fillx pady 2 }

toplevel .receivers
pack append .receivers \
    [scrollbar .receivers.scroll -relief flat \
	-command ".receivers.list yview"] { left filly } \
    [listbox .receivers.list -geometry 60x10 -yscroll ".receivers.scroll set" \
	-relief flat] { left expand fill }
wm minsize .receivers 0 0
wm title .receivers "nv receivers"
wm withdraw .receivers

pack append . \
	.sources { top expand fill } \
	.confControls { top fill }


global send_allowed
if $send_allowed { pack append . .sendControls { top fillx } }

update
set reqw [winfo reqwidth .]
set reqh [winfo reqheight .]
set lineh [ winfo reqheight .sources.title ]

wm minsize . $reqw [expr "$reqh + $lineh"]
wm geometry . ${reqw}x[expr "$reqh + $lineh * 5"]
wm title . "nv v$version"

recursiveBind . <Control-c> {shutdown}
recursiveBind . <q> {shutdown}
recursiveBind .confControls.netinfo.e1 <q> {}
recursiveBind .confControls.netinfo.e2 <q> {}
recursiveBind .confControls.netinfo.e3 <q> {}
recursiveBind .confControls.name.e1 <q> {}
recursiveBind .receivers <Control-c> {}
recursiveBind .receivers <q> {}
recursiveBind .receivers <Control-c> {pressReceivers}
recursiveBind .receivers <q> {pressReceivers}
