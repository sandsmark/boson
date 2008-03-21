<?php
/*
    This file is part of the Boson game
    Copyright (C) 2005 The Boson Team (boson-devel@lists.sourceforge.net)

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
$filename="boson-0.11.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.11 announcement");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Announcement
draw_bigbox_begin("Boson 0.11 announcement");

draw_bigbox_text("
<p class=\"announcement\">
Release of Boson 0.11, a Real Time Strategy (RTS) game for the K Desktop
Environment.
</p>

<p class=\"announcement\">
For this release, the primary focus has been to finally get the release
out ;-)<br>
For nearly the whole time since the last release in early 2004 both main Boson
developers have been too busy with various non-Boson things to concentrate
for long enough on an actual release. However during that time period new
features got constantly added, so the list of changes is very long for this
version.
</p>

<p class=\"announcement\">
Probably the most visible change is the port to libufo.
libufo (<a href=\"http://libufo.sf.net\">http://libufo.sf.net</a>) is a widget toolkit for OpenGL, that enables
Boson to draw buttons, labels and other widgets directly into the OpenGL
scene, instead of using a separated area. An installation of libufo is not
necessary, as Boson provides its own copy of this library.
Note that we still use both, KDE and Qt and will continue to do so. KDE and Qt
are used in Boson for a lot of things, not only the GUI.
</p>

<p class=\"announcement\">
Another much apparent change is the implementation of realistic looking water.
While Boson 0.10 used simply textures only, this release provides a dedicated
water engine, even supporting shaders and reflections.
</p>

<p class=\"announcement\">
In the technical area, Boson has received an game-event subsystem that makes
the game more modular, event driven and will ease the development of an AI.
Furthermore a network synchronization protocol has been developed, that allows
to detect and even fix out-of-sync errors in network games most of the time.
With this addition, network games are supposed to be fully operational now.
</p>

<p class=\"announcement\">
Further changes include:<br>
* Many new effects, such as daylight and wind<br>
* Much improved pathfinder<br>
* Winning conditions<br>
* Simple terrain LOD (level of detail)<br>
* Flying units have their own moving quirks<br>
* Many new neutral models such as trees<br>
* Many much less apparent changes<br>
</p>

<p class=\"announcement\">
For a longer, though still incomplete list see
<a href=\"http://boson.eu.org/announces/changelog-0.11.php\">http://boson.eu.org/announces/changelog-0.11.php</a>
</p>

<p class=\"announcement\">
As a special highlight this release features an installer package with a
precompiled binary of Boson 0.11.<br>
This binary is linked statically against Qt, KDE, and several other libraries,
meaning that you do not require an installation of these libraries. This
package is still very experimental, however because of the amount of
libraries Boson depends on, it is now the recommended way of installing
Boson.<br>
At the moment there is an installer for 32bit x86 Linux platforms only. It is
not known whether it runs on other platforms (non x86 platforms will
definitely not work).
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
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=353607\">http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=353607</a><br>
Installation instructions are at <a href=\"http://boson.eu.org/install.php\">http://boson.eu.org/install.php</a>
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
