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
include("common.php");
include("sidebar.php");
include("main.php");
include("counter.php");
include("boxes.php");
include("variables.php");

/*****  Start of main stuff  *****/

do_start_stuff();

// Headers
html_print_header("Status");
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
draw_bigbox_begin("Status");
draw_bigbox_text("Boson is currently still under heavy development.<br><br>
  We're heading for release of version 0.8, but it is currently
  unclear when it will be released.<br><br>");

draw_bigbox_subheader("<a name=\"features\"></a>Feature list");
draw_bigbox_text("This is a list of features that we consider as
  &quot;to-be-done&quot; for the next release.
  <h2>TODO</h2>
  <ul>
    <li>Support for height maps in editor</li>
    <li>3d-terrains in map files</li>
    <li>New startup widgets</li>
    <li>New save game format</li>
  </ul>
  <h2><a name=\"done\"></a>DONE</h2>
  <blockquote>
  <h4>Internal</h4>
  <ul>
    <li>Height map for 3d-terrain</li>
    <li>New map file format (.tar.gz instead of .gz)</li>
    <li>Preload map information on startup</li>
    <li>Decrease size of textures (fixes problems for 24bpp and 32bpp)</li>
    <li>Don't draw invisible particles</li>
    <li>Particles are drawn depth-sorted</li>
    <li>Big performance improvements</li>
    <li>Binary map format (decreases size of map files a lot, especially for big maps)</li>
  </ul>
  <h4>Game</h4>
  <ul>
    <li>Technology upgrades are working now and were improved</li>
    <li>Added minimap image for disabled minimap</li>
    <li>Weapon specific sound support</li>
    <li>Smoke support for powerplant, oilrefinery, mineral refinery etc.</li>
    <li>Particle system improvements</li>
    <li>Damaged units are getting darker</li>
    <li>Save screenshots as jpg and show the full path as a chat message</li>
    <li>New minimap zooming buttons</li>
    <li>Fixed texture coordinates bug (meaning: it looks better)</li>
  </ul>
  <h4>Data</h4>
  <ul>
    <li>New unit: Ferry</li>
    <li>Unit models improvements</li>
    <li>Warfactory is now Weapons Factory</li>
    <li>Handbook switched from png to jpg images</li>
  </ul>
  <h4>Editor</h4>
  <ul>
    <li>Many bugs are fixed</li>
    <li>Buildings are placed in constructed mode</li>
    <li>Better support for big maps</li>
    <li>Support for changing map name and description</li>
  </ul>
  
  
  
  </blockquote>
  ");
main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
