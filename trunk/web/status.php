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
draw_bigbox_text("Boson is currently still under heavy development.<br>
  We're currently heading for Boson 0.9, which will probably be released later
  this year.<br>
  <a href=\"download.php#cvs\">CVS version</a> already contains many new
  features, see below.<br><br>");

draw_bigbox_subheader("<a name=\"features\"></a>Feature list");

draw_bigbox_text("This is a list of features that we consider as
  &quot;to-be-done&quot; for the next release.
  <h2>TODO</h2>
  <ul>
    <li>Basic scripting support</li>
  </ul>
  <h2><a name=\"done\"></a>DONE</h2>
  <blockquote>
  <ul>
    <li>Units can be rendered as wireframes</li>
    <li>Acceleration/deceleration support for units</li>
    <li>Units accelerate/decelerate realistically</li>
    <li>Rotation speed of units can be configured</li>
    <li>Shots split to subclasses</li>
    <li>Support for mines</li>
    <li>Support for bombs (basically a falling mine)</li>
    <li>Bounding boxes of items can be rendered<li>
    <li>New terrain rendering code. Terrain is now drawn using several layers
        which are blended together. This creates nice effect for transitions and
        looks really nice</li>
    <li>Fixed Puma model</li>
    <li>New unit: Koyote (a nice helicopter with a cannon)</li>
    <li>New Leopard model</li>
    <li>New unit: Warthog (A-10 plane with heavy anti-tank missiles)</li>
    <li>New unit: Wolf (a truck which may carry other units in the future)</li>
    <li>New actions patch</li>
    <li>You can now add multiple units and draw multiple tiles in editor using
        click+drag</li>
    <li>Height maps can be exported</li>
    <li>Added supports for light and halos using particles</li>
    <li>Ground textures are drawn over multiple cells, making tiling effect less
        visible</li>
    <li>Added support for explosion fragments when unit is destroyed</li>
    <li>Texture maps can be exported/imported in editor</li>
    <li>Materials are supported for models</li>
    <li>Pathfinder is much faster now</li>
    <li>New kind of smoke for wreckages</li>
    <li>Models have normals now, meaning that everything should be lit correctly</li>
    <li>Separate process is used for audio</li>
    <li>Better support for wind for particle systems</li>
    <li>Basic scripting support</li>
    <li>OpenGL setting profiles (default/fastest/best quality)</li>
    <li></li>
  </ul>
  </blockquote>");
draw_bigbox_end();
main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
