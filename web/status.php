<?php
/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
$filename="status.php";

/*****  Some includes  *****/
include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");

/*****  Start of main stuff  *****/

start_page("Status");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Status");
draw_bigbox_text("Most recent version, 0.11, was released on 2nd September 2005.
  <br><br>");

draw_bigbox_subheader("<a name=\"features\"></a>Feature list");

draw_bigbox_text("What has been changed since 0.11:
<ul>
  <li>Major PlayerId redesign</li>
  <li>Several bugfixes</li>
  <li>Visual feedbacks when giving orders to units</li>
  <li>Provide ALT+number to center on a selection</li>
  <li>Show all production options in the commandframe, even those with unmet
  requirements (grayed out then)</li>
</ul>");

draw_bigbox_end();
main_area_end();

end_page();


?>
