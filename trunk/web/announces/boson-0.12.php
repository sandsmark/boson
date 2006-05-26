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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


/*****  Variables  *****/
$filename="boson-0.12.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.12 announcement");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Announcement
draw_bigbox_begin("Boson 0.12 announcement");

draw_bigbox_text("
<p class=\"announcement\">
Release of Boson 0.12, a Real Time Strategy (RTS) game for the K Desktop
Environment.
</p>

<p class=\"announcement\">
A lot has changed since the last release - so much that we can't even remember most of it anymore. In particular Boson has moved away from CVS to subversion and away from autotools to cmake. The cmake change will probably cause some trouble in the beginning, as it completely replaces our build system. However it will make many things a lot easier - the autotools version could barely be maintained anymore.
</p>

<p class=\"announcement\">
<b>A few highlights:</b><br>
* Shadows. Units and terrain can now cast and receive shadows. This feature uses shaders, so a pretty recent videocard supporting DirectX 9 is required (NVidia GeForce 6xxx series or ATI Radeon X series card is recommended).<br>
* Radars. A completely new, much more realistic radar system has been implemented that allows for several independent radar stations. Even different radars for different unit types (land and air) are supported <br>
* Ammunition. Units can make use of ammunition now. Most units have an infinite amount of ammunition, but the feature is there. This sets the grounds for several interesting possible unit types. <br>
* Power Plants. The power plant unit has been in Boson for a very long time - now they actually generate power! If a player has insufficient power, several tasks (such as building new units) take more time.<br>
Note that many core features are still in development and thus Boson is still mostly unplayable.<br>
<br>
For a longer, though still very incomplete list see<br>
<a href=\"http://boson.eu.org/announces/changelog-0.12.php\">http://boson.eu.org/announces/changelog-0.12.php</a>
</p>

<p class=\"announcement\">
<b>Requirements</b><br>
You can find a list of hardware and software requirements at <a href=\"http://boson.eu.org/info.php\">http://boson.eu.org/info.php</a><br>
Note that proper 3d acceleration is required.
</p>

<p class=\"announcement\">
<b>Downloading and installing Boson</b><br>
Source packages can be found at<br>
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=419522 \">http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=419522</a><br>
Due to the new cmake based build system the installation instructions completely differ from previous releases. Also note that because of the cmake transition we do not provide static package for this release.<br>
<br>
First of all you need cmake 2.4.2, see <a href=\"http://www.cmake.org/files/v2.4/\">http://www.cmake.org/files/v2.4/</a><br>
Then download boson-all-0.12.tar.bz2 and follow these instructions:<br>
$ tar xjvf boson-all-0.12.tar.bz2<br>
$ cd boson-all-0.12<br>
$ mkdir build<br>
$ cd build<br>
$ cmake ..<br>
$ make<br>
$ su<br>
# make install<br>
<br>
If you want to install at a different prefix, use 'cmake -DCMAKE_INSTALL_PREFIX=/install/boson/to ..' instead of the 'cmake ..' line.<br>
Installation instructions can also be found at<br>
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
