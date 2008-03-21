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
$filename="boson-0.6.1.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.6.1 announcement");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson 0.6.1 announcement");
draw_bigbox_text("
<p class=\"announcement\">
Release of Boson 0.6.1, a Real Time Strategy (RTS) game for the K Desktop
Environment.
</p>

<p class=\"announcement\">
Where 0.6 was a complete rewrite to libkdegames and KDE 3.0, 0.6.1 is a minor
bug fix release.
</p>

<p class=\"announcement\">
Boson currently requires a minimum of 2 players due to lack of an artificial
intelligence. We are in need of more developers, especially graphics artists. If
you have any spare time and the ability to code in Qt/C++ or you know your way
around a graphics editor, we would appreciate the help.
</p>

<p class=\"announcement\">
The 0.6 release does not include the map editor for functionality and stability
reasons.
</p>

<p class=\"announcement\">
List of bug fixes since <a href=\"boson-0.6.php\">0.6</a>:<br>
 - The chat widget is now hidden by default<br>
 - Stopping unit/building production now only returns money if building was
   started (opposed to still in queue)<br>
 - Boson will now compile with gcc 2.96, 3.1, and possibly 3.0<br>
 - Many small fixes<br>
</p>

<p class=\"announcement\">
To compile and install Boson, you will need the following:<br>
 - Gcc, Gnu make (<a href=\"http://www.gnu.org\">http://www.gnu.org</a>)<br>
 - Qt 3.0.x (<a href=\"http://www.trolltech.com\">http://www.trolltech.com</a>)<br>
 - KDE 3.x (<a href=\"http://www.kde.org\">http://www.kde.org</a>)<br>
</p>

<p class=\"announcement\">
note: You do not need the entire KDE distribution. Simply installing kdelibs,
kdebase, arts, kdegames, and kdemultimedia should suffice. If you are using
precompiled packages (such as rpms and debs), make sure you have the devel-
packages for each!
</p>

<p class=\"announcement\">
Installing Boson:<br>

There are two methods of installing Boson from tarballs. The easier way is to grab
<a href=\"http://prdownloads.sourceforge.net/boson/boson-all-0.6.1.tar.bz2?download\">boson-all-0.6.1.tar.bz2</a>. However,
that file is very big, and if you are on a slow connection, you may wish to
simply get the code and data packages. The required packages can be found at:
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087\">http://sourceforge.net/project/showfiles.php?group_id=15087</a>
</p>

<p class=\"announcement\">
Let me say that again: download either boson-all or both boson-code and
boson-data. Do not waste your time downloading all three! If you are updating
via CVS and you have 0.6 installed already, do not update the data module. It
has not changed since 0.6.
</p>

<p class=\"announcement\">
If you decided to grab the individual packages, you can optionally download the
music files for Boson at:
<a href=\"http://boson.eu.org/music/\">http://boson.eu.org/music/</a>
</p>

<p class=\"announcement\">
The installation itself:<br>

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

<p class=\"announcement\">
Tarball checksums and sizes for the paranoid:
</p>
<pre>File name                 Size   MD5 Checksum
boson-all-0.6.1.tar.bz2   27 MB  ee1571120c461787004e869f08fce8e2
boson-code-0.6.1.tar.bz2  487 kB fa403f5387def5fe6d3db1711048fca1
boson-data-0.6.1.tar.bz2  11 MB  da74a8d7cde05211e5adad82771c5174
boson-music-0.6.1.tar.bz2 17 MB  f777b7a232ef55eb23c0b547fc335873</pre>

<p class=\"announcement\">
The Boson Team
</p>
");
draw_bigbox_end();

main_area_end();

end_page();


?>
