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
$filename="boson-0.9.1.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.9.1 announcement");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Announcement
draw_bigbox_begin("Boson 0.9.1 announcement");

draw_bigbox_text("
<p class=\"announcement\">
Release of Boson 0.9.1, a Real Time Strategy (RTS) game for the K Desktop
Environment.
</p>

<p class=\"announcement\">
This is a bugfix release, fixing a couple of important problems in boson 0.9
</p>

<p class=\"announcement\">
Both the data and the music packages are unchanged, so if you already
downloaded and installed them (or maybe the -all package) you need the code 
package only this time.
</p>

<p class=\"announcement\">
Nearly complete changelog (since <a href=\"boson-0.9.php\">Boson 0.9</a>):<br>
- Network with kdegames from KDE < 3.2 is fixed<br>
- Minor file format fix, that causes less trouble with network<br>
- Ground texture pixmaps are loaded correctly in the editor<br>
- Factory productions are saved correctly<br>
- Try to find a usable GL font, even if Qt doesn't find any<br>
- Don't crash when using proprietary NVidia drivers and no usable font was
found (reported to NVidia nearly a year ago)<br>
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
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=197716\">http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=197716</a><br>
Installation instructions are at
http://boson.eu.org/install.php
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
