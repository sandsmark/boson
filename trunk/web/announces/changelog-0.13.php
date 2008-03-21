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
$filename="changelog-0.13.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.13 changelog");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson 0.13 changelog");

draw_bigbox_text("This is a list of bigger changes between
  <a href=\"boson-0.12.php\">Boson 0.12</a> and <a href=\"boson-0.13.php\">Boson
  0.13</a>.

<ul>
  <li>New fog of war system</li>
  <li>Fixed problem with shaders and teamcolor, made textured and teamcolored meshes possible</li>
  <li><span class='wikiword'>BosonConfig</span> scripts</li>
  <li>Unit models are improved by recalculating normals</li>
  <li>Fixed bug with player color display in start game page</li>
  <li>Ground/unit shaders can be toggled on/off independently</li>
  <li>Added map/savegame converter program</li>
  <li>Distant units are shown as icons</li>
  <li>Improved detection of Python libraries</li>
  <li>Bullet trails work again</li>
  <li>Map preview for the newgame widget</li>
  <li>Improved detection of KDE libraries</li>
  <li>Missiles now continue moving towards destroyed targets</li>
  <li>Fixed minimap rendering</li>
  <li>Internal redesign of minimap code</li>
  <li>Several bugfixes/imporvements to cmdframe's production buttons</li>
  <li>Game can now be started with explored map</li>
  <li>Display list of requirements for a production</li>
  <li>Mouse wheel zooms minimap</li>
  <li>Fixed/improved KDE/Python/... detection by the cmake build system</li>
  <li>New (internal) architecture for unit orders</li>
  <li>Rendering speed improvements</li>
  <li>Radar jammers</li>
  <li>Some licensing issues resolved</li>
  <li>Improved airport model</li>
  <li>some gcc 4.1 and 4.2 fixes</li>
  <li>Removed a lot of debug output</li>
  <li>Workaround for broken MESA based OpenGL drivers</li>
  <li>Fix libGL.so loading: it must be loaded with RTLD_GLOBAL flag (which QLibrary can't do), so we use our own code for dlopen()ing libGL.so now</li>
  <li>Don't install some debugging application</li>
</ul>

");

draw_bigbox_end();
main_area_end();

end_page();


?>
