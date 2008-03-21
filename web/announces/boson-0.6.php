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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


/*****  Variables  *****/
$filename="boson-0.6.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.6 announcement");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson 0.6 announcement");
draw_bigbox_text("
<h3 class=\"announcement\">
Boson 0.6 released
</h3>
<p class=\"announcement\">
Release of Boson 0.6, a Real Time Strategy (RTS) game for the K Desktop
Environment.
</p>

<p class=\"announcement\">
Boson still requires a minimum of two players, as there is not yet an
artificial intelligence. The game is still under heavy development and
0.6 is merely a milestone rather than a fully playable game. We are in need of
more developers, especially graphics artists. If you have any
spare time and the ability to code in Qt/C++ or know your way around a
graphics program, please feel free to pop in and offer a hand.
</p>

<p class=\"announcement\">
After a year long hiatus (the last release being in October, 2000), Boson was
revived by a new set of developers. It is now completely
ported to KGame in libkdegames, which provides player and network management. 
Boson 0.6 is a complete rewrite from it's predecessor,
with not a single line of code remaining untouched.
</p>

<p class=\"announcement\">
Because of recent changes in the source code, the 0.6 release does not include
the map editor (see boson-devel archives at
<a href=\"http://sourceforge.net/mailarchive/forum.php?forum_id=6890\">http://sourceforge.net/mailarchive/forum.php?forum_id=6890)</a>. We have, however,
been working very hard to ensure that this the only
limitation compared to the last release (0.5). Due to the massive overhaul 
necessary to bring Boson to this point, a changelog was not
possible.
</p>

<p class=\"announcement\">
Major changes and additions since version 0.5:<br>
- auto-shoot: Defensive buildings automatically shoot at enemies when they are 
in range. Normal units do not do this yet.<br>
- split views: you can split a display and play at several places on the map 
at once<br>
- full network capability (please note that single player mode is quite boring 
without an AI)<br>
- fog of war<br>
- mineral/oil harvesting (experimental)<br>
- fullscreen mode<br>
- Map and scenario files are now XML based (was originally binary). All units 
can be configured using KConfig syntax<br>
- music, sound and speech<br>
- pathfinding (thanks to some excellent work by it's hacker!)<br>
- Most units actually have \"sense\", e.g. if your Comsat station is destroyed
your minimap will disappear.<br>
</p>

<p class=\"announcement\">
Dependencies<br>
To compile and install Boson, you will need (other than the usual compiler 
tools):<br>
- Qt 3 (see <a href=\"http://www.trolltech.com\">http://www.trolltech.com</a>)<br>
- KDE 3 (see <a href=\"http://www.kde.org\">http://www.kde.org</a>)<br>
  - kdelibs<br>
  - kdegames (at least libkdegames)<br>
  - kdemultimedia for sound support (might not compile without it, as we 
didn't test)<br>
Please note that you need the development files (e.g. headers) of all these 
packages. We do not (yet) provide binary packages.
</p>

<p class=\"announcement\">
Installing Boson<br>
There are two methods of installing Boson (from tarballs). The easier way is
to grab <a href=\"http://prdownloads.sourceforge.net/boson/boson-all-0.6.tar.bz2?download\">boson-all-0.6.tar.bz2</a>. However,
that file is very big, and if you are on a slow connection, you may wish to just
get the code and data packages. The required packages can be found at
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087\">http://sourceforge.net/project/showfiles.php?group_id=15087</a>.
</p>

<p class=\"announcement\">
Let me say that again: download either boson-all or both boson-code and
boson-data. Do not waste your time downloading all three!
</p>

<p class=\"announcement\">
If you decided to grab the individual packages, you can optionally download
the music files for Boson at
<a href=\"http://boson.eu.org/music/\">http://boson.eu.org/music/</a>.
</p>

<p class=\"announcement\">
The installation itself:
  Decompress the package(s):
</p>
<pre>% tar xjvf &lt;package name&gt;</pre>
<p class=\"announcement\">
  cd into each directory and:
</p>
<pre>% ./configure
% make</pre>
<p class=\"announcement\">
  As root:
</p>
<pre># make install</pre>
</p>

<p class=\"announcement\">
Tarball checksums and sizes for the paranoid:
<pre>
File name               Description             Size    MD5 Checksum
boson-all-0.6.tar.bz2   Everything              27.6 MB 92e5b16f3bdc7d31238d23d298793e15
boson-code-0.6.tar.bz2  The Boson game engine   501 kB  3d7b8b3967c5ada20b0ae80c56517aad
boson-data-0.6.tar.bz2  Graphics, sounds        10.9 MB 0e470b3f089424da3dc1cb05a0230a6b
boson-music-0.6.tar.bz2 Music (optional)        16.6 MB f777b7a232ef55eb23c0b547fc335873
</pre>
</p>

<p class=\"announcement\">
The Boson Team
</p>

");
draw_bigbox_end();

main_area_end();

end_page();


?>
