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
  Next version, which should be released in one or two weeks will be 0.10.
  <br><br>");

//draw_bigbox_subheader("<a name=\"features\"></a>Feature list");

draw_bigbox_text("What has been changed since 0.9.1:
<ul>
 <h3>General</h3>
 <ul>
    <li>Resolution changing using Xrandr is supported (XFree 4.3 required)</li>
    <li>Dependency changes:</li>
    <ul>
    <li>Added: <a href=\"http://www.openal.org/\" target=\"_blank\">OpenAL</a></li>
    <li>Removed: arts, kdemultimedia, kdegames</li>
    </ul>
 </ul>

 <h3>Sound</h3>
 <ul>
     <li>Sound is played using OpenAL now</li>
     <li>Dependency on arts and kdemultimedia removed</li>
     <li>Sound is enabled by default again</li>
</ul>

 <h3>Rendering</h3>
 <ul>
    <li>Use vertex arrays and vertex buffer objects by default if possible,
    increases speed by a few FPS</li>
    <li>First steps for transparent textures (transparent surfaces support)</li>
    <li>New effects system which replaces particle systems. At the moment this is an
    internal change only</li>
    <li>Units are correctly rotated, depending on the slope of the terrain that
    they're on</li>
    <li>Performance improvements</li>
 </ul>

 <h3>Game</h3>
 <ul>
    <li>OpenGL minimap</li>
    <li>New pathfinding code</li>
    <li>Special pathfinding code for flying units</li>
    <li>The game starts only once all clients have loaded their game data</li>
    <li>Neutral player and species has been added. You cannot play this, it is
    meant for civilian units or things like trees and so on</li>
    <li>Support for oil harvesting. Mineral harvesting is not yet supported due to
    the lack of a mineral mine model</li>
    <li>Oil tower is a neutral unit now</li>
    <li>Day/night support, the default AI script now triggers day/night changes</li>
    <li>Flying units can't fly on unpassable cells (slope > 45 degrees)</li>
    <li>Take unit's size into account when calculating unit's distance from explosions</li>
 </ul>

 <h3>Scripts / AI</h3>
 <ul>
    <li>AI now produces units, thanks to Carlo (aka alea)</li>
    <li>New ai script functions</li>
    <li>Free mode and no-limits mode for camera (to get used by scripts)</li>
</ul>

 <h3>Editor</h3>
 <ul>
     <li>Change height icon</li>
     <li>Bugfixes</li>
 </ul>

 <h3>Data</h3>
 <ul>
     <li>Some neutral objects added (mineralmine, trees, stone, house...)</li>
     <li>New explosions</li>
     <li>Two textured fonts added</li>
     <li>Scaled down a texture</li>
     <li>General map changes</li>
     <li>New map: Canyon</li>
     <li>New map: We are under attack</li>
     <li>Documentation updates</li>
 </ul>

 <h3>Internal</h3>
 <ul>
    <li>Very basic support for ac3d models</li>
    <li>Support for textured fonts. They are faster, more powerful and stable.
    Thanks a lot to <a href=\"http://plib.sf.net\" target=\"_blank\">plib</a> where this code has
    been shamelessy stolen</li>
    <li>Texture compression is used, if available which can reduce texture size an
    therefore improve speed</li>
    <li>The path a unit is using is displayed when it is selected</li>
    <li>The menu is properly removed when a game is ended, fixing menu duplication
    (see <a href=\"http://bugs.kde.org/show_bug.cgi?id=66715\" target=\"_blank\">66715</a>)</li>
    <li>Boson now provides it's own copy of kgame</li>
    <li><a href=\"http://boson.halux2001.de/handbook/en/faq.html#gen_3\" target=\"_blank\">Emergency save</a> added, it saves all game data. A developer can now look at a replay of this session</li>
    <li>Gcc 3.4 fixes</li>
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
