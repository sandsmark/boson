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
draw_bigbox_text("Most recent version, 0.10, was released on 2nd May 2004.
  <br><br>");

draw_bigbox_subheader("<a name=\"features\"></a>Feature list");

draw_bigbox_text("What has been changed since 0.10:
<ul>
 <h3>General</h3>
 <ul>
    <li>Start of porting boson to plib (far from being complete !)</li>
    <li>Added a network sync protocol, now it is possible to <i>check</i> if the clients are in sync</li>
    <li>Improvements to effects system (load/save, species independent, delaying)</li>
 </ul>

<!-- <h3>Sound</h3>
 <ul>
     <li></li>
</ul> -->

 <h3>Rendering</h3>
 <ul>
    <li>Basic support for animated water (lakes and oceans are possible), not included in editor yet</li>
    <li>Many improvements for textured fonts (wrapping, colors, blending)</li>
    <li>Effects for the effects system introduced in 0.10 (bullet trail, lights)</li>
 </ul>

 <h3>Game</h3>
 <ul>
    <li>Support for winning conditions</li>
 </ul>

<!-- <h3>Scripts / AI</h3>
 <ul>
    <li></li>
</ul> -->

 <h3>Editor</h3>
 <ul>
     <li>Basic widget to edit conditions</li>
 </ul>

 <h3>Data</h3>
 <ul>
     <li>Added textured fonts from plib</li>
 </ul>

 <h3>Internal</h3>
 <ul>
    <li>Default font is textured</li>
    <li>Added a memory manager for debugging</li>
 </ul>



</ul>");

draw_bigbox_text("This is a list of features that we consider as
  &quot;to-be-done&quot; for the next release.
  <h2>TODO</h2>
 ");
draw_bigbox_end();
main_area_end();

end_page();


?>
