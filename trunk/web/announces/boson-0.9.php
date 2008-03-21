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
$filename="boson-0.9.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.9 announcement");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Announcement
draw_bigbox_begin("Boson 0.9 announcement");

draw_bigbox_text("
<p class=\"announcement\">
Release of Boson 0.9, a Real Time Strategy (RTS) game for the K Desktop
Environment.
</p>

<p class=\"announcement\">
The last release of Boson (0.8) was more than half a year ago, so we cannot list
all of the changes here. However we will try to present the most interesting
ones.
</p>

<p class=\"announcement\">
Two movies of Boson running got released - see
<a href=\"http://boson.eu.org/movies.php\">http://boson.eu.org/movies.php</a>
These movies were made with the new movie grabbing feature. It is very (!) slow,
but in conjunction with the powerful new scripting features, you can make really
nice movies now.<br>
These scripting features are the very first step on the long road towards AI.
</p>

<p class=\"announcement\">
Lighting support got added and can make the scene look a lot nicer. Lighting is
somewhat experimental for the models, but terrain lighting looks very nice
already.
</p>

<p class=\"announcement\">
The terrain rendering code has been completely rewritten. As a result we have
very nice transitions now, and the texture data size got reduced by many
MegaBytes.
</p>

<p class=\"announcement\">
Here is a (very) short list of the major (!) changes since
<a href=\"boson-0.8.php\">Boson 0.8</a>:<br>
- Basic scripting features<br>
- Movie grabbing possible (but _extremely_ slow)<br>
- New terrain rendering code<br>
- Lighting support for units and especially terrain<br>
- Experimental level of detail<br>
- New file format for savegames and playfields. We use a unified file format
  here, which removes a _lot_ of duplicated code (read: less bugs)<br>
- Sound should be usable again. As it is mostly untested it is disbled by
  default. You can use --sound on command line to enable it.<br>
- Faster pathfinding<br>
- A few new units/models<br>
You can see a longer, more complete list of changes at
<a href=\"http://boson.eu.org/announces/changelog-0.9.php\">http://boson.eu.org/announces/changelog-0.9.php</a>
</p>


<p class=\"announcement\">
Requirements<br>
You can find a list of hardware and software requirements at
<a href=\"http://boson.eu.org/info.php\">http://boson.eu.org/info.php</a><br>
Note that proper 3d acceleration is required.
</p>

<p class=\"announcement\">
Downloading and installing Boson<br>
Source packages can be found at
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=194601\">http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=194601</a>
</p>

<p class=\"announcement\">
Installation instructions are at
<a href=\"http://boson.eu.org/install.php\">http://boson.eu.org/install.php</a>
</p>


<p class=\"announcement\">
The Boson Team<br>
&nbsp;<a href=\"http://boson.eu.org\">http://boson.eu.org</a>
</p>
");

draw_bigbox_end();

main_area_end();

end_page();


?>
