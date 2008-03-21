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
$filename="boson-0.13.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.13 announcement");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Announcement
draw_bigbox_begin("Boson 0.13 announcement");

draw_bigbox_text("
<p class=\"announcement\">
Release of Boson 0.13, a Real Time Strategy (RTS) game for the K Desktop
Environment.
</p>

<p class=\"announcement\">
Most of the work since the last release has been on behind the scenes
features, such as fixing various issues with the new cmake based build system
(which has made our life a lot easier) and a lot of small code improvements.
Also the groundwork of a few new features has been layed that will get used
in a later release, but are not yet supported by the game. An example here is
code support for radar jammers.
</p>

<p class=\"announcement\">
The actual user-visible highlights of this release:<br>
* New fog of war. In addition to the original fog of war that covers the whole
terrain until it has been explored by a unit, we also have \"fog\" that covers
all units that are outside the players' sight range.<br>
* Icons for distant units: Units that are very far away from the camera
position are replaced by a constant-size icon showing unit's type and it's
owner's status toward you (friendly/neutral/enemy). This makes it possible to
still recognize units that would otherwise be a small dot on the screen only.
</p>

<p class=\"announcement\">
Note that many core features are still in development and thus Boson is still
mostly unplayable.
</p>

<p class=\"announcement\">
For a longer, though still very incomplete list see<br>
 <a href=\"http://boson.eu.org/announces/changelog-0.13.php\">http://boson.eu.org/announces/changelog-0.13.php</a>
</p>

<p class=\"announcement\">
<b>Requirements</b><br>
You can find a list of hardware and software requirements at<br>
 <a href=\"http://boson.eu.org/info.php\">http://boson.eu.org/info.php</a><br>
Note that proper 3d acceleration is required.
</p>

<p class=\"announcement\">
<b>Downloading Boson</b><br>
Source packages can be found at<br>
 <a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=451731\">http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=451731</a>
</p>

<p class=\"announcement\">
<b>Installation instructions</b><br>
First of all you need at least cmake 2.4.2, see
 <a href=\"http://www.cmake.org/files/v2.4/\">http://www.cmake.org/files/v2.4/</a><br>
Then download boson-all-0.13.tar.bz2 and follow these instructions: <br>
  tar xjvf boson-all-0.13.tar.bz2<br>
  cd boson-all-0.13<br>
  mkdir build<br>
  cd build<br>
  cmake ..<br>
  make<br>
  su<br>
  make install<br>
</p>

<p class=\"announcement\">
If you want to install at a different prefix,
use \"cmake -DCMAKE_INSTALL_PREFIX=/install/boson/to ..\" instead of
the \"cmake ..\" line.<br>
Installation instructions can also be found at<br>
 <a href=\"http://boson.eu.org/install.php\">http://boson.eu.org/install.php</a>
</p>

<p class=\"announcement\">
The Boson Team <br>
 http://boson.eu.org
</p>
");

draw_bigbox_end();

main_area_end();

end_page();


?>
