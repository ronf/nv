#
# General TK initialization code
#

proc tk_butEnter w {
    global tk_priv tk_strictMotif
    if {[lindex [$w config -state] 4] != "disabled"} {
	if {!$tk_strictMotif} {
	    $w config -state active
	}
	set tk_priv(window) $w
    }
}

proc tk_butLeave w {
    global tk_priv tk_strictMotif
    if {[lindex [$w config -state] 4] != "disabled"} {
	if {!$tk_strictMotif} {
	    $w config -state normal
	}
    }
    set tk_priv(window) ""
}

proc tk_butDown w {
    global tk_priv
    set tk_priv(relief) [lindex [$w config -relief] 4]
    if {[lindex [$w config -state] 4] != "disabled"} {
	$w config -relief sunken
    }
}

proc tk_butUp w {
    global tk_priv
    $w config -relief $tk_priv(relief)
    if {($w == $tk_priv(window))
	    && ([lindex [$w config -state] 4] != "disabled")} {
	uplevel #0 [list $w invoke]
    }
}

proc tk_menus {w args} {
    global tk_priv

    if {$args == ""} {
	if [catch {set result [set tk_priv(menusFor$w)]}] {
	    return ""
	}
	return $result
    }

    if {$args == "{}"} {
	catch {unset tk_priv(menusFor$w)}
	return ""
    }

    set tk_priv(menusFor$w) $args
}

proc tk_bindForTraversal args {
    foreach w $args {
	bind $w <Alt-KeyPress> {tk_traverseToMenu %W %A}
	bind $w <F10> {tk_firstMenu %W}
    }
}

proc tk_mbPost {w} {
    global tk_priv tk_strictMotif
    if {[lindex [$w config -state] 4] == "disabled"} {
	return
    }
    set cur $tk_priv(posted)
    if {$cur == $w} {
	return
    }
    if {$cur != ""} tk_mbUnpost
    set tk_priv(relief) [lindex [$w config -relief] 4]
    $w config -relief raised
    set tk_priv(cursor) [lindex [$w config -cursor] 4]
    $w config -cursor arrow
    $w post
    grab -global $w
    set tk_priv(posted) $w
    if {$tk_priv(focus) == ""} {
	set tk_priv(focus) [focus]
    }
    set menu [lindex [$w config -menu] 4]
    set tk_priv(activeBg) [lindex [$menu config -activebackground] 4]
    set tk_priv(activeFg) [lindex [$menu config -activeforeground] 4]
    if $tk_strictMotif {
	$menu config -activebackground [lindex [$menu config -background] 4]
	$menu config -activeforeground [lindex [$menu config -foreground] 4]
    }
    focus $menu
}

proc tk_mbUnpost {} {
    global tk_priv
    if {$tk_priv(posted) != ""} {
	$tk_priv(posted) config -relief $tk_priv(relief)
	$tk_priv(posted) config -cursor $tk_priv(cursor)
	$tk_priv(posted) config -activebackground $tk_priv(activeBg)
	$tk_priv(posted) config -activeforeground $tk_priv(activeFg)
	$tk_priv(posted) unpost
	grab none
	focus $tk_priv(focus)
	set tk_priv(focus) ""
	set menu [lindex [$tk_priv(posted) config -menu] 4]
	$menu config -activebackground $tk_priv(activeBg)
	$menu config -activeforeground $tk_priv(activeFg)
	set tk_priv(posted) {}
    }
}

proc tk_traverseToMenu {w char} {
    global tk_priv
    if {$char == ""} {
	return
    }
    set char [string tolower $char]

    foreach mb [tk_getMenuButtons $w] {
	if {[winfo class $mb] == "Menubutton"} {
	    set char2 [string index [lindex [$mb config -text] 4] \
		    [lindex [$mb config -underline] 4]]
	    if {[string compare $char [string tolower $char2]] == 0} {
		tk_mbPost $mb
		[lindex [$mb config -menu] 4] activate 0
		return
	    }
	}
    }
}

proc tk_traverseWithinMenu {w char} {
    if {$char == ""} {
	return
    }
    set char [string tolower $char]
    set last [$w index last]
    for {set i 0} {$i <= $last} {incr i} {
	if [catch {set char2 [string index \
		[lindex [$w entryconfig $i -label] 4] \
		[lindex [$w entryconfig $i -underline] 4]]}] {
	    continue
	}
	if {[string compare $char [string tolower $char2]] == 0} {
	    tk_mbUnpost
	    $w invoke $i
	    return
	}
    }
}

proc tk_getMenuButtons w {
    global tk_priv
    set top [winfo toplevel $w]
    if [catch {set buttons [set tk_priv(menusFor$top)]}] {
	return ""
    }
    return $buttons
}

proc tk_nextMenu count {
    global tk_priv
    if {$tk_priv(posted) == ""} {
	return
    }
    set buttons [tk_getMenuButtons $tk_priv(posted)]
    set length [llength $buttons]
    for {set i 0} 1 {incr i} {
	if {$i >= $length} {
	    return
	}
	if {[lindex $buttons $i] == $tk_priv(posted)} {
	    break
	}
    }
    incr i $count
    while 1 {
	while {$i < 0} {
	    incr i $length
	}
	while {$i >= $length} {
	    incr i -$length
	}
	set mb [lindex $buttons $i]
	if {[lindex [$mb configure -state] 4] != "disabled"} {
	    break
	}
	incr i $count
    }
    tk_mbUnpost
    tk_mbPost $mb
    [lindex [$mb config -menu] 4] activate 0
}

proc tk_nextMenuEntry count {
    global tk_priv
    if {$tk_priv(posted) == ""} {
	return
    }
    set menu [lindex [$tk_priv(posted) config -menu] 4]
    set length [expr [$menu index last]+1]
    set i [$menu index active]
    if {$i == "none"} {
	set i 0
    } else {
	incr i $count
    }
    while 1 {
	while {$i < 0} {
	    incr i $length
	}
	while {$i >= $length} {
	    incr i -$length
	}
	if {[catch {$menu entryconfigure $i -state} state] == 0} {
	    if {[lindex $state 4] != "disabled"} {
		break
	    }
	}
	incr i $count
    }
    $menu activate $i
}

proc tk_invokeMenu {menu} {
    set i [$menu index active]
    if {$i != "none"} {
	tk_mbUnpost
	update idletasks
	$menu invoke $i
    }
}

proc tk_firstMenu w {
    set mb [lindex [tk_getMenuButtons $w] 0]
    if {$mb != ""} {
	tk_mbPost $mb
	[lindex [$mb config -menu] 4] activate 0
    }
}

proc tk_mbButtonDown w {
    global tk_priv
    if {[lindex [$w config -state] 4] == "disabled"} {
	return
    } 
    if {$tk_priv(inMenuButton) == $w} {
	tk_mbPost $w
    }
    set menu [lindex [$tk_priv(posted) config -menu] 4]
    if {$tk_priv(window) != $menu} {
	$menu activate none
    }
}

proc tk_entryForwardChar w {
    set x [$w index cursor]
    $w cursor [incr x +1]
}

proc tk_entryBackChar w {
    set x [$w index cursor]
    $w cursor [incr x -1]
}

proc tk_entryBackspace w {
    if [catch {$w delete sel.first sel.last}] {
	set x [expr {[$w index cursor] - 1}]
	if {$x != -1} {$w delete $x}
    }
}

proc tk_entryDelRight w {
    if [catch {$w delete sel.first sel.last}] {
	set x [$w index cursor]
	if {$x != -1} {$w delete $x}
    }
}

proc tk_entryKillRight w {
    if [catch {$w delete sel.first sel.last}] {
	set x [$w index cursor]
	if {$x != -1} {$w delete $x end}
    }
}

proc tk_entrySeeCaret w {
    set c [$w index cursor]
    set left [$w index @0]
    if {$left > $c} {
	$w view $c
	return
    }
    while {[$w index @[expr [winfo width $w]-5]] < $c} {
	set left [expr $left+1]
	$w view $left
    }
}

set tk_strictMotif 0

bind Button <Any-Enter> {tk_butEnter %W}
bind Button <Any-Leave> {tk_butLeave %W}
bind Button <1> {tk_butDown %W}
bind Button <ButtonRelease-1> {tk_butUp %W}

bind CheckButton <Any-Enter> {tk_butEnter %W}
bind CheckButton <Any-Leave> {tk_butLeave %W}
bind CheckButton <1> {tk_butDown %W}
bind CheckButton <ButtonRelease-1> {tk_butUp %W}

bind RadioButton <Any-Enter> {tk_butEnter %W}
bind RadioButton <Any-Leave> {tk_butLeave %W}
bind RadioButton <1> {tk_butDown %W}
bind RadioButton <ButtonRelease-1> {tk_butUp %W}

bind Entry <1> {
    %W cursor @%x
    %W select from @%x
    if {[lindex [%W config -state] 4] == "normal"} {focus %W}
}
bind Entry <B1-Motion> {%W select to @%x}
bind Entry <Double-1> {%W select from 0; %W select to end}
bind Entry <Triple-1> {%W select from 0; %W select to end}
bind Entry <2> {%W insert cursor [selection get]; tk_entrySeeCaret %W}
bind Entry <3> {%W select adjust @%x}
bind Entry <B3-Motion> {%W select to @%x}

bind Entry <Any-KeyPress> {
    if {"%A" != ""} {
	catch {%W delete sel.first sel.last}
	%W insert cursor %A
	tk_entrySeeCaret %W
    }
}

bind Entry <Control-a> {%W cursor -1; tk_entrySeeCaret %W}
bind Entry <Control-b> {tk_entryBackChar %W; tk_entrySeeCaret %W}
bind Entry <Control-d> {tk_entryDelRight %W; tk_entrySeeCaret %W}
bind Entry <Control-e> {%W cursor end; tk_entrySeeCaret %W}
bind Entry <Control-f> {tk_entryForwardChar %W; tk_entrySeeCaret %W}
bind Entry <Control-h> {tk_entryBackspace %W; tk_entrySeeCaret %W}
bind Entry <Control-k> {tk_entryKillRight %W; tk_entrySeeCaret %W}
bind Entry <Control-u> {%W delete 0 end}
bind Entry <Delete> {tk_entryBackspace %W; tk_entrySeeCaret %W}
bind Entry <BackSpace> {tk_entryBackspace %W; tk_entrySeeCaret %W}

tk_bindForTraversal Entry

bind Scrollbar <Any-Enter> {
    if $tk_strictMotif {
	set tk_priv(activeFg) [lindex [%W config -activeforeground] 4]
	%W config -activeforeground [lindex [%W config -foreground] 4]
    }
}
bind Scrollbar <Any-Leave> {
    if {$tk_strictMotif && ($tk_priv(buttons) == 0)} {
	%W config -activeforeground $tk_priv(activeFg)
    }
}
bind Scrollbar <Any-ButtonPress> {incr tk_priv(buttons)}
bind Scrollbar <Any-ButtonRelease> {incr tk_priv(buttons) -1}

bind Scale <Any-Enter> {
    if $tk_strictMotif {
	set tk_priv(activeFg) [lindex [%W config -activeforeground] 4]
	%W config -activeforeground [lindex [%W config -sliderforeground] 4]
    }
}
bind Scale <Any-Leave> {
    if {$tk_strictMotif && ($tk_priv(buttons) == 0)} {
	%W config -activeforeground $tk_priv(activeFg)
    }
}
bind Scale <Any-ButtonPress> {incr tk_priv(buttons)}
bind Scale <Any-ButtonRelease> {incr tk_priv(buttons) -1}

bind Menubutton <Enter> {
    set tk_priv(inMenuButton) %W
    if {[lindex [%W config -state] 4] != "disabled"} {
	if {!$tk_strictMotif} {
	    %W config -state active
	}
    }
}
bind Menubutton <Any-Leave> {
    set tk_priv(inMenuButton) {}
    if {[lindex [%W config -state] 4] != "disabled"} {
	if {!$tk_strictMotif} {
	    %W config -state normal
	}
    }
}
bind Menubutton <1> {tk_mbButtonDown %W}
bind Menubutton <Any-ButtonRelease-1> {
    if {($tk_priv(inMenuButton) != "") && ($tk_priv(posted) != "")} {
	[lindex [$tk_priv(posted) config -menu] 4] activate 0
    } else {
	tk_mbUnpost
    }
}

bind Menubutton <B1-Enter> {
    set tk_priv(inMenuButton) %W
    if {([lindex [%W config -state] 4] != "disabled")
	    && ("%m" != "NotifyGrab") && ("%m" != "NotifyUngrab")} {
	if {!$tk_strictMotif} {
	    %W config -state active
	}
	tk_mbPost %W
    }
}
bind Menubutton <2> {
    if {($tk_priv(posted) == "")
	    && ([lindex [%W config -state] 4] != "disabled")} {
	set tk_priv(dragging) %W
	[lindex [$tk_priv(dragging) config -menu] 4] post %X %Y
    }
}
bind Menubutton <B2-Motion> {
    if {$tk_priv(dragging) != ""} {
	[lindex [$tk_priv(dragging) config -menu] 4] post %X %Y
    }
}
bind Menubutton <ButtonRelease-2> {set tk_priv(dragging) ""}

bind Menu <Any-Leave> {set tk_priv(window) {}; %W activate none}
bind Menu <Any-Motion> {
    if {$tk_priv(window) != ""} {
	%W activate @%y
    }
}
bind Menu <ButtonRelease-1> {tk_invokeMenu %W}
bind Menu <2> {set tk_priv(x) %x; set tk_priv(y) %y}
bind Menu <B2-Motion> {
    if {$tk_priv(posted) == ""} {
	%W post [expr %X-$tk_priv(x)] [expr %Y-$tk_priv(y)]
    }
}
bind Menu <B2-Leave> { }
bind Menu <B2-Enter> { }
bind Menu <Escape> {tk_mbUnpost}
bind Menu <Any-KeyPress> {tk_traverseWithinMenu %W %A}
bind Menu <Left> {tk_nextMenu -1}
bind Menu <Right> {tk_nextMenu 1}
bind Menu <Up> {tk_nextMenuEntry -1}
bind Menu <Down> {tk_nextMenuEntry 1}
bind Menu <Return> {tk_invokeMenu %W}

bind Text <1> {
    set tk_priv(selectMode) char
    %W mark set insert @%x,%y
    %W mark set anchor insert
    if {[lindex [%W config -state] 4] == "normal"} {focus %W}
}
bind Text <Double-1> {
    set tk_priv(selectMode) word
    %W mark set insert "@%x,%y wordstart"
    tk_textSelectTo %W insert
}
bind Text <Triple-1> {
    set tk_priv(selectMode) line
    %W mark set insert "@%x,%y linestart"
    tk_textSelectTo %W insert
}
bind Text <B1-Motion> {tk_textSelectTo %W @%x,%y}
bind Text <Shift-1> {
    tk_textResetAnchor %W @%x,%y
    tk_textSelectTo %W @%x,%y
}
bind Text <Shift-B1-Motion> {tk_textSelectTo %W @%x,%y}
bind Text <2> {%W scan mark %y}
bind Text <B2-Motion> {%W scan dragto %y}
bind Text <Any-KeyPress> {
    if {"%A" != ""} {
	%W insert insert %A
	%W yview -pickplace insert
    }
}
bind Text <Return> {%W insert insert \n; %W yview -pickplace insert}
bind Text <BackSpace> {tk_textBackspace %W; %W yview -pickplace insert}
bind Text <Delete> {tk_textBackspace %W; %W yview -pickplace insert}
bind Text <Control-h> {tk_textBackspace %W; %W yview -pickplace insert}
bind Text <Control-d> {%W delete sel.first sel.last}
bind Text <Control-v> {
    %W insert insert [selection get]
    %W yview -pickplace insert
}
tk_bindForTraversal Text

# Initialize the elements of tk_priv that require initialization.

set tk_priv(buttons) 0
set tk_priv(dragging) {}
set tk_priv(focus) {}
set tk_priv(inMenuButton) {}
set tk_priv(posted) {}
set tk_priv(selectMode) char
set tk_priv(window) {}
