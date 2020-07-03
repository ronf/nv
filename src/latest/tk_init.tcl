#
# General TK initialization code
#

scan [info tclversion] "%d.%d" a b
if {$a != 7} {
    error "wrong version of Tcl loaded ([info tclversion]): need 7.x"
}
scan $tk_version "%d.%d" a b
if {($a != 4) || ($b < 0)} {
    error "wrong version of Tk loaded ($tk_version): need 4.x"
}
unset a b

set tk_strictMotif 0

proc tkScreenChanged screen {
    set disp [file rootname $screen]
    uplevel #0 upvar #0 tkPriv.$disp tkPriv
    global tkPriv
    if [info exists tkPriv] {
	set tkPriv(screen) $screen
	return
    }
    set tkPriv(afterId) {}
    set tkPriv(buttons) 0
    set tkPriv(buttonWindow) {}
    set tkPriv(dragging) {}
    set tkPriv(focus) {}
    set tkPriv(grab) {}
    set tkPriv(initPos) {}
    set tkPriv(inMenubutton) {}
    set tkPriv(listboxPrev) {}
    set tkPriv(mouseMoved) 0
    set tkPriv(popup) {}
    set tkPriv(postedMb) {}
    set tkPriv(pressX) 0
    set tkPriv(pressY) 0
    set tkPriv(prevPos) {}
    set tkPriv(screen) $screen
    set tkPriv(selectMode) char
    set tkPriv(window) {}
}

proc tkButtonEnter {w} {
    global tkPriv
    if {[$w cget -state] != "disabled"} {
	$w config -state active
	if {$tkPriv(buttonWindow) == $w} {
	    $w configure -state active -relief sunken
	}
    }
    set tkPriv(window) $w
}

proc tkCancelRepeat {} {
    global tkPriv
    after cancel $tkPriv(afterId)
    set tkPriv(afterId) {}
}

proc tkButtonLeave w {
    global tkPriv
    if {[$w cget -state] != "disabled"} {
	$w config -state normal
    }
    if {$w == $tkPriv(buttonWindow)} {
	$w configure -relief $tkPriv(relief)
    }
    set tkPriv(window) ""
}

proc tkButtonDown w {
    global tkPriv
    set tkPriv(relief) [lindex [$w config -relief] 4]
    if {[$w cget -state] != "disabled"} {
	set tkPriv(buttonWindow) $w
	$w config -relief sunken
    }
}

proc tkButtonUp w {
    global tkPriv
    if {$w == $tkPriv(buttonWindow)} {
	set tkPriv(buttonWindow) ""
	$w config -relief $tkPriv(relief)
	if {($w == $tkPriv(window))
		&& ([$w cget -state] != "disabled")} {
	    uplevel #0 [list $w invoke]
	}
    }
}

proc tkButtonInvoke w {
    if {[$w cget -state] != "disabled"} {
	set oldRelief [$w cget -relief]
	set oldState [$w cget -state]
	$w configure -state active -relief sunken
	update idletasks
	after 100
	$w configure -state $oldState -relief $oldRelief
	uplevel #0 [list $w invoke]
    }
}

proc tkCheckRadioInvoke w {
    if {[$w cget -state] != "disabled"} {
	uplevel #0 [list $w invoke]
    }
}

proc tkEntryClipboardKeysyms {copy cut paste} {
    bind Entry <$copy> {
	if {[selection own -displayof %W] == "%W"} {
	    clipboard clear -displayof %W
	    catch {
		clipboard append -displayof %W [selection get -displayof %W]
	    }
	}
    }
    bind Entry <$cut> {
	if {[selection own -displayof %W] == "%W"} {
	    clipboard clear -displayof %W
	    catch {
		clipboard append -displayof %W [selection get -displayof %W]
		%W delete sel.first sel.last
	    }
	}
    }
    bind Entry <$paste> {
	catch {
	    %W insert insert [selection get -displayof %W \
		    -selection CLIPBOARD]
	}
    }
}

proc tkEntryButton1 {w x} {
    global tkPriv

    set tkPriv(selectMode) char
    set tkPriv(mouseMoved) 0
    set tkPriv(pressX) $x
    $w icursor @$x
    $w selection from @$x
    if {[lindex [$w configure -state] 4] == "normal"} {focus $w}
}

proc tkEntryMouseSelect {w x} {
    global tkPriv

    set cur [$w index @$x]
    set anchor [$w index anchor]
    if {($cur != $anchor) || (abs($tkPriv(pressX) - $x) >= 3)} {
	set tkPriv(mouseMoved) 1
    }
    switch $tkPriv(selectMode) {
	char {
	    if $tkPriv(mouseMoved) {
		if {$cur < [$w index anchor]} {
		    $w selection to $cur
		} else {
		    $w selection to [expr $cur+1]
		}
	    }
	}
	word {
	    if {$cur < [$w index anchor]} {
		$w selection range [string wordstart [$w get] $cur] \
			[string wordend [$w get] [expr $anchor-1]]
	    } else {
		$w selection range [string wordstart [$w get] $anchor] \
			[string wordend [$w get] $cur]
	    }
	}
	line {
	    $w selection range 0 end
	}
    }
    update idletasks
}

proc tkEntryAutoScan {w} {
    global tkPriv
    set x $tkPriv(x)
    if {$x >= [winfo width $w]} {
	$w xview scroll 2 units
	tkEntryMouseSelect $w $x
    } elseif {$x < 0} {
	$w xview scroll -2 units
	tkEntryMouseSelect $w $x
    }
    set tkPriv(afterId) [after 50 tkEntryAutoScan $w]
}

proc tkEntryKeySelect {w new} {
    if ![$w selection present] {
	$w selection from insert
	$w selection to $new
    } else {
	$w selection adjust $new
    }
    $w icursor $new
}

proc tkEntryInsert {w s} {
    if {$s == ""} {
	return
    }
    catch {
	set insert [$w index insert]
	if {([$w index sel.first] <= $insert)
		&& ([$w index sel.last] >= $insert)} {
	    $w delete sel.first sel.last
	}
    }
    $w insert insert $s
    tkEntrySeeInsert $w
}

proc tkEntryBackspace w {
    if [$w selection present] {
	$w delete sel.first sel.last
    } else {
	set x [expr {[$w index insert] - 1}]
	if {$x >= 0} {$w delete $x}
	if {[$w index @0] >= [$w index insert]} {
	    set range [$w xview]
	    set left [lindex $range 0]
	    set right [lindex $range 1]
	    $w xview moveto [expr $left - ($right - $left)/2.0]
	}
    }
}

proc tkEntrySeeInsert w {
    set c [$w index insert]
    set left [$w index @0]
    if {$left > $c} {
	$w xview $c
	return
    }
    set x [winfo width $w]
    while {([$w index @$x] <= $c) && ($left < $c)} {
	incr left
	$w xview $left
    }
}

proc tkEntrySetCursor {w pos} {
    $w icursor $pos
    $w selection clear
    tkEntrySeeInsert $w
}

proc tkEntryTranspose w {
    set i [$w index insert]
    if {$i < [$w index end]} {
	incr i
    }
    set first [expr $i-2]
    if {$first < 0} {
	return
    }
    set new [string index [$w get] [expr $i-1]][string index [$w get] $first]
    $w delete $first $i
    $w insert insert $new
    tkEntrySeeInsert $w
}

proc tkListboxBeginSelect {w el} {
    global tkPriv
    if {[$w cget -selectmode]  == "multiple"} {
	if [$w selection includes $el] {
	    $w selection clear $el
	} else {
	    $w selection set $el
	}
    } else {
	$w selection clear 0 end
	$w selection set $el
	$w selection anchor $el
	set tkPriv(listboxSelection) {}
	set tkPriv(listboxPrev) $el
    }
}

proc tkListboxMotion {w el} {
    global tkPriv
    if {$el == $tkPriv(listboxPrev)} {
	return
    }
    set anchor [$w index anchor]
    switch [$w cget -selectmode] {
	browse {
	    $w selection clear 0 end
	    $w selection set $el
	    set tkPriv(listboxPrev) $el
	}
	extended {
	    set i $tkPriv(listboxPrev)
	    if [$w selection includes anchor] {
		$w selection clear $i $el
		$w selection set anchor $el
	    } else {
		$w selection clear $i $el
		$w selection clear anchor $el
	    }
	    while {($i < $el) && ($i < $anchor)} {
		if {[lsearch $tkPriv(listboxSelection) $i] >= 0} {
		    $w selection set $i
		}
		incr i
	    }
	    while {($i > $el) && ($i > $anchor)} {
		if {[lsearch $tkPriv(listboxSelection) $i] >= 0} {
		    $w selection set $i
		}
		incr i -1
	    }
	    set tkPriv(listboxPrev) $el
	}
    }
}

proc tkListboxBeginExtend {w el} {
    if {([$w cget -selectmode] == "extended")
	    && [$w selection includes anchor]} {
	tkListboxMotion $w $el
    }
}

proc tkListboxBeginToggle {w el} {
    global tkPriv
    if {[$w cget -selectmode] == "extended"} {
	set tkPriv(listboxSelection) [$w curselection]
	set tkPriv(listboxPrev) $el
	$w selection anchor $el
	if [$w selection includes $el] {
	    $w selection clear $el
	} else {
	    $w selection set $el
	}
    }
}

proc tkListboxAutoScan {w} {
    global tkPriv
    set x $tkPriv(x)
    set y $tkPriv(y)
    if {$y >= [winfo height $w]} {
	$w yview scroll 1 units
    } elseif {$y < 0} {
	$w yview scroll -1 units
    } elseif {$x >= [winfo width $w]} {
	$w xview scroll 2 units
    } elseif {$x < 0} {
	$w xview scroll -2 units
    } else {
	return
    }
    tkListboxMotion $w [$w index @$x,$y]
    set tkPriv(afterId) [after 50 tkListboxAutoScan $w]
}

proc tkListboxUpDown {w amount} {
    global tkPriv
    $w activate [expr [$w index active] + $amount]
    $w see active
    switch [$w cget -selectmode] {
	browse {
	    $w selection clear 0 end
	    $w selection set active
	}
	extended {
	    $w selection clear 0 end
	    $w selection set active
	    $w selection anchor active
	    set tkPriv(listboxPrev) [$w index active]
	    set tkPriv(listboxSelection) {}
	}
    }
}

proc tkListboxExtendUpDown {w amount} {
    if {[$w cget -selectmode] != "extended"} {
	return
    }
    $w activate [expr [$w index active] + $amount]
    $w see active
    tkListboxMotion $w [$w index active]
}

proc tkListboxDataExtend {w el} {
    set mode [$w cget -selectmode]
    if {$mode == "extended"} {
	$w activate $el
	$w see $el
        if [$w selection includes anchor] {
	    tkListboxMotion $w $el
	}
    } elseif {$mode == "multiple"} {
	$w activate $el
	$w see $el
    }
}

proc tkListboxCancel w {
    global tkPriv
    if {[$w cget -selectmode] != "extended"} {
	return
    }
    set first [$w index anchor]
    set last $tkPriv(listboxPrev)
    if {$first > $last} {
	set tmp $first
	set first $last
	set last $tmp
    }
    $w selection clear $first $last
    while {$first <= $last} {
	if {[lsearch $tkPriv(listboxSelection) $first] >= 0} {
	    $w selection set $first
	}
	incr first
    }
}

proc tkListboxSelectAll w {
    set mode [$w cget -selectmode]
    if {($mode == "single") || ($mode == "browse")} {
	$w selection clear 0 end
	$w selection set active
    } else {
	$w selection set 0 end
    }
}

proc tkMbEnter w {
    global tkPriv

    if {$tkPriv(inMenubutton) != ""} {
	tkMbLeave $tkPriv(inMenubutton)
    }
    set tkPriv(inMenubutton) $w
    if {[$w cget -state] != "disabled"} {
	$w configure -state active
    }
}

proc tkMbLeave w {
    global tkPriv

    set tkPriv(inMenubutton) {}
    if ![winfo exists $w] {
	return
    }
    if {[$w cget -state] == "active"} {
	$w configure -state normal
    }
}

proc tkMbPost {w {x {}} {y {}}} {
    global tkPriv
    if {([$w cget -state] == "disabled") || ($w == $tkPriv(postedMb))} {
	return
    }
    set menu [$w cget -menu]
    if {($menu == "") || ([$menu index last] == "none")} {
	return
    }
    if ![string match $w.* $menu] {
	error "can't post $menu:  it isn't a descendant of $w (this is a new requirement in Tk versions 3.0 and later)"
    }
    set cur $tkPriv(postedMb)
    if {$cur != ""} {
	tkMenuUnpost {}
    }
    set tkPriv(cursor) [$w cget -cursor]
    set tkPriv(relief) [$w cget -relief]
    $w configure -cursor arrow
    $w configure -relief raised
    set tkPriv(postedMb) $w
    set tkPriv(focus) [focus]
    $menu activate none

    if {([$w cget -indicatoron] == 1) && ([$w cget -textvariable] != "")} {
	if {$y == ""} {
	    set x [expr [winfo rootx $w] + [winfo width $w]/2]
	    set y [expr [winfo rooty $w] + [winfo height $w]/2]
	}
	tkPostOverPoint $menu $x $y [tkMenuFindName $menu [$w cget -text]]
    } else {
	$menu post [winfo rootx $w] [expr [winfo rooty $w]+[winfo height $w]]
    }
    focus $menu
    grab -global $w
}

proc tkMenuUnpost menu {
    global tkPriv
    set mb $tkPriv(postedMb)

    catch {focus $tkPriv(focus)}
    set tkPriv(focus) ""

    catch {
	if {$mb != ""} {
	    set menu [$mb cget -menu]
	    $menu unpost
	    set tkPriv(postedMb) {}
	    $mb configure -cursor $tkPriv(cursor)
	    $mb configure -relief $tkPriv(relief)
	} elseif {$tkPriv(popup) != ""} {
	    $tkPriv(popup) unpost
	    set tkPriv(popup) {}
	} elseif {[wm overrideredirect $menu]} {
	    while 1 {
		set parent [winfo parent $menu]
		if {([winfo class $parent] != "Menu")
			|| ![winfo ismapped $parent]} {
		    break
		}
		$parent activate none
		$parent postcascade none
		if {![wm overrideredirect $parent]} {
		    break
		}
		set menu $parent
	    }
	    $menu unpost
	}
    }

    if {$menu != ""} {
	set grab [grab current $menu]
	if {$grab != ""} {
	    grab release $grab
	}
    }
}

proc tkMbMotion {w upDown rootx rooty} {
    global tkPriv

    if {$tkPriv(inMenubutton) == $w} {
	return
    }
    set new [winfo containing $rootx $rooty]
    if {($new != $tkPriv(inMenubutton)) && (($new == "")
	    || ([winfo toplevel $new] == [winfo toplevel $w]))} {
	if {$tkPriv(inMenubutton) != ""} {
	    tkMbLeave $tkPriv(inMenubutton)
	}
	if {($new != "") && ([winfo class $new] == "Menubutton")
		&& ([$new cget -indicatoron] == 0)} {
	    if {$upDown == "down"} {
		tkMbPost $new $rootx $rooty
	    } else {
		tkMbEnter $new
	    }
	}
    }
}

proc tkMbButtonUp w {
    global tkPriv

    if  {($tkPriv(postedMb) == $w) && ($tkPriv(inMenubutton) == $w)} {
	tkMenuFirstEntry [$tkPriv(postedMb) cget -menu]
    } else {
	tkMenuUnpost {}
    }
}

proc tkMenuMotion {menu y state} {
    global tkPriv
    if {$menu == $tkPriv(window)} {
	$menu activate @$y
    }
    if {($state & 0x1f00) != 0} {
	$menu postcascade active
    }
}

proc tkMenuButtonDown menu {
    global tkPriv
    $menu postcascade active
    if {$tkPriv(postedMb) != ""} {
	grab -global $tkPriv(postedMb)
    } else {
	while {[wm overrideredirect $menu]
		&& ([winfo class [winfo parent $menu]] == "Menu")
		&& [winfo ismapped [winfo parent $menu]]} {
	    set menu [winfo parent $menu]
	}
	grab -global $menu
    }
}

proc tkMenuLeave {menu rootx rooty state} {
    global tkPriv
    set tkPriv(window) {}
    if {[$menu index active] == "none"} {
	return
    }
    if {([$menu type active] == "cascade")
	    && ([winfo containing $rootx $rooty]
	    == [$menu entrycget active -menu])} {
	return
    }
    $menu activate none
}

proc tkMenuInvoke w {
    if {[$w type active] == "cascade"} {
	$w postcascade active
	set menu [$w entrycget active -menu]
	tkMenuFirstEntry $menu
    } elseif {[$w type active] == "tearoff"} {
	tkMenuUnpost $w
	tkTearOffMenu $w
    } else {
	tkMenuUnpost $w
	uplevel #0 [list $w invoke active]
    }
}

proc tkMenuEscape menu {
    if {[winfo class [winfo parent $menu]] != "Menu"} {
	tkMenuUnpost $menu
    } else {
	tkMenuLeftRight $menu -1
    }
}

proc tkMenuLeftRight {menu direction} {
    global tkPriv

    if {$direction == "right"} {
	set count 1
	if {[$menu type active] == "cascade"} {
	    $menu postcascade active
	    set m2 [$menu entrycget active -menu]
	    if {$m2 != ""} {
		tkMenuFirstEntry $m2
	    }
	    return
	}
    } else {
	set count -1
	set m2 [winfo parent $menu]
	if {[winfo class $m2] == "Menu"} {
	    $menu activate none
	    focus $m2

	    set tmp [$m2 index active]
	    $m2 activate none
	    $m2 activate $tmp
	    return
	}
    }

    set w $tkPriv(postedMb)
    if {$w == ""} {
	return
    }
    set buttons [winfo children [winfo parent $w]]
    set length [llength $buttons]
    set i [expr [lsearch -exact $buttons $w] + $count]
    while 1 {
	while {$i < 0} {
	    incr i $length
	}
	while {$i >= $length} {
	    incr i -$length
	}
	set mb [lindex $buttons $i]
	if {([winfo class $mb] == "Menubutton")
		&& ([$mb cget -state] != "disabled")
		&& ([$mb cget -menu] != "")
		&& ([[$mb cget -menu] index last] != "none")} {
	    break
	}
	if {$mb == $w} {
	    return
	}
	incr i $count
    }
    tkMbPost $mb
    tkMenuFirstEntry [$mb cget -menu]
}

proc tkMenuNextEntry {menu count} {
    global tkPriv
    if {[$menu index last] == "none"} {
	return
    }
    set length [expr [$menu index last]+1]
    set active [$menu index active]
    if {$active == "none"} {
	set i 0
    } else {
	set i [expr $active + $count]
    }
    while 1 {
	while {$i < 0} {
	    incr i $length
	}
	while {$i >= $length} {
	    incr i -$length
	}
	if {[catch {$menu entrycget $i -state} state] == 0} {
	    if {$state != "disabled"} {
		break
	    }
	}
	if {$i == $active} {
	    return
	}
	incr i $count
    }
    $menu activate $i
    $menu postcascade $i
}

proc tkMenuFind {w char} {
    global tkPriv
    set char [string tolower $char]

    foreach child [winfo child $w] {
	switch [winfo class $child] {
	    Menubutton {
		set char2 [string index [$child cget -text] \
			[$child cget -underline]]
		if {([string compare $char [string tolower $char2]] == 0)
			|| ($char == "")} {
		    if {[$child cget -state] != "disabled"} {
			return $child
		    }
		}
	    }
	    Frame {
		set match [tkMenuFind $child $char]
		if {$match != ""} {
		    return $match
		}
	    }
	}
    }
    return {}
}

proc tkTraverseToMenu {w char} {
    if {$char == ""} {
	return
    }
    while {[winfo class $w] == "Menu"} {
	set w [winfo parent $w]
    }
    set w [tkMenuFind [winfo toplevel $w] $char]
    if {$w != ""} {
	tkMbPost $w
	tkMenuFirstEntry [$w cget -menu]
    }
}

proc tkFirstMenu w {
    set w [tkMenuFind [winfo toplevel $w] ""]
    if {$w != ""} {
	tkMbPost $w
	tkMenuFirstEntry [$w cget -menu]
    }
}

proc tkTraverseWithinMenu {w char} {
    if {$char == ""} {
	return
    }
    set char [string tolower $char]
    set last [$w index last]
    if {$last == "none"} {
	return
    }
    for {set i 0} {$i <= $last} {incr i} {
	if [catch {set char2 [string index \
		[$w entrycget $i -label] \
		[$w entrycget $i -underline]]}] {
	    continue
	}
	if {[string compare $char [string tolower $char2]] == 0} {
	    if {[$w type $i] == "cascade"} {
		$w postcascade $i
		$w activate $i
		set m2 [$w entrycget $i -menu]
		if {$m2 != ""} {
		    tkMenuFirstEntry $m2
		}
	    } else {
		tkMenuUnpost $w
		uplevel #0 [list $w invoke $i]
	    }
	    return
	}
    }
}

proc tkMenuFirstEntry menu {
    if {$menu == ""} {
	return
    }
    focus $menu
    if {[$menu index active] != "none"} {
	return
    }
    set last [$menu index last]
    if {$last == "none"} {
	return
    }
    for {set i 0} {$i <= $last} {incr i} {
	if {([catch {set state [$menu entrycget $i -state]}] == 0)
		&& ($state != "disabled") && ([$menu type $i] != "tearoff")} {
	    $menu activate $i
	    return
	}
    }
}

proc tkMenuFindName {menu s} {
    set i ""
    if {![regexp {^active$|^last$|^none$|^[0-9]|^@} $s]} {
	catch {set i [$menu index $s]}
	return $i
    }
    set last [$menu index last]
    if {$last == "none"} {
	return
    }
    for {set i 0} {$i <= $last} {incr i} {
	if ![catch {$menu entrycget $i -label} label] {
	    if {$label == $s} {
		return $i
	    }
	}
    }
    return ""
}

proc tkPostOverPoint {menu x y {entry {}}}  {
    if {$entry != {}} {
	if {$entry == [$menu index last]} {
	    incr y [expr -([$menu yposition $entry] \
		    + [winfo reqheight $menu])/2]
	} else {
	    incr y [expr -([$menu yposition $entry] \
		    + [$menu yposition [expr $entry+1]])/2]
	}
	incr x [expr -[winfo reqwidth $menu]/2]
    }
    $menu post $x $y
    if {($entry != {}) && ([$menu entrycget $entry -state] != "disabled")} {
	$menu activate $entry
    }
}

proc tk_popup {menu x y {entry {}}} {
    global tkPriv
    if {($tkPriv(popup) != "") || ($tkPriv(postedMb) != "")} {
	tkMenuUnpost {}
    }
    tkPostOverPoint $menu $x $y $entry
    grab -global $menu
    set tkPriv(popup) $menu
    set tkPriv(focus) [focus]
    focus $menu
}

proc tkScaleActivate {w x y} {
    global tkPriv
    if {[$w cget -state] == "disabled"} {
	return;
    }
    if {[$w identify $x $y] == "slider"} {
	$w configure -state active
    } else {
	$w configure -state normal
    }
}

proc tkScaleButtonDown {w x y} {
    global tkPriv
    set tkPriv(dragging) 0
    set el [$w identify $x $y]
    if {$el == "trough1"} {
	tkScaleIncrement $w up big initial
    } elseif {$el == "trough2"} {
	tkScaleIncrement $w down big initial
    } elseif {$el == "slider"} {
	set tkPriv(dragging) 1
	set tkPriv(initValue) [$w get]
	set coords [$w coords]
	set tkPriv(deltaX) [expr $x - [lindex $coords 0]]
	set tkPriv(deltaY) [expr $y - [lindex $coords 1]]
    }
}

proc tkScaleDrag {w x y} {
    global tkPriv
    if !$tkPriv(dragging) {
	return
    }
    $w set [$w get [expr $x - $tkPriv(deltaX)] \
	    [expr $y - $tkPriv(deltaY)]]
}

proc tkScaleEndDrag {w} {
    global tkPriv
    set tkPriv(dragging) 0
}

proc tkScaleIncrement {w dir big repeat} {
    global tkPriv
    if {$big == "big"} {
	set inc [$w cget -bigincrement]
	if {$inc == 0} {
	    set inc [expr abs([$w cget -to] - [$w cget -from])/10.0]
	}
	if {$inc < [$w cget -resolution]} {
	    set inc [$w cget -resolution]
	}
    } else {
	set inc [$w cget -resolution]
    }
    if {([$w cget -from] > [$w cget -to]) ^ ($dir == "up")} {
	set inc [expr -$inc]
    }
    $w set [expr [$w get] + $inc]

    if {$repeat == "again"} {
	set tkPriv(afterId) [after [$w cget -repeatinterval] \
		tkScaleIncrement $w $dir $big again]
    } elseif {$repeat == "initial"} {
	set tkPriv(afterId) [after [$w cget -repeatdelay] \
		tkScaleIncrement $w $dir $big again]
    }
}

proc tkScaleControlPress {w x y} {
    set el [$w identify $x $y]
    if {$el == "trough1"} {
	$w set [$w cget -from]
    } elseif {$el == "trough2"} {
	$w set [$w cget -to]
    }
}

proc tkScrollButtonDown {w x y} {
    global tkPriv
    set tkPriv(relief) [$w cget -activerelief]
    $w configure -activerelief sunken
    set element [$w identify $x $y]
    if {$element == "slider"} {
	tkScrollStartDrag $w $x $y
    } else {
	tkScrollSelect $w $element initial
    }
}

proc tkScrollButtonUp {w x y} {
    global tkPriv
    tkCancelRepeat
    $w configure -activerelief $tkPriv(relief)
    tkScrollEndDrag $w $x $y
    $w activate [$w identify $x $y]
}

proc tkScrollSelect {w element repeat} {
    global tkPriv
    if {$element == "arrow1"} {
	tkScrollByUnits $w hv -1
    } elseif {$element == "trough1"} {
	tkScrollByPages $w hv -1
    } elseif {$element == "trough2"} {
	tkScrollByPages $w hv 1
    } elseif {$element == "arrow2"} {
	tkScrollByUnits $w hv 1
    } else {
	return
    }
    if {$repeat == "again"} {
	set tkPriv(afterId) [after [$w cget -repeatinterval] \
		tkScrollSelect $w $element again]
    } elseif {$repeat == "initial"} {
	set tkPriv(afterId) [after [$w cget -repeatdelay] \
		tkScrollSelect $w $element again]
    }
}

proc tkScrollStartDrag {w x y} {
    global tkPriv

    if {[$w cget -command] == ""} {
	return
    }
    set tkPriv(pressX) $x
    set tkPriv(pressY) $y
    set tkPriv(initValues) [$w get]
    set iv0 [lindex $tkPriv(initValues) 0]
    if {[llength $tkPriv(initValues)] == 2} {
	set tkPriv(initPos) $iv0
    } else {
	if {$iv0 == 0} {
	    set tkPriv(initPos) 0.0
	} else {
	    set tkPriv(initPos) [expr (double([lindex $tkPriv(initValues) 2])) \
		    / [lindex $tkPriv(initValues) 0]]
	}
    }
}

proc tkScrollDrag {w x y} {
    global tkPriv

    if {$tkPriv(initPos) == ""} {
	return
    }
    set delta [$w delta [expr $x - $tkPriv(pressX)] [expr $y - $tkPriv(pressY)]]
    if [$w cget -jump] {
	if {[llength $tkPriv(initValues)] == 2} {
	    $w set [expr [lindex $tkPriv(initValues) 0] + $delta] \
		    [expr [lindex $tkPriv(initValues) 1] + $delta]
	} else {
	    set delta [expr round($delta * [lindex $tkPriv(initValues) 0])]
	    eval $w set [lreplace $tkPriv(initValues) 2 3 \
		    [expr [lindex $tkPriv(initValues) 2] + $delta] \
		    [expr [lindex $tkPriv(initValues) 3] + $delta]]
	}
    } else {
	tkScrollToPos $w [expr $tkPriv(initPos) + $delta]
    }
}

proc tkScrollEndDrag {w x y} {
    global tkPriv

    if {$tkPriv(initPos) == ""} {
	return
    }
    if [$w cget -jump] {
	set delta [$w delta [expr $x - $tkPriv(pressX)] \
		[expr $y - $tkPriv(pressY)]]
	tkScrollToPos $w [expr $tkPriv(initPos) + $delta]
    }
    set tkPriv(initPos) ""
}

proc tkScrollByUnits {w orient amount} {
    set cmd [$w cget -command]
    if {($cmd == "") || ([string first \
	    [string index [$w cget -orient] 0] $orient] < 0)} {
	return
    }
    set info [$w get]
    if {[llength $info] == 2} {
	uplevel #0 $cmd scroll $amount units
    } else {
	uplevel #0 $cmd [expr [lindex $info 2] + $amount]
    }
}

proc tkScrollByPages {w orient amount} {
    set cmd [$w cget -command]
    if {($cmd == "") || ([string first \
	    [string index [$w cget -orient] 0] $orient] < 0)} {
	return
    }
    set info [$w get]
    if {[llength $info] == 2} {
	uplevel #0 $cmd scroll $amount pages
    } else {
	uplevel #0 $cmd [expr [lindex $info 2] + $amount*([lindex $info 1] - 1)]
    }
}

proc tkScrollToPos {w pos} {
    set cmd [$w cget -command]
    if {($cmd == "")} {
	return
    }
    set info [$w get]
    if {[llength $info] == 2} {
	uplevel #0 $cmd moveto $pos
    } else {
	uplevel #0 $cmd [expr round([lindex $info 0]*$pos)]
    }
}

proc tkScrollTopBottom {w x y} {
    set element [$w identify $x $y]
    if [string match *1 $element] {
	tkScrollToPos $w 0
    } elseif [string match *2 $element] {
	tkScrollToPos $w 1
    }
}

proc tkTextButton1 {w x y} {
    global tkPriv

    set tkPriv(selectMode) char
    set tkPriv(mouseMoved) 0
    set tkPriv(pressX) $x
    $w mark set insert @$x,$y
    $w mark set anchor insert
    if {[$w cget -state] == "normal"} {focus $w}
}

proc tkTextSelectTo {w x y} {
    global tkPriv

    set cur [$w index @$x,$y]
    if [catch {$w index anchor}] {
	$w mark set anchor $cur
    }
    set anchor [$w index anchor]
    if {[$w compare $cur != $anchor] || (abs($tkPriv(pressX) - $x) >= 3)} {
	set tkPriv(mouseMoved) 1
    }
    switch $tkPriv(selectMode) {
	char {
	    if [$w compare $cur < anchor] {
		set first $cur
		set last anchor
	    } else {
		set first anchor
		set last [$w index "$cur + 1c"]
	    }
	}
	word {
	    if [$w compare $cur < anchor] {
		set first [$w index "$cur wordstart"]
		set last [$w index "anchor - 1c wordend"]
	    } else {
		set first [$w index "anchor wordstart"]
		set last [$w index "$cur wordend"]
	    }
	}
	line {
	    if [$w compare $cur < anchor] {
		set first [$w index "$cur linestart"]
		set last [$w index "anchor - 1c lineend + 1c"]
	    } else {
		set first [$w index "anchor linestart"]
		set last [$w index "$cur lineend + 1c"]
	    }
	}
    }
    if {$tkPriv(mouseMoved) || ($tkPriv(selectMode) != "char")} {
	$w tag remove sel 0.0 $first
	$w tag add sel $first $last
	$w tag remove sel $last end
	update idletasks
    }
}

proc tkTextKeyExtend {w index} {
    global tkPriv

    set cur [$w index $index]
    if [catch {$w index anchor}] {
	$w mark set anchor $cur
    }
    set anchor [$w index anchor]
    if [$w compare $cur < anchor] {
	set first $cur
	set last anchor
    } else {
	set first anchor
	set last $cur
    }
    $w tag remove sel 0.0 $first
    $w tag add sel $first $last
    $w tag remove sel $last end
}

proc tkTextAutoScan {w} {
    global tkPriv
    if {$tkPriv(y) >= [winfo height $w]} {
	$w yview scroll 2 units
    } elseif {$tkPriv(y) < 0} {
	$w yview scroll -2 units
    } elseif {$tkPriv(x) >= [winfo width $w]} {
	$w xview scroll 2 units
    } elseif {$tkPriv(x) < 0} {
	$w xview scroll -2 units
    } else {
	return
    }
    tkTextSelectTo $w $tkPriv(x) $tkPriv(y)
    set tkPriv(afterId) [after 50 tkTextAutoScan $w]
}

proc tkTextSetCursor {w pos} {
    global tkPriv

    if [$w compare $pos == end] {
	set pos {end - 1 chars}
    }
    $w mark set insert $pos
    $w tag remove sel 1.0 end
    $w see insert
}

proc tkTextKeySelect {w new} {
    global tkPriv

    if {[$w tag nextrange sel 1.0 end] == ""} {
	if [$w compare $new < insert] {
	    $w tag add sel $new insert
	} else {
	    $w tag add sel insert $new
	}
	$w mark set anchor insert
    } else {
	if [$w compare $new < anchor] {
	    set first $new
	    set last anchor
	} else {
	    set first anchor
	    set last $new
	}
	$w tag remove sel 1.0 $first
	$w tag add sel $first $last
	$w tag remove sel $last end
    }
    $w mark set insert $new
    $w see insert
    update idletasks
}

proc tkTextResetAnchor {w index} {
    global tkPriv

    if {[$w tag ranges sel] == ""} {
	$w mark set anchor $index
	return
    }
    set a [$w index $index]
    set b [$w index sel.first]
    set c [$w index sel.last]
    if [$w compare $a < $b] {
	$w mark set anchor sel.last
	return
    }
    if [$w compare $a > $c] {
	$w mark set anchor sel.first
	return
    }
    scan $a "%d.%d" lineA chA
    scan $b "%d.%d" lineB chB
    scan $c "%d.%d" lineC chC
    if {$lineB < $lineC+2} {
	set total [string length [$w get $b $c]]
	if {$total <= 2} {
	    return
	}
	if {[string length [$w get $b $a]] < ($total/2)} {
	    $w mark set anchor sel.last
	} else {
	    $w mark set anchor sel.first
	}
	return
    }
    if {($lineA-$lineB) < ($lineC-$lineA)} {
	$w mark set anchor sel.last
    } else {
	$w mark set anchor sel.first
    }
}

proc tkTextInsert {w s} {
    if {($s == "") || ([$w cget -state] == "disabled")} {
	return
    }
    catch {
	if {[$w compare sel.first <= insert]
		&& [$w compare sel.last >= insert]} {
	    $w delete sel.first sel.last
	}
    }
    $w insert insert $s
    $w see insert
}

proc tkTextUpDownLine {w n} {
    global tkPriv

    set i [$w index insert]
    scan $i "%d.%d" line char
    if {[string compare $tkPriv(prevPos) $i] != 0} {
	set tkPriv(char) $char
    }
    set new [$w index [expr $line + $n].$tkPriv(char)]
    if {[$w compare $new == end] || [$w compare $new == "insert linestart"]} {
	set new $i
    }
    set tkPriv(prevPos) $new
    return $new
}

proc tkTextPrevPara {w pos} {
    set pos [$w index "$pos linestart"]
    while 1 {
	if {(([$w get "$pos - 1 line"] == "\n") && ([$w get $pos] != "\n"))
		|| ($pos == "1.0")} {
	    if [regexp -indices {^[ 	]+(.)} [$w get $pos "$pos lineend"] \
		    dummy index] {
		set pos [$w index "$pos + [lindex $index 0] chars"]
	    }
	    if {[$w compare $pos != insert] || ($pos == "1.0")} {
		return $pos
	    }
	}
	set pos [$w index "$pos - 1 line"]
    }
}

proc tkTextNextPara {w start} {
    set pos [$w index "$start linestart + 1 line"]
    while {[$w get $pos] != "\n"} {
	if [$w compare $pos == end] {
	    return [$w index "end - 1c"]
	}
	set pos [$w index "$pos + 1 line"]
    }
    while {[$w get $pos] == "\n"} {
	set pos [$w index "$pos + 1 line"]
	if [$w compare $pos == end] {
	    return [$w index "end - 1c"]
	}
    }
    if [regexp -indices {^[ 	]+(.)} [$w get $pos "$pos lineend"] \
	    dummy index] {
	return [$w index "$pos + [lindex $index 0] chars"]
    }
    return $pos
}

proc tkTextScrollPages {w count} {
    set bbox [$w bbox insert]
    $w yview scroll $count pages
    if {$bbox == ""} {
	return [$w index @[expr [winfo height $w]/2],0]
    }
    return [$w index @[lindex $bbox 0],[lindex $bbox 1]]
}

proc tkTextTranspose w {
    set pos insert
    if [$w compare $pos != "$pos lineend"] {
	set pos [$w index "$pos + 1 char"]
    }
    set new [$w get "$pos - 1 char"][$w get  "$pos - 2 char"]
    if [$w compare "$pos - 1 char" == 1.0] {
	return
    }
    $w delete "$pos - 2 char" $pos
    $w insert insert $new
    $w see insert
}

proc tk_dialog {w title text bitmap default args} {
    global tkPriv

    catch {destroy $w}
    toplevel $w -class Dialog
    wm title $w $title
    wm iconname $w Dialog
    wm protocol $w WM_DELETE_WINDOW { }
    wm transient $w [winfo toplevel [winfo parent $w]]
    frame $w.top -relief raised -bd 1
    pack $w.top -side top -fill both
    frame $w.bot -relief raised -bd 1
    pack $w.bot -side bottom -fill both

    label $w.msg -wraplength 3i -justify left -text $text \
	    -font -Adobe-Times-Medium-R-Normal--*-180-*-*-*-*-*-*
    pack $w.msg -in $w.top -side right -expand 1 -fill both -padx 3m -pady 3m
    if {$bitmap != ""} {
	label $w.bitmap -bitmap $bitmap
	pack $w.bitmap -in $w.top -side left -padx 3m -pady 3m
    }

    set i 0
    foreach but $args {
	button $w.button$i -text $but -command "set tkPriv(button) $i"
	if {$i == $default} {
	    frame $w.default -relief sunken -bd 1
	    raise $w.button$i $w.default
	    pack $w.default -in $w.bot -side left -expand 1 -padx 3m -pady 2m
	    pack $w.button$i -in $w.default -padx 2m -pady 2m
	    bind $w <Return> "$w.button$i flash; set tkPriv(button) $i"
	} else {
	    pack $w.button$i -in $w.bot -side left -expand 1 \
		    -padx 3m -pady 2m
	}
	incr i
    }

    wm withdraw $w
    update idletasks
    set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 \
	    - [winfo vrootx [winfo parent $w]]]
    set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 \
	    - [winfo vrooty [winfo parent $w]]]
    wm geom $w +$x+$y
    wm deiconify $w

    set oldFocus [focus]
    grab $w
    tkwait visibility $w
    if {$default >= 0} {
	focus $w.button$default
    } else {
	focus $w
    }

    tkwait variable tkPriv(button)
    catch {focus $oldFocus}
    destroy $w
    return $tkPriv(button)
}

proc tk_focusNext w {
    set cur $w
    while 1 {
	set parent $cur
	set children [winfo children $cur]
	set i -1

	while 1 {
	    incr i
	    if {$i < [llength $children]} {
		set cur [lindex $children $i]
		if {[winfo toplevel $cur] == $cur} {
		    continue
		} else {
		    break
		}
	    }

	    set cur $parent
	    if {[winfo toplevel $cur] == $cur} {
		break
	    }
	    set parent [winfo parent $parent]
	    set children [winfo children $parent]
	    set i [lsearch -exact $children $cur]
	}
	if {($cur == $w) || [tkFocusOK $cur]} {
	    return $cur
	}
    }
}

proc tk_focusPrev w {
    set cur $w
    while 1 {
	if {[winfo toplevel $cur] == $cur}  {
	    set parent $cur
	    set children [winfo children $cur]
	    set i [llength $children]
	} else {
	    set parent [winfo parent $cur]
	    set children [winfo children $parent]
	    set i [lsearch -exact $children $cur]
	}

	while {$i > 0} {
	    incr i -1
	    set cur [lindex $children $i]
	    if {[winfo toplevel $cur] == $cur} {
		continue
	    }
	    set parent $cur
	    set children [winfo children $parent]
	    set i [llength $children]
	}
	set cur $parent
	if {($cur == $w) || [tkFocusOK $cur]} {
	    return $cur
	}
    }
}

proc tkFocusOK w {
    if {![winfo viewable $w]} {
	return 0
    }
    set code [catch {$w cget -takefocus} value]
    if {($code == 0) && ($value != "")} {
	if {$value == 0} {
	    return 0
	} elseif {$value == 1} {
	    return 1
	} else {
	    set value [uplevel #0 $value $w]
	    if {$value != ""} {
		return $value
	    }
	}
    }
    set code [catch {$w cget -state} value]
    if {($code == 0) && ($value == "disabled")} {
	return 0
    }
    regexp Key|Focus "[bind $w] [bind [winfo class $w]]"
}

proc tk_focusFollowsMouse {} {
    set old [bind all <Enter>]
    set script {
	if {("%d" == "NotifyAncestor") || ("%d" == "NotifyNonlinear")
		|| ("%d" == "NotifyInferior")} {
	    focus %W
	}
    }
    if {$old != ""} {
	bind all <Enter> "$old; $script"
    } else {
	bind all <Enter> $script
    }
}

proc tk_optionMenu {w varName firstValue args} {
    upvar #0 $varName var

    if ![info exists var] {
	set var $firstValue
    }
    menubutton $w -textvariable $varName -indicatoron 1 -menu $w.menu \
	    -relief raised -bd 2 -padx 4p -pady 4p -highlightthickness 2 \
	    -anchor c
    menu $w.menu -tearoff 0
    $w.menu add command -label $firstValue \
	    -command [list set $varName $firstValue]
    foreach i $args {
	$w.menu add command -label $i -command [list set $varName $i]
    }
    return $w.menu
}

proc tkTearOffMenu w {
    set parent [winfo parent $w]
    while {([winfo toplevel $parent] != $parent)
	    || ([winfo class $parent] == "Menu")} {
	set parent [winfo parent $parent]
    }
    if {$parent == "."} {
	set parent ""
    }
    for {set i 1} 1 {incr i} {
	set menu $parent.tearoff$i
	if ![winfo exists $menu] {
	    break
	}
    }

    tkMenuDup $w $menu
    wm overrideredirect $menu 0

    set parent [winfo parent $w]
    switch [winfo class $parent] {
	Menubutton {
	    wm title $menu [$parent cget -text]
	}
	Menu {
	    wm title $menu [$parent entrycget active -label]
	}
    }

    $menu configure -tearoff 0
    $menu post [winfo x $w] [winfo y $w]

    bind $menu <Enter> {
	set tkPriv(focus) %W
    }
}

proc tkMenuDup {src dst} {
    set cmd "menu $dst"
    foreach option [$src configure] {
	if {[llength $option] == 2} {
	    continue
	}
	lappend cmd [lindex $option 0] [lindex $option 4]
    }
    eval $cmd
    set last [$src index last]
    if {$last == "none"} {
	return
    }
    for {set i [$src cget -tearoff]} {$i <= $last} {incr i} {
	set cmd "$dst add [$src type $i]"
	foreach option [$src entryconfigure $i]  {
	    lappend cmd [lindex $option 0] [lindex $option 4]
	}
	eval $cmd
	if {[$src type $i] == "cascade"} {
	    tkMenuDup [$src entrycget $i -menu] $dst.m$i
	    $dst entryconfigure $i -menu $dst.m$i
	}
    }

    regsub -all . $src {\\&} quotedSrc
    regsub -all . $dst {\\&} quotedDst
    regsub -all $quotedSrc [bindtags $src] $dst x
    bindtags $dst $x
    foreach event [bind $src] {
	regsub -all $quotedSrc [bind $src $event] $dst x
	bind $dst $event $x
    }
}

proc tkerror err {
    global errorInfo

    if {[grab current .] != ""} {
	grab release [grab current .]
    }
    set info $errorInfo
    set button [tk_dialog .tkerrorDialog "Error in Tcl Script" \
	    "Error: $err" error 0 OK "Skip Messages" "Stack Trace"]
    if {$button == 0} {
	return
    } elseif {$button == 1} {
	return -code break
    }

    set w .tkerrorTrace
    catch {destroy $w}
    toplevel $w -class ErrorTrace
    wm minsize $w 1 1
    wm title $w "Stack Trace for Error"
    wm iconname $w "Stack Trace"
    button $w.ok -text OK -command "destroy $w"
    text $w.text -relief sunken -bd 2 -yscrollcommand "$w.scroll set" \
	    -setgrid true -width 60 -height 20
    scrollbar $w.scroll -relief sunken -command "$w.text yview"
    pack $w.ok -side bottom -padx 3m -pady 2m
    pack $w.scroll -side right -fill y
    pack $w.text -side left -expand yes -fill both
    $w.text insert 0.0 $info
    $w.text mark set insert 0.0

    wm withdraw $w
    update idletasks
    set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 \
	    - [winfo vrootx [winfo parent $w]]]
    set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 \
	    - [winfo vrooty [winfo parent $w]]]
    wm geom $w +$x+$y
    wm deiconify $w
}

tkScreenChanged [winfo screen .]

bind Button <FocusIn> {}
bind Button <Enter> {tkButtonEnter %W}
bind Button <Leave> {tkButtonLeave %W}
bind Button <1> {tkButtonDown %W}
bind Button <ButtonRelease-1> {tkButtonUp %W}
bind Button <space> {tkButtonInvoke %W}
bind Button <Return> {
    if !$tk_strictMotif {
	tkButtonInvoke %W
    }
}

bind Checkbutton <FocusIn> {}
bind Checkbutton <Enter> {tkButtonEnter %W}
bind Checkbutton <Leave> {tkButtonLeave %W}
bind Checkbutton <1> {tkCheckRadioInvoke %W}
bind Checkbutton <space> {tkCheckRadioInvoke %W}
bind Checkbutton <Return> {
    if !$tk_strictMotif {
	tkCheckRadioInvoke %W
    }
}

bind Radiobutton <FocusIn> {}
bind Radiobutton <Enter> {tkButtonEnter %W}
bind Radiobutton <Leave> {tkButtonLeave %W}
bind Radiobutton <1> {tkCheckRadioInvoke %W}
bind Radiobutton <space> {tkCheckRadioInvoke %W}
bind Radiobutton <Return> {
    if !$tk_strictMotif {
	tkCheckRadioInvoke %W
    }
}

bind Entry <1> {tkEntryButton1 %W %x; %W selection clear}
bind Entry <B1-Motion> {set tkPriv(x) %x; tkEntryMouseSelect %W %x}
bind Entry <Double-1> {
    set tkPriv(selectMode) word
    tkEntryMouseSelect %W %x
    catch {%W icursor sel.first}
}
bind Entry <Triple-1> {
    set tkPriv(selectMode) line
    tkEntryMouseSelect %W %x
    %W icursor 0
}
bind Entry <Shift-1> {
    set tkPriv(selectMode) char
    %W selection adjust @%x
}
bind Entry <Double-Shift-1> {
    set tkPriv(selectMode) word
    tkEntryMouseSelect %W %x
}
bind Entry <Triple-Shift-1>	{
    set tkPriv(selectMode) line
    tkEntryMouseSelect %W %x
}
bind Entry <B1-Leave> {set tkPriv(x) %x; tkEntryAutoScan %W}
bind Entry <B1-Enter> {tkCancelRepeat}
bind Entry <ButtonRelease-1> {tkCancelRepeat}
bind Entry <Control-1> {%W icursor @%x}
bind Entry <Left> {tkEntrySetCursor %W [expr [%W index insert]-1]}
bind Entry <Right> {tkEntrySetCursor %W [expr [%W index insert]+1]}
bind Entry <Shift-Left> {
    tkEntryKeySelect %W [expr [%W index insert]-1]
    tkEntrySeeInsert %W
}
bind Entry <Shift-Right> {
    tkEntryKeySelect %W [expr [%W index insert]+1]
    tkEntrySeeInsert %W
}
bind Entry <Control-Left> {
    tkEntrySetCursor %W [string wordstart [%W get] [expr [%W index insert]-1]]
}
bind Entry <Control-Right> {
    tkEntrySetCursor %W [string wordend [%W get] [%W index insert]]
}
bind Entry <Shift-Control-Left> {
    tkEntryKeySelect %W [string wordstart [%W get] [expr [%W index insert]-1]]
    tkEntrySeeInsert %W
}
bind Entry <Shift-Control-Right> {
    tkEntryKeySelect %W [string wordend [%W get] [%W index insert]]
    tkEntrySeeInsert %W
}
bind Entry <Home> {tkEntrySetCursor %W 0}
bind Entry <Shift-Home> {tkEntryKeySelect %W 0; tkEntrySeeInsert %W}
bind Entry <End> {tkEntrySetCursor %W end}
bind Entry <Shift-End> {tkEntryKeySelect %W end; tkEntrySeeInsert %W}
bind Entry <Delete> {tkEntryBackspace %W}
bind Entry <BackSpace> {tkEntryBackspace %W}
bind Entry <Control-space> {%W selection from insert}
bind Entry <Select> {%W selection from insert}
bind Entry <Control-Shift-space> {%W selection adjust insert}
bind Entry <Shift-Select> {%W selection adjust insert}
bind Entry <Control-slash> {%W selection range 0 end}
bind Entry <Control-backslash> {%W selection clear}
tkEntryClipboardKeysyms F16 F20 F18
bind Entry <KeyPress> {tkEntryInsert %W %A}
bind Entry <Alt-KeyPress> {# nothing}
bind Entry <Meta-KeyPress> {# nothing}
bind Entry <Control-KeyPress> {# nothing}
bind Entry <Escape> {# nothing}
bind Entry <Return> {# nothing}
bind Entry <KP_Enter> {# nothing}
bind Entry <Tab> {# nothing}
bind Entry <Insert> {catch {tkEntryInsert %W [selection get -displayof %W]}}

if !$tk_strictMotif {
    bind Entry <Control-a> {tkEntrySetCursor %W 0}
    bind Entry <Control-b> {tkEntrySetCursor %W [expr [%W index insert]-1]}
    bind Entry <Control-d> {%W delete insert}
    bind Entry <Control-e> {tkEntrySetCursor %W end}
    bind Entry <Control-f> {tkEntrySetCursor %W [expr [%W index insert]+1]}
    bind Entry <Control-h> {tkEntryBackspace %W}
    bind Entry <Control-k> {%W delete insert end}
    bind Entry <Control-t> {tkEntryTranspose %W}
    bind Entry <Meta-b> {
	tkEntrySetCursor %W [string wordstart [%W get] \
				    [expr [%W index insert]-1]]
    }
    bind Entry <Meta-d> {
	%W delete insert [string wordend [%W get] [%W index insert]]
    }
    bind Entry <Meta-f> {
	tkEntrySetCursor %W [string wordend [%W get] [%W index insert]]
    }
    bind Entry <Meta-BackSpace> {
	%W delete [string wordstart [%W get] [expr [%W index insert]-1]] insert
    }
    tkEntryClipboardKeysyms Meta-w Control-w Control-y

    bind Entry <2> {
	%W scan mark %x
	set tkPriv(x) %x
	set tkPriv(y) %y
	set tkPriv(mouseMoved) 0
    }
    bind Entry <B2-Motion> {
	if {abs(%x-$tkPriv(x)) > 2} {
	    set tkPriv(mouseMoved) 1
	}
	%W scan dragto %x
    }
    bind Entry <ButtonRelease-2> {
	if !$tkPriv(mouseMoved) {
	    catch {%W insert @%x [selection get -displayof %W]}
	}
    }
}

bind Listbox <1> {tkListboxBeginSelect %W [%W index @%x,%y]}
bind Listbox <B1-Motion> {
    set tkPriv(x) %x
    set tkPriv(y) %y
    tkListboxMotion %W [%W index @%x,%y]
}
bind Listbox <ButtonRelease-1> {tkCancelRepeat; %W activate @%x,%y}
bind Listbox <Shift-1> {tkListboxBeginExtend %W [%W index @%x,%y]}
bind Listbox <Control-1> {tkListboxBeginToggle %W [%W index @%x,%y]}
bind Listbox <B1-Leave> {
    set tkPriv(x) %x
    set tkPriv(y) %y
    tkListboxAutoScan %W
}
bind Listbox <B1-Enter> {tkCancelRepeat}
bind Listbox <Up> {tkListboxUpDown %W -1}
bind Listbox <Shift-Up> {tkListboxExtendUpDown %W -1}
bind Listbox <Down> {tkListboxUpDown %W 1}
bind Listbox <Shift-Down> {tkListboxExtendUpDown %W 1}
bind Listbox <Left> {%W xview scroll -1 units}
bind Listbox <Control-Left> {%W xview scroll -1 pages}
bind Listbox <Right> {%W xview scroll 1 units}
bind Listbox <Control-Right> {%W xview scroll 1 pages}
bind Listbox <Prior> {%W yview scroll -1 pages}
bind Listbox <Next> {%W yview scroll 1 pages}
bind Listbox <Control-Prior> {%W xview scroll -1 pages}
bind Listbox <Control-Next> {%W xview scroll 1 pages}
bind Listbox <Home> {%W xview moveto 0}
bind Listbox <End> {%W xview moveto 1}
bind Listbox <Control-Home> {
    %W activate 0
    %W see 0
    %W selection clear 0 end
    %W selection set 0
}
bind Listbox <Shift-Control-Home> {tkListboxDataExtend %W 0}
bind Listbox <Control-End> {
    %W activate end
    %W see end
    %W selection clear 0 end
    %W selection set end
}
bind Listbox <Shift-Control-End> {tkListboxDataExtend %W end}
bind Listbox <F16> {
    if {[selection own -displayof %W] == "%W"} {
	clipboard clear -displayof %W
	clipboard append -displayof %W [selection get -displayof %W]
    }
}
bind Listbox <space> {tkListboxBeginSelect %W [%W index active]}
bind Listbox <Select> {tkListboxBeginSelect %W [%W index active]}
bind Listbox <Control-Shift-space> {tkListboxBeginExtend %W [%W index active]}
bind Listbox <Shift-Select> {tkListboxBeginExtend %W [%W index active]}
bind Listbox <Escape> {tkListboxCancel %W}
bind Listbox <Control-slash> {tkListboxSelectAll %W}
bind Listbox <Control-backslash> {
    if {[%W cget -selectmode] != "browse"} {
	%W selection clear 0 end
    }
}
bind Listbox <2> {%W scan mark %x %y}
bind Listbox <B2-Motion> {%W scan dragto %x %y}

bind Menubutton <FocusIn> {}
bind Menubutton <Enter> {tkMbEnter %W}
bind Menubutton <Leave> {tkMbLeave %W}
bind Menubutton <1> {
    if {$tkPriv(inMenubutton) != ""} {
	tkMbPost $tkPriv(inMenubutton) %X %Y
    }
}
bind Menubutton <Motion> {tkMbMotion %W up %X %Y}
bind Menubutton <B1-Motion> {tkMbMotion %W down %X %Y}
bind Menubutton <ButtonRelease-1> {tkMbButtonUp %W}
bind Menubutton <space> {
    tkMbPost %W
    tkMenuFirstEntry [%W cget -menu]
}
bind Menubutton <Return> {
    tkMbPost %W
    tkMenuFirstEntry [%W cget -menu]
}

bind Menu <FocusIn> {}
bind Menu <Enter> {
    set tkPriv(window) %W
    focus %W
}
bind Menu <Leave> {tkMenuLeave %W %X %Y %s}
bind Menu <Motion> {tkMenuMotion %W %y %s}
bind Menu <ButtonPress> {tkMenuButtonDown %W}
bind Menu <ButtonRelease> {tkMenuInvoke %W}
bind Menu <space> {tkMenuInvoke %W}
bind Menu <Return> {tkMenuInvoke %W}
bind Menu <Escape> {tkMenuEscape %W}
bind Menu <Left> {tkMenuLeftRight %W left}
bind Menu <Right> {tkMenuLeftRight %W right}
bind Menu <Up> {tkMenuNextEntry %W -1}
bind Menu <Down> {tkMenuNextEntry %W +1}
bind Menu <KeyPress> {tkTraverseWithinMenu %W %A}

bind all <Alt-KeyPress> {tkTraverseToMenu %W %A}
bind all <F10> {tkFirstMenu %W}

bind Scale <Enter> {
    if $tk_strictMotif {
	set tkPriv(activeBg) [%W cget -activebackground]
	%W config -activebackground [%W cget -background]
    }
    tkScaleActivate %W %x %y
}
bind Scale <Motion> {tkScaleActivate %W %x %y}
bind Scale <Leave> {
    if $tk_strictMotif {
	%W config -activebackground $tkPriv(activeBg)
    }
    if {[%W cget -state] == "active"} {
	%W configure -state normal
    }
}
bind Scale <1> {tkScaleButtonDown %W %x %y}
bind Scale <B1-Motion> {tkScaleDrag %W %x %y}
bind Scale <B1-Leave> {}
bind Scale <B1-Enter> {}
bind Scale <ButtonRelease-1> {
    tkCancelRepeat
    tkScaleEndDrag %W
    tkScaleActivate %W %x %y
}
bind Scale <2> {tkScaleButtonDown %W %x %y}
bind Scale <B2-Motion> {tkScaleDrag %W %x %y}
bind Scale <B2-Leave> {}
bind Scale <B2-Enter> {}
bind Scale <ButtonRelease-2> {
    tkCancelRepeat
    tkScaleEndDrag %W
    tkScaleActivate %W %x %y
}
bind Scale <Control-1> {tkScaleControlPress %W %x %y}
bind Scale <Up> {tkScaleIncrement %W up little noRepeat}
bind Scale <Down> {tkScaleIncrement %W down little noRepeat}
bind Scale <Left> {tkScaleIncrement %W up little noRepeat}
bind Scale <Right> {tkScaleIncrement %W down little noRepeat}
bind Scale <Control-Up> {tkScaleIncrement %W up big noRepeat}
bind Scale <Control-Down> {tkScaleIncrement %W down big noRepeat}
bind Scale <Control-Left> {tkScaleIncrement %W up big noRepeat}
bind Scale <Control-Right> {tkScaleIncrement %W down big noRepeat}
bind Scale <Home> {%W set [%W cget -from]}
bind Scale <End> {%W set [%W cget -to]}

bind Scrollbar <Enter> {
    if $tk_strictMotif {
	set tkPriv(activeBg) [%W cget -activebackground]
	%W config -activebackground [%W cget -background]
    }
    %W activate [%W identify %x %y]
}
bind Scrollbar <Motion> {%W activate [%W identify %x %y]}
bind Scrollbar <Leave> {
    if $tk_strictMotif {
	%W config -activebackground $tkPriv(activeBg)
    }
    %W activate {}
}
bind Scrollbar <1> {tkScrollButtonDown %W %x %y}
bind Scrollbar <B1-Motion> {tkScrollDrag %W %x %y}
bind Scrollbar <ButtonRelease-1> {tkScrollButtonUp %W %x %y}
bind Scrollbar <B1-Leave> {# Prevents <Leave> binding from being invoked.}
bind Scrollbar <B1-Enter> {# Prevents <Enter> binding from being invoked.}
bind Scrollbar <2> {tkScrollButtonDown %W %x %y}
bind Scrollbar <B2-Motion> {tkScrollDrag %W %x %y}
bind Scrollbar <ButtonRelease-2> {tkScrollButtonUp %W %x %y}
bind Scrollbar <B2-Leave> {# Prevents <Leave> binding from being invoked.}
bind Scrollbar <B2-Enter> {# Prevents <Enter> binding from being invoked.}
bind Scrollbar <Control-1> {tkScrollTopBottom %W %x %y}
bind Scrollbar <Control-2> {tkScrollTopBottom %W %x %y}

bind Scrollbar <Up> {tkScrollByUnits %W v -1}
bind Scrollbar <Down> {tkScrollByUnits %W v 1}
bind Scrollbar <Control-Up> {tkScrollByPages %W v -1}
bind Scrollbar <Control-Down> {tkScrollByPages %W v 1}
bind Scrollbar <Left> {tkScrollByUnits %W h -1}
bind Scrollbar <Right> {tkScrollByUnits %W h 1}
bind Scrollbar <Control-Left> {tkScrollByPages %W h -1}
bind Scrollbar <Control-Right> {tkScrollByPages %W h 1}
bind Scrollbar <Prior> {tkScrollByPages %W hv -1}
bind Scrollbar <Next> {tkScrollByPages %W hv 1}
bind Scrollbar <Home> {tkScrollToPos %W 0}
bind Scrollbar <End> {tkScrollToPos %W 1}

bind all <Tab> {focus [tk_focusNext %W]}
bind all <Shift-Tab> {focus [tk_focusPrev %W]}

proc tkTextClipboardKeysyms {copy cut paste} {
    bind Text <$copy> {
	if {[selection own -displayof %W] == "%W"} {
	    clipboard clear -displayof %W
	    catch {
		clipboard append -displayof %W [selection get -displayof %W]
	    }
	}
    }
    bind Text <$cut> {
	if {[selection own -displayof %W] == "%W"} {
	    clipboard clear -displayof %W
	    catch {
		clipboard append -displayof %W [selection get -displayof %W]
		%W delete sel.first sel.last
	    }
	}
    }
    bind Text <$paste> {
	catch {
	    %W insert insert [selection get -displayof %W \
		    -selection CLIPBOARD]
	}
    }
}

bind Text <1> {tkTextButton1 %W %x %y; %W tag remove sel 0.0 end}
bind Text <B1-Motion> {
    set tkPriv(x) %x
    set tkPriv(y) %y
    tkTextSelectTo %W %x %y
}
bind Text <Double-1> {
    set tkPriv(selectMode) word
    tkTextSelectTo %W %x %y
    catch {%W mark set insert sel.first}
}
bind Text <Triple-1> {
    set tkPriv(selectMode) line
    tkTextSelectTo %W %x %y
    catch {%W mark set insert sel.first}
}
bind Text <Shift-1> {
    tkTextResetAnchor %W @%x,%y
    set tkPriv(selectMode) char
    tkTextSelectTo %W %x %y
}
bind Text <Double-Shift-1> {
    set tkPriv(selectMode) word
    tkTextSelectTo %W %x %y
}
bind Text <Triple-Shift-1> {
    set tkPriv(selectMode) line
    tkTextSelectTo %W %x %y
}
bind Text <B1-Leave> {
    set tkPriv(x) %x
    set tkPriv(y) %y
    tkTextAutoScan %W
}
bind Text <B1-Enter> {tkCancelRepeat}
bind Text <ButtonRelease-1> {tkCancelRepeat}
bind Text <Control-1> {%W mark set insert @%x,%y}
bind Text <Left> {tkTextSetCursor %W [%W index {insert - 1c}]}
bind Text <Right> {tkTextSetCursor %W [%W index {insert + 1c}]}
bind Text <Up> {tkTextSetCursor %W [tkTextUpDownLine %W -1]}
bind Text <Down> {tkTextSetCursor %W [tkTextUpDownLine %W 1]}
bind Text <Shift-Left> {tkTextKeySelect %W [%W index {insert - 1c}]}
bind Text <Shift-Right> {tkTextKeySelect %W [%W index {insert + 1c}]}
bind Text <Shift-Up> {tkTextKeySelect %W [tkTextUpDownLine %W -1]}
bind Text <Shift-Down> {tkTextKeySelect %W [tkTextUpDownLine %W 1]}
bind Text <Control-Left> {tkTextSetCursor %W [%W index {insert - 1c wordstart}]}
bind Text <Control-Right> {tkTextSetCursor %W [%W index {insert wordend}]}
bind Text <Control-Up> {tkTextSetCursor %W [tkTextPrevPara %W insert]}
bind Text <Control-Down> {tkTextSetCursor %W [tkTextNextPara %W insert]}
bind Text <Shift-Control-Left> {
    tkTextKeySelect %W [%W index {insert - 1c wordstart}]
}
bind Text <Shift-Control-Right> {
    tkTextKeySelect %W [%W index {insert wordend}]
}
bind Text <Shift-Control-Up> {tkTextKeySelect %W [tkTextPrevPara %W insert]}
bind Text <Shift-Control-Down> {tkTextKeySelect %W [tkTextNextPara %W insert]}
bind Text <Prior> {tkTextSetCursor %W [tkTextScrollPages %W -1]}
bind Text <Shift-Prior> {tkTextKeySelect %W [tkTextScrollPages %W -1]}
bind Text <Next> {tkTextSetCursor %W [tkTextScrollPages %W 1]}
bind Text <Shift-Next> {tkTextKeySelect %W [tkTextScrollPages %W 1]}
bind Text <Control-Prior> {%W xview scroll -1 page}
bind Text <Control-Next> {%W xview scroll 1 page}
bind Text <Home> {tkTextSetCursor %W {insert linestart}}
bind Text <Shift-Home> {tkTextKeySelect %W {insert linestart}}
bind Text <End> {tkTextSetCursor %W {insert lineend}}
bind Text <Shift-End> {tkTextKeySelect %W {insert lineend}}
bind Text <Control-Home> {tkTextSetCursor %W 1.0}
bind Text <Control-Shift-Home> {tkTextKeySelect %W 1.0}
bind Text <Control-End> {tkTextSetCursor %W {end - 1 char}}
bind Text <Control-Shift-End> {tkTextKeySelect %W {end - 1 char}}

bind Text <Tab> {tkTextInsert %W \t; focus %W; break}
bind Text <Shift-Tab> {# Needed only to keep <Tab> binding from triggering.}
bind Text <Control-Tab> {focus [tk_focusNext %W]}
bind Text <Control-Shift-Tab> {focus [tk_focusPrev %W]}
bind Text <Control-i> {tkTextInsert %W \t}
bind Text <Return> {tkTextInsert %W \n}
bind Text <Delete> {
    if {[%W tag nextrange sel 1.0 end] != ""} {
	%W delete sel.first sel.last
    } else {
	%W delete insert-1c
	%W see insert
    }
}
bind Text <BackSpace> {
    if {[%W tag nextrange sel 1.0 end] != ""} {
	%W delete sel.first sel.last
    } elseif [%W compare insert != 1.0] {
	%W delete insert-1c
	%W see insert
    }
}
bind Text <Control-space> {%W mark set anchor insert}
bind Text <Select> {%W mark set anchor insert}
bind Text <Control-Shift-space> {
    set tkPriv(selectMode) char
    tkTextKeyExtend %W insert
}
bind Text <Shift-Select> {
    set tkPriv(selectMode) char
    tkTextKeyExtend %W insert
}
bind Text <Control-slash> {%W tag add sel 1.0 end}
bind Text <Control-backslash> {%W tag remove sel 1.0 end}
tkTextClipboardKeysyms F16 F20 F18
bind Text <Insert> {catch {tkTextInsert %W [selection get -displayof %W]}}
bind Text <KeyPress> {tkTextInsert %W %A}
bind Text <Alt-KeyPress> {# nothing}
bind Text <Meta-KeyPress> {# nothing}
bind Text <Control-KeyPress> {# nothing}
bind Text <Escape> {# nothing}
bind Text <KP_Enter> {# nothing}

if !$tk_strictMotif {
    bind Text <Control-a> {tkTextSetCursor %W {insert linestart}}
    bind Text <Control-b> {tkTextSetCursor %W insert-1c}
    bind Text <Control-d> {%W delete insert}
    bind Text <Control-e> {tkTextSetCursor %W {insert lineend}}
    bind Text <Control-f> {tkTextSetCursor %W insert+1c}
    bind Text <Control-k> {
	if [%W compare insert == {insert lineend}] {
	    %W delete insert
	} else {
	    %W delete insert {insert lineend}
	}
    }
    bind Text <Control-n> {tkTextSetCursor %W [tkTextUpDownLine %W 1]}
    bind Text <Control-o> {
	%W insert insert \n
	%W mark set insert insert-1c
    }
    bind Text <Control-p> {tkTextSetCursor %W [tkTextUpDownLine %W -1]}
    bind Text <Control-t> {tkTextTranspose %W}
    bind Text <Control-v> {tkTextScrollPages %W 1}
    bind Text <Meta-b> {tkTextSetCursor %W {insert - 1c wordstart}}
    bind Text <Meta-d> {%W delete insert {insert wordend}}
    bind Text <Meta-f> {tkTextSetCursor %W {insert wordend}}
    bind Text <Meta-less> {tkTextSetCursor %W 1.0}
    bind Text <Meta-greater> {tkTextSetCursor %W end-1c}
    bind Text <Meta-BackSpace> {%W delete {insert -1c wordstart} insert}
    bind Text <Meta-Delete> {%W delete {insert -1c wordstart} insert}
    tkTextClipboardKeysyms Meta-w Control-w Control-y

    bind Text <Control-h> {
	if [%W compare insert != 1.0] {
	    %W delete insert-1c
	    %W see insert
	}
    }
    bind Text <2> {
	%W scan mark %x %y
	set tkPriv(x) %x
	set tkPriv(y) %y
	set tkPriv(mouseMoved) 0
    }
    bind Text <B2-Motion> {
	if {(%x != $tkPriv(x)) || (%y != $tkPriv(y))} {
	    set tkPriv(mouseMoved) 1
	}
	if $tkPriv(mouseMoved) {
	    %W scan dragto %x %y
	}
    }
    bind Text <ButtonRelease-2> {
	if !$tkPriv(mouseMoved) {
	    catch {
		%W insert @%x,%y [selection get -displayof %W]
	    }
	}
    }
}
