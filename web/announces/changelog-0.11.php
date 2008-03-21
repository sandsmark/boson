<?php
/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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
$filename="changelog-0.11.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.11 changelog");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson 0.11 changelog");

draw_bigbox_text("This is a list of bigger changes between
  <a href=\"boson-0.10.php\">Boson 0.10</a> and <a href=\"boson-0.11.php\">Boson
  0.11</a>.

<ul>
 <h3>General</h3>
 <ul>
    <li>Port to <a href=\"http://libufo.sourceforge.net\">libufo</a> </li>
    <li>Added a network sync protocol, now it is possible to <i>check</i> if the
    clients are in sync. Additionally most sync errors can now be fixed.</li>
    <li>Improvements to effects system (load/save, species independent, delaying)</li>
    <li>Improvements to weapons</li>
    <li>Very simple and experimental dedicated server</li>
 </ul>

<!-- <h3>Sound</h3>
 <ul>
     <li></li>
</ul> -->

 <h3>Rendering</h3>
 <ul>
    <li>Basic support for animated water (lakes and oceans are possible), not included in editor yet</li>
    <li>Simple terrain LOD (level of detail) system</li>
    <li>Light effect (dynamic lights)</li>
    <li>Daylight effect, realistically simulating Sun's movement</li>
    <li>Wind effect</li>
    <li>Bullet trail effect</li>
    <li>Environmental effects (can be used to create snow, rain, etc)</li>
    <li>Improved and much better-looking fog-of-war rendering for terrain</li>
    <li>Support for anisotropic filtering for textures (better quality)</li>
    <li>Support for water rendering using shaders</li>
 </ul>

 <h3>Game</h3>
 <ul>
    <li>Support for winning conditions</li>
    <li>Forbid placing refineries too close to resource mines</li>
    <li>Skip rendering some frames when the game becomes overloaded. This
    reserves more CPU power for the actual game tasks and keeps it playable</li>
    <li>Descriptions for produceable facilities are now displayed in the game</li>
    <li>Flying units have their own moving quirks</li>
    <li>New Pathfinder</li>
    <li>Bomb-dropping is more realistic</li>
    <li>Description widget, showing info about highlighted unit type when a factory is selected</li>
 </ul>

 <h3>Scripts / AI</h3>
 <ul>
    <li>Events support for scripts</li>
    <li>Loading/saving for scripts</li>
    <li>Many new script functions</li>
 </ul>

 <h3>Editor</h3>
 <ul>
     <li>Basic widget to edit conditions</li>
     <li>Improvements to unit placing</li>
     <li>Support for undo/redo of unit placement/deletion</li>
 </ul>

<h3>Data</h3>
<ul>
  <li>Added textured fonts from plib (not functional right now, due to the
      libufo port)</li>
  <li>Many new neutral models (trees, houses, etc)</li>
  <li>Few new maps (Cross)</li>
  <li>Scripts are now in data module</li>
  <li>Some new effects</li>
  <li>Sight and weapon ranges of units are much bigger now</li>
  <li>Some new cursor themes</li>
</ul>

 <h3>Internal</h3>
 <ul>
    <li>New texture class and texture manager</li>
    <li>Canvas coordinates are now same as cell coordinates (except that
        they're floats, not ints)</li>
    <li>Use new bofixed (fixed-precision number) data type in logic code to
        prevent errors caused by rounding of floats</li>
    <li>Unit speeds are now saved as cells/second in confg files. This makes
        editing the files much easier</li>
    <li>Weapon types are now saved as strings instead of ints in config files,
        making them more readable</li>
    <li>New profiling architecture</li>
    <li>Adding a cache file format for model files (decreases startup times)</li>
    <li>Adding an internal math library</li>
    <li>Load libGL.so dynamically on startup using dlopen()</li>
    <li>Compress map data before sending it over network</li>
    <li>Game Event subsystem</li>
    <li>Making game starting more modular</li>
    <li>bosonwidget.cpp has been removed. It has seen 218 HEAD revisions and
    some more from branches. bosonwidget.cpp, 2001/11/09 - 2004/12/10, may it
    rest in peace.</li>
    <li>Complete game logic and game view separation</li>
    <li>New upgrade architecture</li>
 </ul>



</ul>");

draw_bigbox_end();
main_area_end();

end_page();


?>
