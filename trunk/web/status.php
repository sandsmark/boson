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
draw_bigbox_text("Most recent version, 0.9.1, was released on 16th November 2003.<br>
  Next version will be 0.10, but release date it not yet set.
  <br><br>");

//draw_bigbox_subheader("<a name=\"features\"></a>Feature list");

draw_bigbox_text("What has been changed since 0.9.1:
<ul>
 <li>Support for textured fonts. They are faster and more powerful, but most
 important: not even proprietary NVidia drivers crash when they are invalid.
 Thanks a lot to <a href=\"http://plib.sf.net\">plib</a> where this code has
 been shamelessy stolen</li>
 <li>Neutral player and species has been added. You cannot play this, it is
 meant for civilian units or things like trees and so on.</li>
 <li>Texture compression is used, if available which can reduce texture size an
 therefore improve speed.</li>
 <li>OpenGL minimap</li>
 <li>The path a unit is using is displayed when it is selected</li>
 <li>The menu is properly removed when a game is ended, fixing menu duplication
 (see <a href=\"http://bugs.kde.org/show_bug.cgi?id=66715\">66715</a>)</li>
 <li>The game starts only once all clients have loaded their game data</li>
 <li>Units are correctly rotated, depending on the slope of the terrain that
 they're on</li>
 <li>New pathfinding code</li>
 <li>Resolution changing using Xrandr supported</li>
 <li>Use vertex arrays and vertex buffer objects by default if possible. This
 increases speed by a few FPS.</li>
 <li>Sound is played using OpenAL now. Dependancy on arts and kdemultimedia
 removed.</li>
 <li>Sound is enabled by default again</li>
</ul>");

draw_bigbox_text("This is a list of features that we consider as
  &quot;to-be-done&quot; for the next release.
  <h2>TODO</h2>
  <ul>
    <li>Convert playfields in data module to use the new file format without
    conversion on startup</li>
    <li>Find out whether save/load is still working</li>
    <li>Find out whether playfield saving is still working</li>
  </ul>");
draw_bigbox_end();
main_area_end();

end_page();


?>
