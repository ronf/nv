#
# nv_init.tcl - TK interface definition for NV
#

set version "3.3beta"
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

set nvPort				4444
set nvChan				32

set nvBrightness			60
set nvConfInfo				true
set nvContrast				50
set nvEncoding				""
set nvGrabber				""
set nvGrabPanel				false
set nvMaxBandwidth			128
set nvMaxBWLimit			1024
set nvMaxFrameRate			30
set nvRecvColor				color
set nvRecvPanel				false
set nvRecvSize				normal
set nvXmitColor				color
set nvXmitPanel				true
set nvXmitSize				medium
set nvTTL				16

option add Nv.port				$nvPort		  startupFile
option add Nv.chan				$nvChan		  startupFile

option add Nv.brightness			$nvBrightness	  startupFile
option add Nv.confInfo				$nvConfInfo	  startupFile
option add Nv.contrast				$nvContrast	  startupFile
option add Nv.encoding				$nvEncoding	  startupFile
option add Nv.grabber				$nvGrabber	  startupFile
option add Nv.grabPanel				$nvGrabPanel	  startupFile
option add Nv.maxBandwidth			$nvMaxBandwidth	  startupFile
option add Nv.maxBandwidthLimit			$nvMaxBWLimit	  startupFile
option add Nv.maxFrameRate			$nvMaxFrameRate	  startupFile
option add Nv.recvColor				$nvRecvColor	  startupFile
option add Nv.recvPanel				$nvRecvPanel	  startupFile
option add Nv.recvSize				$nvRecvSize	  startupFile
option add Nv.xmitColor				$nvXmitColor	  startupFile
option add Nv.xmitPanel				$nvXmitPanel	  startupFile
option add Nv.xmitSize				$nvXmitSize	  startupFile
option add Nv.title				"nv v$version"	  startupFile
option add Nv.ttl				$nvTTL		  startupFile

option add *activeBackground			$activeBgColor	  startupFile
option add *activeForeground			$activeFgColor	  startupFile
option add *selector				$FgColor	  startupFile
option add *background				$BgColor	  startupFile
option add *foreground				$FgColor	  startupFile
option add *selectBackground			$selectBgColor	  startupFile
option add *Scale.activeForeground		$activeBgColor	  startupFile
option add *Scale.sliderForeground		$BgColor	  startupFile
option add *Scrollbar.foreground		$BgColor	  startupFile
option add *Scrollbar.activeForeground		$activeBgColor	  startupFile
option add *Button.disabledForeground		$disabledFgColor  startupFile
option add *Button.disabledForeground		$disabledFgColor  startupFile
option add *Radiobutton.disabledForeground	$disabledFgColor  startupFile
option add *Menu.disabledForeground		$disabledFgColor  startupFile

proc max {args} {
    set m [lindex $args 0]
    foreach i $args {
	if {$i > $m} {set m $i}
    }
    return $m
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
    set minh [expr [winfo height .]-[max 95 [winfo height .sources]]+95]
    wm minsize . $minw $minh
    wm geometry . [wm geometry .]
    .sources config -width 1 -height 1
}

proc pressSend {} {
    set state [lindex [ .sendControls.enable config -text] 4]
    if {$state == "Start sending"} {
	startSending
    } else {
	stopSending
    }
}

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

tk_menuBar .menu .menu.info .menu.grabbers .menu.encodings .menu.panels
pack append .menu .menu.info {left}
pack append .menu .menu.grabbers {left}
pack append .menu .menu.encodings {left}
pack append .menu .menu.panels {left}

frame .sources
pack append .sources \
    [scrollbar .sources.sb -relief raised -command ".sources.canvas yview"] \
	{left filly} \
    [canvas .sources.canvas -height 95 -width 300 -relief raised \
	-scrollincrement 95 -scrollregion { 0 0 100 95 } \
	-yscroll ".sources.sb set"] {left expand fill}

frame .confInfo -borderwidth 2 -relief raised

label .confInfo.title -text "Conference info"

frame .confInfo.netinfo -relief flat
pack append .confInfo.netinfo \
    [label .confInfo.netinfo.l1 -text "Address:"] {left filly} \
    [entry .confInfo.netinfo.e1 -relief flat -width 16] {left expand fill} \
    [label .confInfo.netinfo.l2 -text "Port:"] {left filly} \
    [entry .confInfo.netinfo.e2 -relief flat -width 6] {left filly} \
    [label .confInfo.netinfo.l3 -text "Chan:"] {left filly} \
    [entry .confInfo.netinfo.e3 -relief flat -width 5] {left filly}

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

scale  .sendControls.bandwidth -command "changeMaxBandwidth" \
    -label "Max Bandwidth (kbps)" -from 1 -to $bwLimit -orient horizontal

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

bind .confInfo.netinfo.e3 <Return> {
    focus .
    .confInfo.netinfo.e3 select clear
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
bind .confInfo.netinfo.e3 <FocusOut> {changeChan [.confInfo.netinfo.e3 get]}
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
.confInfo.netinfo.e3 insert 0 [option get . chan Nv]
changeChan [.confInfo.netinfo.e3 get]
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

. config -bg $BgColor
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
