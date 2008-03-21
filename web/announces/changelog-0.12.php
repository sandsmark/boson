<?php
/*
    This file is part of the Boson game
    Copyright (C) 2006 The Boson Team (boson-devel@lists.sourceforge.net)

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
$filename="changelog-0.12.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.12 changelog");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson 0.12 changelog");

draw_bigbox_text("This is a list of bigger changes between
  <a href=\"boson-0.11.php\">Boson 0.11</a> and <a href=\"boson-0.12.php\">Boson
  0.12</a>.

<ul>
  <li>Major PlayerId redesign </li>
  <li>Several bugfixes </li>
  <li>Visual feedbacks when giving orders to units </li>
  <li>Provide ALT+number to center on a selection </li>
  <li>Show all production options in the commandframe, even those with unmet requirements (grayed out then) </li>
  <li>Powerplants now actually plant power </li>
  <li>Shadow support </li>
  <li>Turrets turn towards their target </li>
  <li>Weapons can use ammunition </li>
  <li>New radar system </li>
  <li>New method to find the 3d-position of the mouse position. This fixes problems with some broken OpenGL drivers and also fixes a performance issue </li>
  <li>A completely new species with its own models </li>
  <li>autotools replaced by cmake </li>
  <li>cvs replaced by subversion</li>
</ul>

");

draw_bigbox_end();
main_area_end();

end_page();


?>
