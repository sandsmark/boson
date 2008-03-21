<?php
/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
$filename="changelog-0.9.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.9 changelog");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson 0.9 changelog");

draw_bigbox_text("This is a list of bigger changes between
  <a href=\"boson-0.8.php\">Boson 0.8</a> and <a href=\"boson-0.9.php\">Boson
  0.9</a>.

  <h4>Internal</h4>
  <ul>
    <li>Units can be rendered as wireframes</li>
    <li>Rotation speed of units can be configured</li>
    <li>Bounding boxes of items can be rendered</li>
    <li>Shots split to subclasses</li>
    <li>Pathfinder is much faster now</li>
    <li>Separate process is used for audio</li>
    <li>OpenGL setting profiles (default/fastest/best quality)</li>
    <li>Redesigned start widgets</li>
    <li>Campaign support for the map selector</li>
    <li>Game and network logs can be saved</li>
  </ul>

  <h4>Rendering</h4>
  <ul>
    <li>New terrain rendering code. Terrain is now drawn using several layers
        which are blended together. This creates nice effect for transitions and
        looks really nice</li>
    <li>Added support for lights and halos using particles</li>
    <li>Added support for explosion fragments when unit is destroyed</li>
    <li>Materials are supported for models</li>
    <li>Models have normals now, meaning that everything should be lit correctly</li>
    <li>Better support for wind for particle systems</li>
    <li>New kind of smoke for wreckages</li>
    <li>Improved particle systems</li>
    <li>Weapon flashes when shooting</li>
    <li>Support for smooth surfaces</li>
    <li>Terrain normals (i.e. terrain is lit correctly)</li>
    <li>Experimental LOD support</li>
    <li>Much improved lighting support. Lighting is now enabled by default</li>
  </ul>


  <h4>Game</h4>
  <ul>
    <li>Acceleration/deceleration support for units</li>
    <li>Support for mines</li>
    <li>Support for bombs (basically a falling mine)</li>
    <li>New actions patch</li>
    <li>Basic scripting support</li>
    <li>Movies can be grabbed from running game (but it's extremely slow)</li>
    <li>Better wreckage handling (aircraft wreckages no more hang in the air and
        other ones slowly disappear into the ground)</li>
  </ul>

  <h4>Data</h4>
  <ul>
    <li>Fixed Puma model</li>
    <li>New Leopard model</li>
    <li>New unit: Koyote (a nice helicopter with a cannon)</li>
    <li>New unit: Warthog (A-10 plane with heavy anti-tank missiles)</li>
    <li>New unit: Wolf (a truck which may carry other units in the future)</li>
  </ul>

  <h4>Editor</h4>
  <ul>
    <li>You can now add multiple units and draw multiple tiles in editor using
        click+drag</li>
    <li>Height maps can be exported</li>
    <li>Texture maps can be exported/imported in editor</li>
  </ul>");

draw_bigbox_end();

main_area_end();

end_page();


?>
