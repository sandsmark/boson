<?php
/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


/*****  Variables  *****/
$filename="contact.php";

/*****  Some includes  *****/
include("common.php");
include("sidebar.php");
include("main.php");
include("counter.php");
include("boxes.php");
include("variables.php");

/*****  Start of main stuff  *****/

do_start_stuff();

// Headers
html_print_header("Contact us");
print_header();

// Main table
main_table_begin();

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Contact us");
draw_bigbox_subheader("<a name=\"mail\"></a>Mailing lists");
draw_bigbox_text("There are three mailing lists.<br><br>
    <a href=\"mailto:boson-devel@lists.sourceforge.net\">boson-devel@lists.sourceforge.net</a>
    is meant for users and developers. If you have questions or problems, you can always come there and ask.
    Note that you have to subscribe or you message needs some days to reach the list. Also include a note
    if you aren't on list so the other users can CC you.<br>
    <br>
    boson-cvs@lists.sourceforge.net is only meant for CVS logs, please do not post there.<br>
    <br>
    boson-bugs@lists.sourceforge.net is also not meant for posting, bug reports are sent there.
    See below on how to report bugs.
    <br><br>
    You can subscribe and unsubscribe them
    <a href=\"http://sourceforge.net/mail/?group_id=15087\" target=\"_blank\"> here.</a><br><br>");

draw_bigbox_subheader("IRC");
draw_bigbox_text("We also have an IRC channel. If you have questions/problems/compile errors,
    you can just go to #boson on openprojects (e.g. irc.kde.org).<br><br>");

draw_bigbox_subheader("<a name=\"bugs\"></a>Reporting bugs");
draw_bigbox_text("You can report bugs using <a href=\"http://bugs.kde.org/\">KDE's bug tracking system</a>
    or the 'report bug' feature in boson's help menu.");

draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
