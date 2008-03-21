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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


/*****  Variables  *****/
$filename="stories.php";

/*****  Some includes  *****/
include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson stories");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson stories");
draw_bigbox_text("Boson stories:
    <ul>
    <li><a href=\"stories/story-20021017.php\">Story #1&nbsp;&nbsp;(Oct. 17, 2002)</a></li>
    <li><a href=\"stories/story-20021108.php\">Story #2&nbsp;&nbsp;(Nov. 08, 2002)</a></li>
    <li><a href=\"stories/story-20030331.php\">Story #3&nbsp;&nbsp;(March 31, 2003)</a></li>
    </ul>");
draw_bigbox_end();

main_area_end();

end_page();


?>
