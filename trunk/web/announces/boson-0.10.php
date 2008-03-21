<?php
/*
    This file is part of the Boson game
    Copyright (C) 2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
$filename="boson-0.10.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.10 announcement");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Announcement
draw_bigbox_begin("Boson 0.10 announcement");

draw_bigbox_text("
<p class=\"announcement\">
Release of Boson 0.10, a Real Time Strategy (RTS) game for the K Desktop
Environment.
</p>

<p class=\"announcement\">
The main focus of this release are dependencies. Boson does not depend on
arts/kdemultimedia and kdegames/libkdegames anymore, instead boson requires
OpenAL for sound. You can find a hopefully complete list of the dependencies at
<a href=\"http://boson.eu.org/info.php\">http://boson.eu.org/info.php</a>
</p>

<p class=\"announcement\">
Another core feature is improved level of detail code. You can look at a large
area of the map containign many units without significant speed loss now. Also
there are many other performance related improvements in this release.
</p>

<p class=\"announcement\">
On a non technical side the most important feature is probably the new
harvesting code. Harvesting both, minerals and oil, is now supported. We
believe this is a big step to make boson fully playable.
</p>

<p class=\"announcement\">
Last but not least there are ongoing improvements on our AI code. The computer
players have now the ability to produce units.
</p>


<p class=\"announcement\">
The most important changes in this version:<br>
 - libkdegames is included in this release. You do not need to have it
   installed anymore.<br>
 - Arts is not used anymore, OpenAL is used for sound instead, fixing many
   problems.<br>
 - Mineral and oil harvesting is supported<br>
 - Many performance improvements<br>
 - Improved pathfinding<br>
 - Neutral player added<br>
 - Elementary AI<br>
 - Huge amount of bugfixes<br>
</p>

<p class=\"announcement\">
You can see a longer, more complete list of changes at
 <a href=\"http://boson.eu.org/announces/changelog-0.10.php\">http://boson.eu.org/announces/changelog-0.10.php</a>
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
 <a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=235412\">http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=235412</a>
</p>

<p class=\"announcement\">
Installation instructions are at
 <a href=\"http://boson.eu.org/install.php\">http://boson.eu.org/install.php</a>
</p>


<p class=\"announcement\">
The Boson Team<br>
  http://boson.eu.org
</p>
");

draw_bigbox_end();

main_area_end();

end_page();


?>
