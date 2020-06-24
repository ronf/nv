char TK_Init[] = "\
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
proc tk_menuBar {w args} {\n\
    global tk_priv\n\
    if {$args == \"\"} {\n\
	if [catch {set menus [set tk_priv(menusFor$w)]}] {\n\
	    return \"\"\n\
	}\n\
	return $menus\n\
    }\n\
    if [info exists tk_priv(menusFor$w)] {\n\
	unset tk_priv(menusFor$w)\n\
	unset tk_priv(menuBarFor[winfo toplevel $w])\n\
    }\n\
    if {$args == \"{}\"} {\n\
	return\n\
    }\n\
\n\
    set tk_priv(menusFor$w) $args\n\
    set tk_priv(menuBarFor[winfo toplevel $w]) $w\n\
    bind $w <Any-ButtonRelease-1> tk_mbUnpost\n\
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
    if {([lindex [$w config -state] 4] == \"disabled\")\n\
	|| ($w == $tk_priv(posted))} {\n\
	return\n\
    }\n\
    set menu [lindex [$w config -menu] 4]\n\
    if {$menu == \"\"} {\n\
	return\n\
    }\n\
    if ![string match $w* $menu] {\n\
	error \"can't post $menu: it isn't a descendant of $w\"\n\
    }\n\
    set cur $tk_priv(posted)\n\
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
    $menu activate none\n\
    focus $menu\n\
    $menu post [winfo rootx $w] [expr [winfo rooty $w]+[winfo height $w]]\n\
    if [catch {set grab $tk_priv(menuBarFor[winfo toplevel $w])}] {\n\
	set grab $w\n\
    } else {\n\
	if [lsearch $tk_priv(menusFor$grab) $w]<0 {\n\
	    set grab $w\n\
	}\n\
    }\n\
    set tk_priv(cursor) [lindex [$grab config -cursor] 4]\n\
    $grab config -cursor arrow\n\
    set tk_priv(grab) $grab\n\
    grab -global $grab\n\
}\n\
\n\
proc tk_mbUnpost {} {\n\
    global tk_priv\n\
    set w $tk_priv(posted)\n\
    if {$w != \"\"} {\n\
	$w config -relief $tk_priv(relief)\n\
	$tk_priv(grab) config -cursor $tk_priv(cursor)\n\
	grab release $tk_priv(grab)\n\
	focus $tk_priv(focus)\n\
	set tk_priv(focus) \"\"\n\
	set menu [lindex [$w config -menu] 4]\n\
	$menu unpost\n\
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
    if [catch {set buttons [set tk_priv(menuBarFor$top)]}] {\n\
	return \"\"\n\
    }\n\
    return $tk_priv(menusFor$bar)\n\
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
}\n\
\n\
proc tk_entryForwardChar w {\n\
    set x [$w index insert]\n\
    $w icursor [incr x +1]\n\
}\n\
\n\
proc tk_entryBackChar w {\n\
    set x [$w index insert]\n\
    $w icursor [incr x -1]\n\
}\n\
\n\
proc tk_entryBackspace w {\n\
    if [catch {$w delete sel.first sel.last}] {\n\
	set x [expr {[$w index insert] - 1}]\n\
	if {$x != -1} {$w delete $x}\n\
    }\n\
}\n\
\n\
proc tk_entryDelRight w {\n\
    if [catch {$w delete sel.first sel.last}] {\n\
	set x [$w index insert]\n\
	if {$x != -1} {$w delete $x}\n\
    }\n\
}\n\
\n\
proc tk_entryKillRight w {\n\
    if [catch {$w delete sel.first sel.last}] {\n\
	set x [$w index insert]\n\
	if {$x != -1} {$w delete $x end}\n\
    }\n\
}\n\
\n\
proc tk_entrySeeCaret w {\n\
    set c [$w index insert]\n\
    set left [$w index @0]\n\
    if {$left > $c} {\n\
	$w view $c\n\
	return\n\
    }\n\
    while {([$w index @[expr [winfo width $w]-5]] < $c)\n\
	  && ($left < $c)} {\n\
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
bind Checkbutton <Any-Enter> {tk_butEnter %W}\n\
bind Checkbutton <Any-Leave> {tk_butLeave %W}\n\
bind Checkbutton <1> {tk_butDown %W}\n\
bind Checkbutton <ButtonRelease-1> {tk_butUp %W}\n\
\n\
bind Radiobutton <Any-Enter> {tk_butEnter %W}\n\
bind Radiobutton <Any-Leave> {tk_butLeave %W}\n\
bind Radiobutton <1> {tk_butDown %W}\n\
bind Radiobutton <ButtonRelease-1> {tk_butUp %W}\n\
\n\
bind Entry <1> {\n\
    %W icursor @%x\n\
    %W select from @%x\n\
    if {[lindex [%W config -state] 4] == \"normal\"} {focus %W}\n\
}\n\
bind Entry <B1-Motion> {%W select to @%x}\n\
bind Entry <Double-1> {%W select from 0; %W select to end}\n\
bind Entry <Triple-1> {%W select from 0; %W select to end}\n\
bind Entry <2> {%W insert insert [selection get]; tk_entrySeeCaret %W}\n\
bind Entry <3> {%W select adjust @%x}\n\
bind Entry <B3-Motion> {%W select to @%x}\n\
\n\
bind Entry <Any-KeyPress> {\n\
    if {\"%A\" != \"\"} {\n\
	catch {%W delete sel.first sel.last}\n\
	%W insert insert %A\n\
	tk_entrySeeCaret %W\n\
    }\n\
}\n\
\n\
bind Entry <Control-a> {%W icursor -1; tk_entrySeeCaret %W}\n\
bind Entry <Control-b> {tk_entryBackChar %W; tk_entrySeeCaret %W}\n\
bind Entry <Control-d> {tk_entryDelRight %W; tk_entrySeeCaret %W}\n\
bind Entry <Control-e> {%W icursor end; tk_entrySeeCaret %W}\n\
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
bind Menubutton <Any-Enter> {\n\
    set tk_priv(inMenuButton) %W\n\
    if {[lindex [%W config -state] 4] != \"disabled\"} {\n\
	if {!$tk_strictMotif} {\n\
	    %W config -state active\n\
	}\n\
    }\n\
}\n\
bind Menubutton <Any-Leave> {\n\
    set tk_priv(inMenuButton) {}\n\
    if {[lindex [%W config -state] 4] == \"active\"} {\n\
	%W config -state normal\n\
    }\n\
}\n\
bind Menubutton <1> {tk_mbButtonDown %W}\n\
bind Menubutton <Any-ButtonRelease-1> {\n\
    if {($tk_priv(posted) == \"%W\") && ($tk_priv(inMenuButton) == \"%W\")} {\n\
	[lindex [$tk_priv(posted) config -menu] 4] activate 0\n\
    } else {\n\
	tk_mbUnpost\n\
    }\n\
}\n\
\n\
bind Menubutton <B1-Enter> {\n\
    set tk_priv(inMenuButton) %W\n\
    if {[lindex [%W config -state] 4] != \"disabled\")} {\n\
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
bind Menu <Any-Motion> {%W activate @%y}\n\
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
proc tk_textSelectTo {w index} {\n\
    global tk_priv\n\
 \n\
    case $tk_priv(selectMode) {\n\
	char {\n\
	    if [$w compare $index < anchor] {\n\
		set first $index\n\
		set last anchor\n\
	    } else {\n\
		set first anchor\n\
		set last [$w index $index+1c]\n\
	    }\n\
	}\n\
	word {\n\
	    if [$w compare $index < anchor] {\n\
		set first [$w index \"$index wordstart\"]\n\
		set last [$w index \"anchor wordend\"]\n\
	    } else {\n\
		set first [$w index \"anchor wordstart\"]\n\
		set last [$w index \"$index wordend\"]\n\
	    }\n\
	}\n\
	line {\n\
	    if [$w compare $index < anchor] {\n\
		set first [$w index \"$index linestart\"]\n\
		set last [$w index \"anchor lineend + 1c\"]\n\
	    } else {\n\
		set first [$w index \"anchor linestart\"]\n\
		set last [$w index \"$index lineend + 1c\"]\n\
	    }\n\
	}\n\
    }\n\
    $w tag remove sel 0.0 $first\n\
    $w tag add sel $first $last\n\
    $w tag remove sel $last end\n\
}\n\
\n\
proc tk_textBackspace w {\n\
    if [catch {$w delete sel.first sel.last}] {\n\
	$w delete insert-1c\n\
    }\n\
}\n\
\n\
proc tk_textDelRight w {\n\
    if [catch {$w delete sel.first sel.last}] {\n\
	$w delete insert\n\
    }\n\
}\n\
\n\
proc tk_textKillRight w {\n\
    if [catch {$w delete sel.first sel.last}] {\n\
	$w delete insert \"insert lineend\"\n\
    }\n\
}\n\
\n\
proc tk_textKillLine w {\n\
    if [catch {$w delete sel.first sel.last}] {\n\
	$w delete \"insert linestart\" \"insert lineend\"\n\
    }\n\
}\n\
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
bind Text <2> {%W insert insert [select get]; %W yview -pickplace insert}\n\
\n\
bind Text <Any-KeyPress> {\n\
    if {\"%A\" != \"\"} {\n\
	catch {%W delete sel.first sel.last}\n\
	%W insert insert %A\n\
	%W yview -pickplace insert\n\
    }\n\
}\n\
bind Text <Return> {%W insert insert \\n; %W yview -pickplace insert}\n\
bind Text <Delete> {tk_textBackspace %W; %W yview -pickplace insert}\n\
bind Text <Control-a> \\\n\
    {%W mark set insert \"insert linestart\"; %W yview -pickplace insert}\n\
bind Text <Control-b> \\\n\
    {%W mark set insert insert-1c; %W yview -pickplace insert}\n\
bind Text <Control-d> {tk_textDelRight %W; %W yview -pickplace insert}\n\
bind Text <Control-e> \\\n\
    {%W mark set insert \"insert lineend\";  %W yview -pickplace insert}\n\
bind Text <Control-f> \\\n\
    {%W mark set insert insert+1c; %W yview -pickplace insert}\n\
bind Text <Control-h> {tk_textBackspace %W; %W yview -pickplace insert}\n\
bind Text <Control-k> {tk_textKillRight %W; %W yview -pickplace insert}\n\
bind Text <Control-n> \\\n\
    {%W mark set insert insert+1l; %W yview -pickplace insert}\n\
bind Text <Control-p> \\\n\
    {%W mark set insert insert-1l; %W yview -pickplace insert}\n\
bind Text <Control-u> {tk_textKillLine %W; %W yview -pickplace insert}\n\
bind Text <Delete> {tk_textBackspace %W; %W yview -pickplace insert}\n\
bind Text <BackSpace> {tk_textBackspace %W; %W yview -pickplace insert}\n\
\n\
tk_bindForTraversal Text\n\
\n\
# Initialize the elements of tk_priv that require initialization.\n\
\n\
set tk_priv(buttons) 0\n\
set tk_priv(dragging) {}\n\
set tk_priv(focus) {}\n\
set tk_priv(grab) {}\n\
set tk_priv(inMenuButton) {}\n\
set tk_priv(posted) {}\n\
set tk_priv(selectMode) char\n\
set tk_priv(window) {}\n\
";
char NV_Subr[] = "\
#\n\
# nv_subr.tcl - TK utility subroutines used by NV\n\
#\n\
\n\
set source_list \"\"\n\
set copy 0\n\
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
proc findSource { name } {\n\
    set match \"\"\n\
    foreach child [winfo children .sources] {\n\
	if [string match \".sources.source*\" $child] {\n\
	    set sourcename [lindex [$child config -text] 4]\n\
	    if [string match $name $sourcename] {\n\
		set match $child\n\
		break\n\
	    }\n\
	}\n\
    }\n\
 \n\
    if [string length $match] {\n\
	return [string range $match 15 end]\n\
    } else {\n\
	error \"Source not found: $name\"\n\
    }\n\
}\n\
\n\
proc changeSourceColor {source colormode} {\n\
    case $colormode in {\n\
	grey  {set wantcolor 0}\n\
	color {set wantcolor 1}\n\
    }\n\
    .nvsource$source.ctl.colorbuttons.grey config -state normal\n\
    .nvsource$source.ctl.colorbuttons.color config -state normal\n\
    .nvsource$source.ctl.colorbuttons.$colormode config -state disabled\n\
    .nvsource$source.video color $wantcolor\n\
}\n\
\n\
proc changeSourceSize {source size} {\n\
    case $size in {\n\
	half    {set scaler  1}\n\
	default {set scaler  0}\n\
	double  {set scaler -1}\n\
    }\n\
    .nvsource$source.ctl.sizebuttons.half config -state normal\n\
    .nvsource$source.ctl.sizebuttons.normal config -state normal\n\
    .nvsource$source.ctl.sizebuttons.double config -state normal\n\
    .nvsource$source.ctl.sizebuttons.$size config -state disabled\n\
    .nvsource$source.video scale $scaler\n\
}\n\
\n\
proc doSourceCapture {source} {\n\
    global copy\n\
    set name [wm title .nvsource$source]\n\
    toplevel .nvcopy$copy\n\
    wm title .nvcopy$copy \"$name (captured)\"\n\
    wm maxsize .nvcopy$copy [winfo reqwidth .nvsource$source.video] 2000\n\
    wm minsize .nvcopy$copy [winfo reqwidth .nvsource$source.video] \\\n\
	[winfo reqheight .nvsource$source.video]\n\
    wm protocol .nvcopy$copy WM_TAKE_FOCUS \"focus .nvcopy$copy.text\"\n\
    pack append .nvcopy$copy \\\n\
	[frame .nvcopy$copy.video] {top fill} \\\n\
	[scrollbar .nvcopy$copy.sb -relief raised \\\n\
	    -command \".nvcopy$copy.text yview\"] {left filly} \\\n\
	[text .nvcopy$copy.text -width 1 -height 3 -relief raised \\\n\
	    -yscrollcommand \".nvcopy$copy.sb set\"] {left expand fill}\n\
    .nvsource$source.video copy .nvcopy$copy.video\n\
    bind .nvcopy$copy.text <Control-c> \"destroy .nvcopy$copy\"\n\
    bind .nvcopy$copy.text <Escape> \"destroy .nvcopy$copy\"\n\
    set copy [expr $copy+1]\n\
}\n\
\n\
proc setSourceName {source name} {\n\
    wm title .nvsource$source $name\n\
    if [regexp \"(.*) \\[<(\\].*\\[>)\\]\" $name fullname shortname] {\n\
	set name $shortname\n\
    }\n\
    .sources.canvas itemconfig title$source -text $name\n\
    while 1 {\n\
	set bbox [.sources.canvas bbox title$source]\n\
	set w [expr [lindex $bbox 2]-[lindex $bbox 0]]\n\
	if {($w > 100) && ([string length $name] > 1)} {\n\
	    set name [string range $name 0 [expr [string length $name]-2]]\n\
	    .sources.canvas itemconfig title$source -text ${name}...\n\
	} else {\n\
	    break\n\
	}\n\
    }\n\
}\n\
\n\
proc setFPSandBPS {source fps bps} {\n\
    if {$fps == \"\"} {\n\
	.nvsource$source.ctl.frames.fps config -text \"\"\n\
	.nvsource$source.ctl.bytes.bps config -text \"\"\n\
    } else {\n\
	.nvsource$source.ctl.frames.fps config -text \"$fps  fps\"\n\
	.nvsource$source.ctl.bytes.bps config -text \"$bps kbps\"\n\
    }\n\
}\n\
";
char NV_UIProcs[] = "\
#\n\
# nv_procs.tcl - TK UI procedures used by NV\n\
#\n\
\n\
proc showRecvControls {source} {\n\
    pack append .nvsource$source .nvsource$source.ctl {top expand fill}\n\
    bind .nvsource$source <1> \"hideRecvControls $source\"\n\
    bind .nvsource$source.video <1> \"hideRecvControls $source\"\n\
    bind .nvsource$source <2> \"hideRecvControls $source\"\n\
    bind .nvsource$source.video <2> \"hideRecvControls $source\"\n\
    bind .nvsource$source <3> \"hideRecvControls $source\"\n\
    bind .nvsource$source.video <3> \"hideRecvControls $source\"\n\
}\n\
 \n\
proc hideRecvControls {source} {\n\
    pack unpack .nvsource$source.ctl\n\
    bind .nvsource$source <1> \"showRecvControls $source\"\n\
    bind .nvsource$source.video <1> \"showRecvControls $source\"\n\
    bind .nvsource$source <2> \"showRecvControls $source\"\n\
    bind .nvsource$source.video <2> \"showRecvControls $source\"\n\
    bind .nvsource$source <3> \"showRecvControls $source\"\n\
    bind .nvsource$source.video <3> \"showRecvControls $source\"\n\
}\n\
\n\
proc enterSource {id} {\n\
    global activeBgColor screenDepth\n\
    .sources.canvas itemconfig $id -fill $activeBgColor\n\
    if {$screenDepth == 1} {.sources.canvas itemconfig [expr $id+2] -fill white}\n\
}\n\
\n\
proc leaveSource {id} {\n\
    global BgColor screenDepth\n\
    .sources.canvas itemconfig $id -fill $BgColor\n\
    if {$screenDepth == 1} {.sources.canvas itemconfig [expr $id+2] -fill black}\n\
}\n\
\n\
proc packSources {} {\n\
    global source_list\n\
    set width  [winfo width .sources.canvas]\n\
    set height [winfo height .sources.canvas]\n\
    if {$width == 0} {set width 1}\n\
    set x 0\n\
    set y 0\n\
    foreach source $source_list {\n\
	.sources.canvas coords rect$source $x $y [expr $x+100] [expr $y+95]\n\
	.sources.canvas coords video$source [expr $x+50] [expr $y+38]\n\
	.sources.canvas coords title$source [expr $x+50] [expr $y+76]\n\
	incr x 100\n\
	if {[expr $x+100] >= $width} {\n\
	    set x 0\n\
	    incr y 95\n\
	}\n\
    }\n\
    if {$x != 0} {incr y 95}\n\
    .sources.canvas config -scrollregion \"0 0 $width $y\"\n\
}\n\
\n\
proc showSource {source} {\n\
    global nvgeom$source\n\
    receiveVideo $source 1\n\
    wm geometry .nvsource$source [set nvgeom$source]\n\
    wm deiconify .nvsource$source\n\
}\n\
\n\
proc hideSource {source} {\n\
    global nvgeom$source\n\
    receiveVideo $source 0\n\
    regexp \"\\[+-\\].*\" [wm geometry .nvsource$source] nvgeom$source\n\
    wm withdraw .nvsource$source\n\
}\n\
\n\
proc toggleSource {source} {\n\
    if [winfo ismapped .nvsource$source] {\n\
	hideSource $source\n\
    } else {\n\
	showSource $source\n\
    }\n\
}\n\
\n\
proc addSource {source address} {\n\
    global source_list BgColor\n\
\n\
    .sources.video$source scale 2\n\
\n\
    set id [.sources.canvas create rect 0 0 0 0 -fill $BgColor -outline \"\" \\\n\
	-tags rect$source]\n\
    .sources.canvas create window 0 0 -anchor center \\\n\
	-window .sources.video$source -tags video$source\n\
    .sources.canvas create text 0 0 -anchor n -text $address -tags title$source\n\
\n\
    lappend source_list $source\n\
    packSources\n\
\n\
    bind .sources.video$source <Any-Enter> \"enterSource $id\"\n\
    bind .sources.video$source <Any-Leave> \"leaveSource $id\"\n\
    bind .sources.video$source <1> \"toggleSource $source\"\n\
    bind .sources.video$source <2> \"toggleSource $source\"\n\
    bind .sources.video$source <3> \"toggleSource $source\"\n\
\n\
    frame .nvsource$source.ctl -borderwidth 2 -relief raised\n\
\n\
    frame .nvsource$source.ctl.sizebuttons\n\
    pack append .nvsource$source.ctl.sizebuttons \\\n\
	[button .nvsource$source.ctl.sizebuttons.half   -text \"Half\" \\\n\
	    -command \"changeSourceSize $source half\"]   {left expand fill} \\\n\
	[button .nvsource$source.ctl.sizebuttons.normal -text \"Normal\" \\\n\
	    -command \"changeSourceSize $source normal\"] {left expand fill} \\\n\
	[button .nvsource$source.ctl.sizebuttons.double -text \"Double\" \\\n\
	    -command \"changeSourceSize $source double\"] {left expand fill}\n\
\n\
    frame .nvsource$source.ctl.colorbuttons\n\
    pack append .nvsource$source.ctl.colorbuttons \\\n\
	[button .nvsource$source.ctl.colorbuttons.grey  -text \"Greyscale\" \\\n\
	    -command \"changeSourceColor $source grey\"]  {left expand fill} \\\n\
	[button .nvsource$source.ctl.colorbuttons.color -text \"Color\" \\\n\
	    -command \"changeSourceColor $source color\"] {left expand fill}\n\
\n\
    scale .nvsource$source.ctl.brightness -command \"changeBrightness $source\" \\\n\
	-label \"Brightness\" -from 0 -to 100 -orient horizontal\n\
\n\
    scale .nvsource$source.ctl.contrast -command \"changeContrast $source\" \\\n\
	-label \"Contrast\" -from 0 -to 100 -orient horizontal\n\
\n\
    frame .nvsource$source.ctl.address -relief flat\n\
    pack append .nvsource$source.ctl.address \\\n\
	[label .nvsource$source.ctl.address.label -anchor w -width 10 \\\n\
	    -text \"Address:\"] {left filly} \\\n\
	[label .nvsource$source.ctl.address.value -anchor e -width 12 \\\n\
	    -text $address] {left filly} \\\n\
	[frame .nvsource$source.ctl.address.fill] {left expand fill}\n\
\n\
    frame .nvsource$source.ctl.frames -relief flat\n\
    pack append .nvsource$source.ctl.frames \\\n\
	[label .nvsource$source.ctl.frames.label -anchor w -width 10 \\\n\
	    -text \"Frame rate:\"] {left filly} \\\n\
	[label .nvsource$source.ctl.frames.fps -anchor e -width 12] \\\n\
	    {left filly} \\\n\
	[frame .nvsource$source.ctl.frames.fill] {left expand fill}\n\
\n\
    frame .nvsource$source.ctl.bytes -relief flat\n\
    pack append .nvsource$source.ctl.bytes \\\n\
	[label .nvsource$source.ctl.bytes.label -anchor w -width 10 \\\n\
	    -text \"Bandwidth:\"] {left filly} \\\n\
	[label .nvsource$source.ctl.bytes.bps -anchor e -width 12] \\\n\
	    {left filly} \\\n\
	[frame .nvsource$source.ctl.bytes.fill] {left expand fill}\n\
\n\
    button .nvsource$source.ctl.capture -text \"Capture\" \\\n\
	-command \"doSourceCapture $source\"\n\
\n\
    wm title .nvsource$source $address\n\
\n\
    global nvgeom$source\n\
    set nvgeom$source \"\"\n\
    wm withdraw .nvsource$source\n\
\n\
    pack append .nvsource$source.ctl \\\n\
	.nvsource$source.ctl.brightness {top fillx pady 4 frame s} \\\n\
	.nvsource$source.ctl.contrast {top fill} \\\n\
	.nvsource$source.ctl.address {top fill} \\\n\
	.nvsource$source.ctl.frames {top fill} \\\n\
	.nvsource$source.ctl.bytes {top fill} \\\n\
	.nvsource$source.ctl.sizebuttons {top fill} \\\n\
	.nvsource$source.ctl.colorbuttons {top fill} \\\n\
	.nvsource$source.ctl.capture {top fill}\n\
\n\
    pack append .nvsource$source .nvsource$source.video {top frame center}\n\
\n\
    wm protocol .nvsource$source WM_DELETE_WINDOW \"hideSource $source\"\n\
    wm protocol .nvsource$source WM_TAKE_FOCUS \"focus .nvsource$source\"\n\
\n\
    bind .nvsource$source <Control-c> \"hideSource $source\"\n\
    bind .nvsource$source <Escape> \"hideSource $source\"\n\
    bind .nvsource$source <q> \"hideSource $source\"\n\
\n\
    bind .nvsource$source <h> \"changeSourceSize $source half\"\n\
    bind .nvsource$source <n> \"changeSourceSize $source normal\"\n\
    bind .nvsource$source <d> \"changeSourceSize $source double\"\n\
    bind .nvsource$source <c> \"doSourceCapture $source\"\n\
\n\
    bind .nvsource$source <Key-1> \"changeSourceSize $source half\"\n\
    bind .nvsource$source <Key-2> \"changeSourceSize $source normal\"\n\
    bind .nvsource$source <Key-3> \"changeSourceSize $source double\"\n\
\n\
    bind .nvsource$source <1> \"showRecvControls $source\"\n\
    bind .nvsource$source.video <1> \"showRecvControls $source\"\n\
    bind .nvsource$source <2> \"showRecvControls $source\"\n\
    bind .nvsource$source.video <2> \"showRecvControls $source\"\n\
    bind .nvsource$source <3> \"showRecvControls $source\"\n\
    bind .nvsource$source.video <3> \"showRecvControls $source\"\n\
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
    changeSourceSize  $source [option get . recvSize Nv]\n\
    changeSourceColor $source [option get . recvColor Nv]\n\
}\n\
\n\
proc deleteSource {source} {\n\
    global source_list\n\
\n\
    receiveVideo $source 0\n\
    .sources.canvas delete title$source\n\
    .sources.canvas delete video$source\n\
    .sources.canvas delete rect$source\n\
    destroy .nvsource$source\n\
    destroy .sources.video$source\n\
\n\
    set ind [lsearch $source_list $source]\n\
    set source_list [lreplace $source_list $ind $ind]\n\
    packSources\n\
}\n\
";
char NV_API[] = "\
#\n\
# nv_api.tcl - TK procedures which define the remote NV API\n\
#\n\
\n\
set recvgeom \"\"\n\
\n\
proc findSource { name } {\n\
    set match \"\"\n\
    foreach child [winfo children .sources] {\n\
	if [string match \".sources.source*\" $child] {\n\
	    set sourcename [lindex [$child config -text] 4]\n\
	    if [string match $name $sourcename] {\n\
		set match $child\n\
		break\n\
	    }\n\
	}\n\
    }\n\
 \n\
    if [string length $match] {\n\
	return [string range $match 15 end]\n\
    } else {\n\
	error \"Source not found: $name\"\n\
    }\n\
}\n\
\n\
proc openSource { name } {\n\
    set source [findSource $name]\n\
    global source$source\n\
    set source$source 1\n\
    receiveVideo $source 1\n\
    wm deiconify .nvsource$source\n\
}\n\
\n\
proc closeSource { name } {\n\
    set source [findSource $name]\n\
    global source$source\n\
    set source$source 0\n\
    receiveVideo $source 0\n\
    wm withdraw .nvsource$source\n\
}\n\
\n\
proc setSourceBrightness { name brightness } {\n\
    set source [findSource $name]\n\
    .nvsource$source.ctl.brightness set $brightness\n\
}\n\
\n\
proc setSourceContrast { name contrast } {\n\
    set source [findSource $name]\n\
    .nvsource$source.ctl.contrast set $contrast\n\
}\n\
\n\
proc setSourceSize { name size } {\n\
    set source [findSource $name]\n\
    changeSourceSize $source $size\n\
}\n\
\n\
proc captureSource { name } {\n\
    set source [findSource $name]\n\
    doSourceCapture $source\n\
}\n\
\n\
proc setAddress { addr } {\n\
    .confControls.netinfo.e1 delete 0 end\n\
    .confControls.netinfo.e1 insert 0 $addr\n\
    changeAddress $addr\n\
}\n\
\n\
proc setPort { port } {\n\
    .confControls.netinfo.e2 delete 0 end\n\
    .confControls.netinfo.e2 insert 0 $port\n\
    changePort $port\n\
}\n\
\n\
proc setTTL { ttl } {\n\
    .confControls.netinfo.e3 delete 0 end\n\
    .confControls.netinfo.e3 insert 0 $ttl\n\
    changeTTL $ttl\n\
}\n\
\n\
proc setName { name } {\n\
    .confControls.name.e1 delete 0 end\n\
    .confControls.name.e1 insert 0 $name\n\
    changeName $name\n\
}\n\
\n\
proc setMaxBandwidth { bw } {\n\
    .sendControls.bandwidth set $bw\n\
}\n\
\n\
proc startSending { } {\n\
    sendVideo 1\n\
    .sendControls.buttons.enable config -text \"Stop sending\"\n\
}\n\
\n\
proc stopSending { } {\n\
    sendVideo 0\n\
    .sendControls.buttons.enable config -text \"Start sending\"\n\
}\n\
\n\
proc showReceivers { } {\n\
    global recvgeom\n\
    wm geometry .receivers $recvgeom\n\
    wm deiconify .receivers\n\
    .sendControls.buttons.receivers config -text \"Hide receivers\"\n\
}\n\
\n\
proc hideReceivers { } {\n\
    global recvgeom\n\
    wm withdraw .receivers\n\
    regexp \"\\[+-\\].*\" [wm geometry .receivers] recvgeom\n\
    .sendControls.buttons.receivers config -text \"Show receivers\"\n\
}\n\
";
char NV_Init[] = "\
#\n\
# nv_init.tcl - TK interface definition for NV\n\
#\n\
\n\
set version \"3.2\"\n\
set copy 0\n\
\n\
set screenDepth [winfo screendepth .]\n\
case $screenDepth in {\n\
    1		{\n\
		  set activeBgColor	black\n\
		  set activeFgColor	white\n\
		  set disabledFgColor	\"\"\n\
		  set selectBgColor	black\n\
		  set BgColor		white\n\
		  set FgColor		black\n\
		}\n\
    default	{\n\
		  set activeBgColor	#dfdfdf\n\
		  set activeFgColor	black\n\
		  set disabledFgColor	#777777\n\
		  set selectBgColor	#bfdfff\n\
		  set BgColor		#bfbfbf\n\
		  set FgColor		black\n\
		}\n\
}\n\
\n\
set nvBrightness			50\n\
set nvContrast				50\n\
set nvMaxBandwidth			128\n\
set nvRecvColor				color\n\
set nvRecvSize				normal\n\
set nvXmitColor				color\n\
set nvTTL				16\n\
\n\
option add Nv.brightness		$nvBrightness		startupFile\n\
option add Nv.contrast			$nvContrast		startupFile\n\
option add Nv.maxBandwidth		$nvMaxBandwidth		startupFile\n\
option add Nv.recvColor			$nvRecvColor		startupFile\n\
option add Nv.recvSize			$nvRecvSize		startupFile\n\
option add Nv.xmitColor			$nvXmitColor		startupFile\n\
option add Nv.title			\"nv v$version\"		startupFile\n\
option add Nv.ttl			$nvTTL			startupFile\n\
\n\
option add *activeBackground		$activeBgColor		startupFile\n\
option add *activeForeground		$activeFgColor		startupFile\n\
option add *CheckButton.selector	$FgColor		startupFile\n\
option add *background			$BgColor		startupFile\n\
option add *foreground			$FgColor		startupFile\n\
option add *selectBackground		$selectBgColor		startupFile\n\
option add *Scale.activeForeground	$activeBgColor		startupFile\n\
option add *Scale.sliderForeground	$BgColor		startupFile\n\
option add *Scrollbar.foreground	$BgColor		startupFile\n\
option add *Scrollbar.activeForeground	$activeBgColor		startupFile\n\
option add *Button.disabledForeground	$disabledFgColor	startupFile\n\
\n\
proc pressSend {} {\n\
    set state [lindex [ .sendControls.buttons.enable config -text] 4]\n\
    if {$state == \"Start sending\"} {\n\
	startSending\n\
    } else {\n\
	stopSending\n\
    }\n\
}\n\
 \n\
proc pressReceivers {} {\n\
    set state [lindex [ .sendControls.buttons.receivers config -text] 4]\n\
    if {$state == \"Show receivers\"} {\n\
	showReceivers\n\
    } else {\n\
	hideReceivers\n\
    }\n\
}\n\
\n\
loadAppDefaults {Nv NV} userDefault\n\
if {[envVal XENVIRONMENT] != {}} {\n\
    option readfile [envVal XENVIRONMENT] userDefault\n\
}\n\
\n\
frame .sources\n\
pack append .sources \\\n\
    [scrollbar .sources.sb -relief raised -command \".sources.canvas yview\"] \\\n\
	{left filly} \\\n\
    [canvas .sources.canvas -height 95 -width 300 -relief raised \\\n\
	-scrollincrement 95 -scrollregion { 0 0 100 95 } \\\n\
	-yscroll \".sources.sb set\"] {left expand fill}\n\
\n\
frame .confControls -borderwidth 2 -relief raised\n\
\n\
label .confControls.title -text \"Conference info\"\n\
\n\
frame .confControls.netinfo -relief flat\n\
pack append .confControls.netinfo \\\n\
    [label .confControls.netinfo.l1 -text \"Address:\"] {left filly} \\\n\
    [entry .confControls.netinfo.e1 -relief flat -width 14] {left expand fill} \\\n\
    [label .confControls.netinfo.l2 -text \"Port:\"] {left filly} \\\n\
    [entry .confControls.netinfo.e2 -relief flat -width 4] {left filly} \\\n\
    [label .confControls.netinfo.l3 -text \"TTL:\"] {left filly} \\\n\
    [entry .confControls.netinfo.e3 -relief flat -width 1] {left filly}\n\
\n\
frame .confControls.name -relief flat\n\
pack append .confControls.name \\\n\
    [label .confControls.name.l1 -text \"Name:\"] {left filly} \\\n\
    [entry .confControls.name.e1 -relief flat -width 1] {left expand fill}\n\
\n\
pack append .confControls \\\n\
    .confControls.title {top fillx} \\\n\
    .confControls.netinfo {top fill} \\\n\
    .confControls.name {top fill}\n\
\n\
frame .sendControls -borderwidth 2 -relief raised\n\
\n\
label  .sendControls.title -text \"Video transmit options\"\n\
\n\
scale  .sendControls.bandwidth -command \"changeMaxBandwidth\" \\\n\
    -label \"Max Bandwidth (kbps)\" -from 1 -to 1024 -orient horizontal\n\
\n\
frame .sendControls.color\n\
pack append .sendControls.color \\\n\
    [button .sendControls.color.grey -text \"Send greyscale\" -width 20 \\\n\
	-command \"sendColor 0\"] {left expand fill} \\\n\
    [button .sendControls.color.color -text \"Send color\" -width 20 \\\n\
	-state disabled -command \"sendColor 1\"] {left expand fill}\n\
\n\
frame .sendControls.buttons\n\
pack append .sendControls.buttons \\\n\
    [button .sendControls.buttons.enable -text \"Start sending\" -width 20 \\\n\
	-command \"pressSend\"] {left expand fill} \\\n\
    [button .sendControls.buttons.receivers -text \"Show receivers\" -width 20 \\\n\
	-command \"pressReceivers\"] {left expand fill}\n\
\n\
bind .sources.canvas <Configure> \"packSources\"\n\
\n\
bind .confControls.netinfo.e1 <Return> {\n\
    focus .\n\
    changeAddress [.confControls.netinfo.e1 get]\n\
    .confControls.netinfo.e1 select clear\n\
}\n\
\n\
bind .confControls.netinfo.e2 <Return> {\n\
    focus .\n\
    changePort [.confControls.netinfo.e2 get]\n\
    .confControls.netinfo.e2 select clear\n\
}\n\
\n\
bind .confControls.netinfo.e3 <Return> {\n\
    focus .\n\
    changeTTL [.confControls.netinfo.e3 get]\n\
    .confControls.netinfo.e3 select clear\n\
}\n\
\n\
bind .confControls.name.e1 <Return> {\n\
    focus .\n\
    changeName [.confControls.name.e1 get]\n\
    .confControls.name.e1 select clear\n\
}\n\
\n\
if [catch {.sendControls.bandwidth set [option get . maxBandwidth Nv]}] {\n\
    .sendControls.maxBandwidth set $nvMaxBandwidth\n\
}\n\
\n\
.confControls.netinfo.e1 insert 0 [option get . address Nv]\n\
changeAddress [.confControls.netinfo.e1 get]\n\
.confControls.netinfo.e2 insert 0 [option get . port Nv]\n\
changePort [.confControls.netinfo.e2 get]\n\
.confControls.netinfo.e3 insert 0 [option get . ttl Nv]\n\
changeTTL [.confControls.netinfo.e3 get]\n\
.confControls.name.e1 insert 0 [option get . name Nv]\n\
changeName [.confControls.name.e1 get]\n\
sendVideo 0\n\
\n\
pack append .sendControls \\\n\
    .sendControls.title {top expand fillx} \\\n\
    .sendControls.bandwidth {top expand fillx} \\\n\
    .sendControls.color {top fillx} \\\n\
    .sendControls.buttons {top fillx}\n\
\n\
toplevel .receivers\n\
pack append .receivers \\\n\
    [scrollbar .receivers.scroll -relief raised \\\n\
	-command \".receivers.list yview\"] { left filly } \\\n\
    [listbox .receivers.list -geometry 60x10 -yscroll \".receivers.scroll set\" \\\n\
	-relief raised] { left expand fill }\n\
wm minsize .receivers 0 0\n\
wm title .receivers \"nv receivers\"\n\
wm withdraw .receivers\n\
\n\
pack append . .sources {top expand fill} .confControls {top fill}\n\
\n\
global send_allowed\n\
if $send_allowed { pack append . .sendControls { top fillx } }\n\
\n\
update\n\
set reqw [winfo reqwidth .]\n\
set reqh [winfo reqheight .]\n\
\n\
wm minsize . $reqw $reqh\n\
wm geometry . ${reqw}x$reqh\n\
wm title . [option get . title Nv]\n\
\n\
wm protocol . WM_TAKE_FOCUS {focus .}\n\
wm protocol .receivers WM_TAKE_FOCUS {focus .receivers}\n\
\n\
bind . <Control-c> {shutdown}\n\
bind . <Escape> {shutdown}\n\
bind . <q> {shutdown}\n\
\n\
bind .receivers <Control-c> {pressReceivers}\n\
bind .receivers <Escape> {pressReceivers}\n\
bind .receivers <q> {pressReceivers}\n\
wm protocol .receivers WM_DELETE_WINDOW {pressReceivers}\n\
";
