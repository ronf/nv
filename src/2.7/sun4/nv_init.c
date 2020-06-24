char InitTK[] = "\
#\n\
# General TK initialization code\n\
#\n\
\n\
proc tk_butEnter w {\n\
    global tk_priv tk_strictMotif\n\
    if {[lindex [$w config -state] 4] != \"disabled\"} {\n\
	if {!$tk_strictMotif} {\n\
	    $w config -state active\n\
	}\n\
	set tk_priv(window) $w\n\
    }\n\
}\n\
\n\
proc tk_butLeave w {\n\
    global tk_priv tk_strictMotif\n\
    if {[lindex [$w config -state] 4] != \"disabled\"} {\n\
	if {!$tk_strictMotif} {\n\
	    $w config -state normal\n\
	}\n\
    }\n\
    set tk_priv(window) \"\"\n\
}\n\
\n\
proc tk_butDown w {\n\
    global tk_priv\n\
    set tk_priv(relief) [lindex [$w config -relief] 4]\n\
    if {[lindex [$w config -state] 4] != \"disabled\"} {\n\
	$w config -relief sunken\n\
    }\n\
}\n\
\n\
proc tk_butUp w {\n\
    global tk_priv\n\
    $w config -relief $tk_priv(relief)\n\
    if {($w == $tk_priv(window))\n\
	    && ([lindex [$w config -state] 4] != \"disabled\")} {\n\
	uplevel #0 [list $w invoke]\n\
    }\n\
}\n\
\n\
proc tk_menus {w args} {\n\
    global tk_priv\n\
\n\
    if {$args == \"\"} {\n\
	if [catch {set result [set tk_priv(menusFor$w)]}] {\n\
	    return \"\"\n\
	}\n\
	return $result\n\
    }\n\
\n\
    if {$args == \"{}\"} {\n\
	catch {unset tk_priv(menusFor$w)}\n\
	return \"\"\n\
    }\n\
\n\
    set tk_priv(menusFor$w) $args\n\
}\n\
\n\
proc tk_bindForTraversal args {\n\
    foreach w $args {\n\
	bind $w <Alt-KeyPress> {tk_traverseToMenu %W %A}\n\
	bind $w <F10> {tk_firstMenu %W}\n\
    }\n\
}\n\
\n\
proc tk_mbPost {w} {\n\
    global tk_priv tk_strictMotif\n\
    if {[lindex [$w config -state] 4] == \"disabled\"} {\n\
	return\n\
    }\n\
    set cur $tk_priv(posted)\n\
    if {$cur == $w} {\n\
	return\n\
    }\n\
    if {$cur != \"\"} tk_mbUnpost\n\
    set tk_priv(relief) [lindex [$w config -relief] 4]\n\
    $w config -relief raised\n\
    set tk_priv(cursor) [lindex [$w config -cursor] 4]\n\
    $w config -cursor arrow\n\
    $w post\n\
    grab -global $w\n\
    set tk_priv(posted) $w\n\
    if {$tk_priv(focus) == \"\"} {\n\
	set tk_priv(focus) [focus]\n\
    }\n\
    set menu [lindex [$w config -menu] 4]\n\
    set tk_priv(activeBg) [lindex [$menu config -activebackground] 4]\n\
    set tk_priv(activeFg) [lindex [$menu config -activeforeground] 4]\n\
    if $tk_strictMotif {\n\
	$menu config -activebackground [lindex [$menu config -background] 4]\n\
	$menu config -activeforeground [lindex [$menu config -foreground] 4]\n\
    }\n\
    focus $menu\n\
}\n\
\n\
proc tk_mbUnpost {} {\n\
    global tk_priv\n\
    if {$tk_priv(posted) != \"\"} {\n\
	$tk_priv(posted) config -relief $tk_priv(relief)\n\
	$tk_priv(posted) config -cursor $tk_priv(cursor)\n\
	$tk_priv(posted) config -activebackground $tk_priv(activeBg)\n\
	$tk_priv(posted) config -activeforeground $tk_priv(activeFg)\n\
	$tk_priv(posted) unpost\n\
	grab none\n\
	focus $tk_priv(focus)\n\
	set tk_priv(focus) \"\"\n\
	set menu [lindex [$tk_priv(posted) config -menu] 4]\n\
	$menu config -activebackground $tk_priv(activeBg)\n\
	$menu config -activeforeground $tk_priv(activeFg)\n\
	set tk_priv(posted) {}\n\
    }\n\
}\n\
\n\
proc tk_traverseToMenu {w char} {\n\
    global tk_priv\n\
    if {$char == \"\"} {\n\
	return\n\
    }\n\
    set char [string tolower $char]\n\
\n\
    foreach mb [tk_getMenuButtons $w] {\n\
	if {[winfo class $mb] == \"Menubutton\"} {\n\
	    set char2 [string index [lindex [$mb config -text] 4] \\\n\
		    [lindex [$mb config -underline] 4]]\n\
	    if {[string compare $char [string tolower $char2]] == 0} {\n\
		tk_mbPost $mb\n\
		[lindex [$mb config -menu] 4] activate 0\n\
		return\n\
	    }\n\
	}\n\
    }\n\
}\n\
\n\
proc tk_traverseWithinMenu {w char} {\n\
    if {$char == \"\"} {\n\
	return\n\
    }\n\
    set char [string tolower $char]\n\
    set last [$w index last]\n\
    for {set i 0} {$i <= $last} {incr i} {\n\
	if [catch {set char2 [string index \\\n\
		[lindex [$w entryconfig $i -label] 4] \\\n\
		[lindex [$w entryconfig $i -underline] 4]]}] {\n\
	    continue\n\
	}\n\
	if {[string compare $char [string tolower $char2]] == 0} {\n\
	    tk_mbUnpost\n\
	    $w invoke $i\n\
	    return\n\
	}\n\
    }\n\
}\n\
\n\
proc tk_getMenuButtons w {\n\
    global tk_priv\n\
    set top [winfo toplevel $w]\n\
    if [catch {set buttons [set tk_priv(menusFor$top)]}] {\n\
	return \"\"\n\
    }\n\
    return $buttons\n\
}\n\
\n\
proc tk_nextMenu count {\n\
    global tk_priv\n\
    if {$tk_priv(posted) == \"\"} {\n\
	return\n\
    }\n\
    set buttons [tk_getMenuButtons $tk_priv(posted)]\n\
    set length [llength $buttons]\n\
    for {set i 0} 1 {incr i} {\n\
	if {$i >= $length} {\n\
	    return\n\
	}\n\
	if {[lindex $buttons $i] == $tk_priv(posted)} {\n\
	    break\n\
	}\n\
    }\n\
    incr i $count\n\
    while 1 {\n\
	while {$i < 0} {\n\
	    incr i $length\n\
	}\n\
	while {$i >= $length} {\n\
	    incr i -$length\n\
	}\n\
	set mb [lindex $buttons $i]\n\
	if {[lindex [$mb configure -state] 4] != \"disabled\"} {\n\
	    break\n\
	}\n\
	incr i $count\n\
    }\n\
    tk_mbUnpost\n\
    tk_mbPost $mb\n\
    [lindex [$mb config -menu] 4] activate 0\n\
}\n\
\n\
proc tk_nextMenuEntry count {\n\
    global tk_priv\n\
    if {$tk_priv(posted) == \"\"} {\n\
	return\n\
    }\n\
    set menu [lindex [$tk_priv(posted) config -menu] 4]\n\
    set length [expr [$menu index last]+1]\n\
    set i [$menu index active]\n\
    if {$i == \"none\"} {\n\
	set i 0\n\
    } else {\n\
	incr i $count\n\
    }\n\
    while 1 {\n\
	while {$i < 0} {\n\
	    incr i $length\n\
	}\n\
	while {$i >= $length} {\n\
	    incr i -$length\n\
	}\n\
	if {[catch {$menu entryconfigure $i -state} state] == 0} {\n\
	    if {[lindex $state 4] != \"disabled\"} {\n\
		break\n\
	    }\n\
	}\n\
	incr i $count\n\
    }\n\
    $menu activate $i\n\
}\n\
\n\
proc tk_invokeMenu {menu} {\n\
    set i [$menu index active]\n\
    if {$i != \"none\"} {\n\
	tk_mbUnpost\n\
	update idletasks\n\
	$menu invoke $i\n\
    }\n\
}\n\
\n\
proc tk_firstMenu w {\n\
    set mb [lindex [tk_getMenuButtons $w] 0]\n\
    if {$mb != \"\"} {\n\
	tk_mbPost $mb\n\
	[lindex [$mb config -menu] 4] activate 0\n\
    }\n\
}\n\
\n\
proc tk_mbButtonDown w {\n\
    global tk_priv\n\
    if {[lindex [$w config -state] 4] == \"disabled\"} {\n\
	return\n\
    } \n\
    if {$tk_priv(inMenuButton) == $w} {\n\
	tk_mbPost $w\n\
    }\n\
    set menu [lindex [$tk_priv(posted) config -menu] 4]\n\
    if {$tk_priv(window) != $menu} {\n\
	$menu activate none\n\
    }\n\
}\n\
\n\
proc tk_entryForwardChar w {\n\
    set x [$w index cursor]\n\
    $w cursor [incr x +1]\n\
}\n\
\n\
proc tk_entryBackChar w {\n\
    set x [$w index cursor]\n\
    $w cursor [incr x -1]\n\
}\n\
\n\
proc tk_entryBackspace w {\n\
    if [catch {$w delete sel.first sel.last}] {\n\
	set x [expr {[$w index cursor] - 1}]\n\
	if {$x != -1} {$w delete $x}\n\
    }\n\
}\n\
\n\
proc tk_entryDelRight w {\n\
    if [catch {$w delete sel.first sel.last}] {\n\
	set x [$w index cursor]\n\
	if {$x != -1} {$w delete $x}\n\
    }\n\
}\n\
\n\
proc tk_entryKillRight w {\n\
    if [catch {$w delete sel.first sel.last}] {\n\
	set x [$w index cursor]\n\
	if {$x != -1} {$w delete $x end}\n\
    }\n\
}\n\
\n\
proc tk_entrySeeCaret w {\n\
    set c [$w index cursor]\n\
    set left [$w index @0]\n\
    if {$left > $c} {\n\
	$w view $c\n\
	return\n\
    }\n\
    while {[$w index @[expr [winfo width $w]-5]] < $c} {\n\
	set left [expr $left+1]\n\
	$w view $left\n\
    }\n\
}\n\
\n\
set tk_strictMotif 0\n\
\n\
bind Button <Any-Enter> {tk_butEnter %W}\n\
bind Button <Any-Leave> {tk_butLeave %W}\n\
bind Button <1> {tk_butDown %W}\n\
bind Button <ButtonRelease-1> {tk_butUp %W}\n\
\n\
bind CheckButton <Any-Enter> {tk_butEnter %W}\n\
bind CheckButton <Any-Leave> {tk_butLeave %W}\n\
bind CheckButton <1> {tk_butDown %W}\n\
bind CheckButton <ButtonRelease-1> {tk_butUp %W}\n\
\n\
bind RadioButton <Any-Enter> {tk_butEnter %W}\n\
bind RadioButton <Any-Leave> {tk_butLeave %W}\n\
bind RadioButton <1> {tk_butDown %W}\n\
bind RadioButton <ButtonRelease-1> {tk_butUp %W}\n\
\n\
bind Entry <1> {\n\
    %W cursor @%x\n\
    %W select from @%x\n\
    if {[lindex [%W config -state] 4] == \"normal\"} {focus %W}\n\
}\n\
bind Entry <B1-Motion> {%W select to @%x}\n\
bind Entry <Double-1> {%W select from 0; %W select to end}\n\
bind Entry <Triple-1> {%W select from 0; %W select to end}\n\
bind Entry <2> {%W insert cursor [selection get]; tk_entrySeeCaret %W}\n\
bind Entry <3> {%W select adjust @%x}\n\
bind Entry <B3-Motion> {%W select to @%x}\n\
\n\
bind Entry <Any-KeyPress> {\n\
    if {\"%A\" != \"\"} {\n\
	catch {%W delete sel.first sel.last}\n\
	%W insert cursor %A\n\
	tk_entrySeeCaret %W\n\
    }\n\
}\n\
\n\
bind Entry <Control-a> {%W cursor -1; tk_entrySeeCaret %W}\n\
bind Entry <Control-b> {tk_entryBackChar %W; tk_entrySeeCaret %W}\n\
bind Entry <Control-d> {tk_entryDelRight %W; tk_entrySeeCaret %W}\n\
bind Entry <Control-e> {%W cursor end; tk_entrySeeCaret %W}\n\
bind Entry <Control-f> {tk_entryForwardChar %W; tk_entrySeeCaret %W}\n\
bind Entry <Control-h> {tk_entryBackspace %W; tk_entrySeeCaret %W}\n\
bind Entry <Control-k> {tk_entryKillRight %W; tk_entrySeeCaret %W}\n\
bind Entry <Control-u> {%W delete 0 end}\n\
bind Entry <Delete> {tk_entryBackspace %W; tk_entrySeeCaret %W}\n\
bind Entry <BackSpace> {tk_entryBackspace %W; tk_entrySeeCaret %W}\n\
\n\
tk_bindForTraversal Entry\n\
\n\
bind Scrollbar <Any-Enter> {\n\
    if $tk_strictMotif {\n\
	set tk_priv(activeFg) [lindex [%W config -activeforeground] 4]\n\
	%W config -activeforeground [lindex [%W config -foreground] 4]\n\
    }\n\
}\n\
bind Scrollbar <Any-Leave> {\n\
    if {$tk_strictMotif && ($tk_priv(buttons) == 0)} {\n\
	%W config -activeforeground $tk_priv(activeFg)\n\
    }\n\
}\n\
bind Scrollbar <Any-ButtonPress> {incr tk_priv(buttons)}\n\
bind Scrollbar <Any-ButtonRelease> {incr tk_priv(buttons) -1}\n\
\n\
bind Scale <Any-Enter> {\n\
    if $tk_strictMotif {\n\
	set tk_priv(activeFg) [lindex [%W config -activeforeground] 4]\n\
	%W config -activeforeground [lindex [%W config -sliderforeground] 4]\n\
    }\n\
}\n\
bind Scale <Any-Leave> {\n\
    if {$tk_strictMotif && ($tk_priv(buttons) == 0)} {\n\
	%W config -activeforeground $tk_priv(activeFg)\n\
    }\n\
}\n\
bind Scale <Any-ButtonPress> {incr tk_priv(buttons)}\n\
bind Scale <Any-ButtonRelease> {incr tk_priv(buttons) -1}\n\
\n\
bind Menubutton <Enter> {\n\
    set tk_priv(inMenuButton) %W\n\
    if {[lindex [%W config -state] 4] != \"disabled\"} {\n\
	if {!$tk_strictMotif} {\n\
	    %W config -state active\n\
	}\n\
    }\n\
}\n\
bind Menubutton <Any-Leave> {\n\
    set tk_priv(inMenuButton) {}\n\
    if {[lindex [%W config -state] 4] != \"disabled\"} {\n\
	if {!$tk_strictMotif} {\n\
	    %W config -state normal\n\
	}\n\
    }\n\
}\n\
bind Menubutton <1> {tk_mbButtonDown %W}\n\
bind Menubutton <Any-ButtonRelease-1> {\n\
    if {($tk_priv(inMenuButton) != \"\") && ($tk_priv(posted) != \"\")} {\n\
	[lindex [$tk_priv(posted) config -menu] 4] activate 0\n\
    } else {\n\
	tk_mbUnpost\n\
    }\n\
}\n\
\n\
bind Menubutton <B1-Enter> {\n\
    set tk_priv(inMenuButton) %W\n\
    if {([lindex [%W config -state] 4] != \"disabled\")\n\
	    && (\"%m\" != \"NotifyGrab\") && (\"%m\" != \"NotifyUngrab\")} {\n\
	if {!$tk_strictMotif} {\n\
	    %W config -state active\n\
	}\n\
	tk_mbPost %W\n\
    }\n\
}\n\
bind Menubutton <2> {\n\
    if {($tk_priv(posted) == \"\")\n\
	    && ([lindex [%W config -state] 4] != \"disabled\")} {\n\
	set tk_priv(dragging) %W\n\
	[lindex [$tk_priv(dragging) config -menu] 4] post %X %Y\n\
    }\n\
}\n\
bind Menubutton <B2-Motion> {\n\
    if {$tk_priv(dragging) != \"\"} {\n\
	[lindex [$tk_priv(dragging) config -menu] 4] post %X %Y\n\
    }\n\
}\n\
bind Menubutton <ButtonRelease-2> {set tk_priv(dragging) \"\"}\n\
\n\
bind Menu <Any-Leave> {set tk_priv(window) {}; %W activate none}\n\
bind Menu <Any-Motion> {\n\
    if {$tk_priv(window) != \"\"} {\n\
	%W activate @%y\n\
    }\n\
}\n\
bind Menu <ButtonRelease-1> {tk_invokeMenu %W}\n\
bind Menu <2> {set tk_priv(x) %x; set tk_priv(y) %y}\n\
bind Menu <B2-Motion> {\n\
    if {$tk_priv(posted) == \"\"} {\n\
	%W post [expr %X-$tk_priv(x)] [expr %Y-$tk_priv(y)]\n\
    }\n\
}\n\
bind Menu <B2-Leave> { }\n\
bind Menu <B2-Enter> { }\n\
bind Menu <Escape> {tk_mbUnpost}\n\
bind Menu <Any-KeyPress> {tk_traverseWithinMenu %W %A}\n\
bind Menu <Left> {tk_nextMenu -1}\n\
bind Menu <Right> {tk_nextMenu 1}\n\
bind Menu <Up> {tk_nextMenuEntry -1}\n\
bind Menu <Down> {tk_nextMenuEntry 1}\n\
bind Menu <Return> {tk_invokeMenu %W}\n\
\n\
bind Text <1> {\n\
    set tk_priv(selectMode) char\n\
    %W mark set insert @%x,%y\n\
    %W mark set anchor insert\n\
    if {[lindex [%W config -state] 4] == \"normal\"} {focus %W}\n\
}\n\
bind Text <Double-1> {\n\
    set tk_priv(selectMode) word\n\
    %W mark set insert \"@%x,%y wordstart\"\n\
    tk_textSelectTo %W insert\n\
}\n\
bind Text <Triple-1> {\n\
    set tk_priv(selectMode) line\n\
    %W mark set insert \"@%x,%y linestart\"\n\
    tk_textSelectTo %W insert\n\
}\n\
bind Text <B1-Motion> {tk_textSelectTo %W @%x,%y}\n\
bind Text <Shift-1> {\n\
    tk_textResetAnchor %W @%x,%y\n\
    tk_textSelectTo %W @%x,%y\n\
}\n\
bind Text <Shift-B1-Motion> {tk_textSelectTo %W @%x,%y}\n\
bind Text <2> {%W scan mark %y}\n\
bind Text <B2-Motion> {%W scan dragto %y}\n\
bind Text <Any-KeyPress> {\n\
    if {\"%A\" != \"\"} {\n\
	%W insert insert %A\n\
	%W yview -pickplace insert\n\
    }\n\
}\n\
bind Text <Return> {%W insert insert \\n; %W yview -pickplace insert}\n\
bind Text <BackSpace> {tk_textBackspace %W; %W yview -pickplace insert}\n\
bind Text <Delete> {tk_textBackspace %W; %W yview -pickplace insert}\n\
bind Text <Control-h> {tk_textBackspace %W; %W yview -pickplace insert}\n\
bind Text <Control-d> {%W delete sel.first sel.last}\n\
bind Text <Control-v> {\n\
    %W insert insert [selection get]\n\
    %W yview -pickplace insert\n\
}\n\
tk_bindForTraversal Text\n\
\n\
# Initialize the elements of tk_priv that require initialization.\n\
\n\
set tk_priv(buttons) 0\n\
set tk_priv(dragging) {}\n\
set tk_priv(focus) {}\n\
set tk_priv(inMenuButton) {}\n\
set tk_priv(posted) {}\n\
set tk_priv(selectMode) char\n\
set tk_priv(window) {}\n\
";
char InitNVProcs[] = "\
#\n\
# nvprocs.tk - TK procedures used by the netvideo application\n\
#\n\
\n\
proc envVal { envValName } {\n\
    global env\n\
    if [info exists env($envValName)] {\n\
	return $env($envValName)\n\
    } else {\n\
	return {}\n\
    }\n\
}\n\
\n\
proc doSubst { s pat rep } {\n\
    while { 1 } {\n\
	set a [expr \"[string first $pat $s] - 1\"]\n\
	set b [expr \"$a + [string length $pat] + 1\"]\n\
	if { $a == -2 } break\n\
	set s \"[string range $s 0 $a]${rep}[string range $s $b end]\"\n\
    }\n\
    return $s\n\
}\n\
\n\
proc loadAppDefaults { classNameList {priority startupFile} } {\n\
    set filepath \"[split [envVal XUSERFILESEARCHPATH] :] \\\n\
		  [envVal XAPPLRESDIR]/%N \\\n\
		  [split [envVal XFILESEARCHPATH] :] \\\n\
		  /usr/lib/X11/%T/%N%S\"\n\
    foreach i $classNameList {\n\
	foreach j $filepath {\n\
	    set f [doSubst [doSubst [doSubst $j  %T app-defaults] %S .ad] %N $i]\n\
	    if {[file exists $f]} {\n\
		catch \"option readfile $f $priority\"\n\
		break\n\
	    }\n\
	}\n\
    }\n\
}\n\
\n\
proc recursiveBind { w event action } {\n\
    bind $w $event $action\n\
    foreach child [winfo children $w] { recursiveBind $child $event $action }\n\
}\n\
\n\
proc pressSend { } {\n\
    set state [lindex [ .sendControls.enable config -text] 4]\n\
    if { $state == \"Start sending\" } {\n\
	sendVideo 1\n\
	.sendControls.enable config -text \"Stop sending\"\n\
    } else {\n\
	sendVideo 0\n\
	.sendControls.enable config -text \"Start sending\"\n\
    }\n\
}\n\
\n\
proc pressReceivers { } {\n\
    set state [lindex [ .sendControls.receivers config -text] 4]\n\
    if { $state == \"Show receivers\" } {\n\
	wm deiconify .receivers\n\
	.sendControls.receivers config -text \"Hide receivers\"\n\
    } else {\n\
	wm withdraw .receivers\n\
	.sendControls.receivers config -text \"Show receivers\"\n\
    }\n\
}\n\
\n\
proc showRecvPanel { source } {\n\
    pack append .nvsource$source .nvsource$source.ctl { top expand fill }\n\
    bind .nvsource$source <1> \"hideRecvPanel $source\"\n\
    bind .nvsource$source.video <1> \"hideRecvPanel $source\"\n\
    bind .nvsource$source <2> \"hideRecvPanel $source\"\n\
    bind .nvsource$source.video <2> \"hideRecvPanel $source\"\n\
    bind .nvsource$source <3> \"hideRecvPanel $source\"\n\
    bind .nvsource$source.video <3> \"hideRecvPanel $source\"\n\
}\n\
\n\
proc hideRecvPanel { source } {\n\
    pack unpack .nvsource$source.ctl\n\
    bind .nvsource$source <1> \"showRecvPanel $source\"\n\
    bind .nvsource$source.video <1> \"showRecvPanel $source\"\n\
    bind .nvsource$source <2> \"showRecvPanel $source\"\n\
    bind .nvsource$source.video <2> \"showRecvPanel $source\"\n\
    bind .nvsource$source <3> \"showRecvPanel $source\"\n\
    bind .nvsource$source.video <3> \"showRecvPanel $source\"\n\
}\n\
\n\
proc checkSource { source } {\n\
    global source$source\n\
    set enabled [set source$source]\n\
    set name [lindex [ .sources.source$source config -text] 4]\n\
    receiveVideo $source $enabled\n\
    if $enabled {\n\
	wm deiconify .nvsource$source\n\
    } else {\n\
	wm withdraw .nvsource$source\n\
    }\n\
}\n\
\n\
proc setRecvSize { source size } {\n\
    case $size in {\n\
	half	{ set scaler 2 }\n\
	default	{ set scaler 1 }\n\
	double	{ set scaler 0 }\n\
    }\n\
    .nvsource$source.ctl.sizebuttons.half config -state normal\n\
    .nvsource$source.ctl.sizebuttons.normal config -state normal\n\
    .nvsource$source.ctl.sizebuttons.double config -state normal\n\
    .nvsource$source.ctl.sizebuttons.$size config -state disabled\n\
    .nvsource$source.video scale $scaler\n\
}\n\
\n\
proc copySource { source } {\n\
    global copy\n\
    set name [lindex [ .sources.source$source config -text] 4]\n\
    toplevel .nvcopy$copy\n\
    wm title .nvcopy$copy \"$name (captured)\"\n\
    .nvsource$source.video copy .nvcopy$copy\n\
    bind .nvcopy$copy <1> \"destroy .nvcopy$copy\"\n\
    bind .nvcopy$copy <2> \"destroy .nvcopy$copy\"\n\
    bind .nvcopy$copy <3> \"destroy .nvcopy$copy\"\n\
    set copy [expr \"$copy + 1\"]\n\
}\n\
\n\
proc addSource { source name } {\n\
    pack before .sources.fill \\\n\
	[ checkbutton .sources.source$source -anchor w \\\n\
	    -command \"checkSource $source\" -padx 5 -relief flat \\\n\
	    -text $name ] { top fillx }\n\
\n\
    frame .nvsource$source.ctl -borderwidth 2 -relief raised\n\
\n\
    frame .nvsource$source.ctl.sizebuttons\n\
    pack append .nvsource$source.ctl.sizebuttons \\\n\
	[ button .nvsource$source.ctl.sizebuttons.half -text \"Half\" \\\n\
	    -command \"setRecvSize $source half\" ] { left expand fill } \\\n\
	[ button .nvsource$source.ctl.sizebuttons.normal -text \"Normal\" \\\n\
	    -command \"setRecvSize $source normal\" ] { left expand fill } \\\n\
	[ button .nvsource$source.ctl.sizebuttons.double -text \"Double\" \\\n\
	    -command \"setRecvSize $source double\" ] { left expand fill }\n\
\n\
    scale .nvsource$source.ctl.brightness -command \"setBrightness $source\" \\\n\
	-label \"Brightness\" -from 0 -to 100 -orient horizontal\n\
\n\
    scale .nvsource$source.ctl.contrast -command \"setContrast $source\" \\\n\
	-label \"Contrast\" -from 0 -to 100 -orient horizontal\n\
\n\
    frame .nvsource$source.ctl.frames -relief flat\n\
    pack append .nvsource$source.ctl.frames \\\n\
	[ label .nvsource$source.ctl.frames.label -anchor w -width 10 \\\n\
	    -text \"Frame rate:\" ] { left filly } \\\n\
	[ label .nvsource$source.ctl.frames.fps -anchor e -width 9 ] \\\n\
	    { left filly } \\\n\
	[ frame .nvsource$source.ctl.frames.fill ] { left expand fill }\n\
\n\
    frame .nvsource$source.ctl.bytes -relief flat\n\
    pack append .nvsource$source.ctl.bytes \\\n\
	[ label .nvsource$source.ctl.bytes.label -anchor w -width 10 \\\n\
	    -text \"Bandwidth:\" ] { left filly } \\\n\
	[ label .nvsource$source.ctl.bytes.bps -anchor e -width 9 ] \\\n\
	    { left filly } \\\n\
	[ frame .nvsource$source.ctl.bytes.fill ] { left expand fill }\n\
\n\
    button .nvsource$source.ctl.capture -text \"Capture\" \\\n\
	-command \"copySource $source\"\n\
\n\
    wm title .nvsource$source $name\n\
    wm withdraw .nvsource$source\n\
\n\
    pack append .nvsource$source.ctl \\\n\
	.nvsource$source.ctl.brightness { top expand fillx pady 4 frame s } \\\n\
	.nvsource$source.ctl.contrast { top expand fill } \\\n\
	.nvsource$source.ctl.frames { top expand fill } \\\n\
	.nvsource$source.ctl.bytes { top expand fill } \\\n\
	.nvsource$source.ctl.sizebuttons { top expand fill } \\\n\
	.nvsource$source.ctl.capture { top expand fill }\n\
\n\
    pack append .nvsource$source .nvsource$source.video { top expand fill }\n\
\n\
    bind .sources.source$source <Control-c> {shutdown}\n\
    bind .sources.source$source <q> {shutdown}\n\
    recursiveBind .nvsource$source <Control-c> \\\n\
	\"set source$source 0; checkSource $source\"\n\
    recursiveBind .nvsource$source <q> \\\n\
	\"set source$source 0; checkSource $source\"\n\
    bind .nvsource$source <1> \"showRecvPanel $source\"\n\
    bind .nvsource$source.video <1> \"showRecvPanel $source\"\n\
    bind .nvsource$source <2> \"showRecvPanel $source\"\n\
    bind .nvsource$source.video <2> \"showRecvPanel $source\"\n\
    bind .nvsource$source <3> \"showRecvPanel $source\"\n\
    bind .nvsource$source.video <3> \"showRecvPanel $source\"\n\
\n\
    if [catch \".nvsource[set source].ctl.brightness set \\\n\
		   [option get . brightness Nv]\"] {\n\
	.nvsource$source.ctl.brightness set $nvBrightness\n\
    }\n\
\n\
    if [catch \".nvsource[set source].ctl.contrast set \\\n\
		   [option get . contrast Nv]\"] {\n\
	.nvsource$source.ctl.contrast set $nvContrast\n\
    }\n\
\n\
    setRecvSize $source [option get . recvSize Nv]\n\
}\n\
\n\
proc deleteSource { source } {\n\
    global source$source\n\
    set enabled [set source$source]\n\
    if $enabled {\n\
	receiveVideo $source 0\n\
    }\n\
    destroy .nvsource$source\n\
    destroy .sources.source$source\n\
}\n\
\n\
proc setSourceName { source name } {\n\
    .sources.source$source config -text $name\n\
    wm title .nvsource$source $name\n\
}\n\
\n\
proc setFPSandBPS { source fps bps } {\n\
    if { $fps == \"\" } {\n\
	.nvsource$source.ctl.frames.fps config -text \"\"\n\
	.nvsource$source.ctl.bytes.bps config -text \"\"\n\
    } else {\n\
	.nvsource$source.ctl.frames.fps config -text \"$fps  fps\"\n\
	.nvsource$source.ctl.bytes.bps config -text \"$bps kbps\"\n\
    }\n\
}\n\
";
char InitNV[] = "\
#\n\
# nv.tk - A TK interface to the netvideo application\n\
#\n\
\n\
set version \"2.7\"\n\
set copy 0\n\
\n\
set screenDepth [winfo screendepth .]\n\
case $screenDepth in {\n\
    1		{\n\
		  set activeBgColor	black\n\
		  set activeFgColor	white\n\
		  set disabledFgColor	\"\"\n\
		  set BgColor		white\n\
		  set FgColor		black\n\
		}\n\
    default	{\n\
		  set activeBgColor	#dddddd\n\
		  set activeFgColor	black\n\
		  set disabledFgColor	#777777\n\
		  set BgColor		#bbbbbb\n\
		  set FgColor		black\n\
		}\n\
}\n\
\n\
set nvBrightness			60\n\
set nvContrast				60\n\
set nvMaxBandwidth			128\n\
set nvRecvSize				normal\n\
set nvTTL				16\n\
\n\
option add Nv.brightness		$nvBrightness		startupFile\n\
option add Nv.contrast			$nvContrast		startupFile\n\
option add Nv.maxBandwidth		$nvMaxBandwidth		startupFile\n\
option add Nv.recvSize			$nvRecvSize		startupFile\n\
option add Nv.ttl			$nvTTL			startupFile\n\
\n\
option add *activeBackground		$activeBgColor		startupFile\n\
option add *activeForeground		$activeFgColor		startupFile\n\
option add *CheckButton.selector	$FgColor		startupFile\n\
option add *background			$BgColor		startupFile\n\
option add *foreground			$FgColor		startupFile\n\
option add *Scale.activeForeground	$activeBgColor		startupFile\n\
option add *Scale.sliderForeground	$BgColor		startupFile\n\
option add *Button.disabledForeground	$disabledFgColor	startupFile\n\
\n\
loadAppDefaults {Nv NV} userDefault\n\
if { [envVal XENVIRONMENT] != {} } {\n\
    option readfile [envVal XENVIRONMENT] userDefault\n\
}\n\
\n\
frame .sources -borderwidth 2 -relief raised\n\
pack append .sources \\\n\
	[ label .sources.title -text \"Network video sources\" ] { top fillx } \\\n\
	[ frame .sources.fill ] { top expand fill }\n\
\n\
frame .confControls -borderwidth 2 -relief raised\n\
\n\
label .confControls.title -text \"Conference info\"\n\
\n\
frame .confControls.netinfo -relief flat\n\
pack append .confControls.netinfo \\\n\
	[ label .confControls.netinfo.l1 -text \"Address:\" ] { left filly } \\\n\
	[ entry .confControls.netinfo.e1 -relief flat -width 14 ] \\\n\
		{ left expand fill } \\\n\
	[ label .confControls.netinfo.l2 -text \"Port:\" ] { left filly } \\\n\
	[ entry .confControls.netinfo.e2 -relief flat -width 4 ] \\\n\
		{ left filly } \\\n\
	[ label .confControls.netinfo.l3 -text \"TTL:\" ] { left filly } \\\n\
	[ entry .confControls.netinfo.e3 -relief flat -width 1 ] \\\n\
		{ left filly }\n\
\n\
frame .confControls.name -relief flat\n\
pack append .confControls.name \\\n\
	[ label .confControls.name.l1 -text \"Name:\" ] { left filly } \\\n\
	[ entry .confControls.name.e1 -relief flat -width 1 ] \\\n\
		{ left expand fill }\n\
\n\
pack append .confControls \\\n\
	.confControls.title { top fillx } \\\n\
	.confControls.netinfo { top fill } \\\n\
	.confControls.name { top fill }\n\
\n\
frame .sendControls -borderwidth 2 -relief raised\n\
\n\
label  .sendControls.title -text \"Video transmit options\"\n\
\n\
scale  .sendControls.bandwidth -command \"setBandwidth\" \\\n\
	-label \"Max Bandwidth (kbps)\" -from 0 -to 512 -orient horizontal\n\
\n\
button .sendControls.enable -text \"Start sending\" -command \"pressSend\"\n\
button .sendControls.receivers -text \"Show receivers\" -command \"pressReceivers\"\n\
\n\
bind .confControls.netinfo.e1 <Return> {\n\
    focus .\n\
    setAddress [.confControls.netinfo.e1 get]\n\
    .confControls.netinfo.e1 select clear\n\
}\n\
\n\
bind .confControls.netinfo.e2 <Return> {\n\
    focus .\n\
    setPort [.confControls.netinfo.e2 get]\n\
    .confControls.netinfo.e2 select clear\n\
}\n\
\n\
bind .confControls.netinfo.e3 <Return> {\n\
    focus .\n\
    setTTL [.confControls.netinfo.e3 get]\n\
    .confControls.netinfo.e3 select clear\n\
}\n\
\n\
bind .confControls.name.e1 <Return> {\n\
    focus .\n\
    setName [.confControls.name.e1 get]\n\
    .confControls.name.e1 select clear\n\
}\n\
\n\
if [catch {.sendControls.bandwidth set [option get . maxBandwidth Nv]}] {\n\
    .sendControls.maxBandwidth set $nvMaxBandwidth\n\
}\n\
\n\
.confControls.netinfo.e1 insert 0 [option get . address Nv]\n\
setAddress [.confControls.netinfo.e1 get]\n\
.confControls.netinfo.e2 insert 0 [option get . port Nv]\n\
setPort [.confControls.netinfo.e2 get]\n\
.confControls.netinfo.e3 insert 0 [option get . ttl Nv]\n\
setTTL [.confControls.netinfo.e3 get]\n\
.confControls.name.e1 insert 0 [option get . name Nv]\n\
setName [.confControls.name.e1 get]\n\
sendVideo 0\n\
\n\
pack append .sendControls \\\n\
	.sendControls.title { top expand fillx pady 2 } \\\n\
	.sendControls.bandwidth { top expand fillx pady 2 } \\\n\
	.sendControls.enable { top expand fillx pady 2 } \\\n\
	.sendControls.receivers { top expand fillx pady 2 }\n\
\n\
toplevel .receivers\n\
pack append .receivers \\\n\
    [scrollbar .receivers.scroll -relief flat \\\n\
	-command \".receivers.list yview\"] { left filly } \\\n\
    [listbox .receivers.list -geometry 60x10 -yscroll \".receivers.scroll set\" \\\n\
	-relief flat] { left expand fill }\n\
wm minsize .receivers 0 0\n\
wm title .receivers \"nv receivers\"\n\
wm withdraw .receivers\n\
\n\
pack append . \\\n\
	.sources { top expand fill } \\\n\
	.confControls { top fill }\n\
\n\
\n\
global send_allowed\n\
if $send_allowed { pack append . .sendControls { top fillx } }\n\
\n\
update\n\
set reqw [winfo reqwidth .]\n\
set reqh [winfo reqheight .]\n\
set lineh [ winfo reqheight .sources.title ]\n\
\n\
wm minsize . $reqw [expr \"$reqh + $lineh\"]\n\
wm geometry . ${reqw}x[expr \"$reqh + $lineh * 5\"]\n\
wm title . \"nv v$version\"\n\
\n\
recursiveBind . <Control-c> {shutdown}\n\
recursiveBind . <q> {shutdown}\n\
recursiveBind .confControls.netinfo.e1 <q> {}\n\
recursiveBind .confControls.netinfo.e2 <q> {}\n\
recursiveBind .confControls.netinfo.e3 <q> {}\n\
recursiveBind .confControls.name.e1 <q> {}\n\
recursiveBind .receivers <Control-c> {}\n\
recursiveBind .receivers <q> {}\n\
recursiveBind .receivers <Control-c> {pressReceivers}\n\
recursiveBind .receivers <q> {pressReceivers}\n\
";
