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
draw_bigbox_text("Most recent version, 0.9.1, was released on 16th November 2003.<br>
  Next version will be 0.10, but release date it not yet set.
  <br><br>");

//draw_bigbox_subheader("<a name=\"features\"></a>Feature list");

/*draw_bigbox_text("What has been changed since 0.9 (will go into 0.9.1):
<ul>
 <li>Network with kdegames from KDE < 3.2 is fixed</li>
 <li>Minor file format fix, that causes less trouble with network</li>
 <li>Ground texture pixmaps are loaded correctly in the editor</li>
 <li>Factory productions are saved correctly</li>
 <li>Try to find a usable GL font, even if Qt doesn't find any</li>
 <li>Don't crash when using proprietary NVidia drivers and no usable font was found (reported to NVidia nearly a year ago)</li>
</ul>");*/
/*
draw_bigbox_text("This is a list of features that we consider as
  &quot;to-be-done&quot; for the next release.
  <h2>TODO</h2>
  <ul>
    <li>Fixing all critical bugs</li>
  </ul>
  <h2><a name=\"done\"></a>DONE</h2>

  <h4>Internal</h4>
  <ul>
  </ul>

  <h4>Rendering</h4>
  <ul>
  </ul>


  <h4>Game</h4>
  <ul>
  </ul>

  <h4>Data</h4>
  <ul>
  </ul>

  <h4>Editor</h4>
  <ul>
  </ul>");
*/
draw_bigbox_end();
main_area_end();

end_page();


?>
