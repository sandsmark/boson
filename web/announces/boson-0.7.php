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
$filename="boson-0.7.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.7 announcement");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson 0.7 announcement");
draw_bigbox_text("
<p class=\"announcement\">
Release of Boson 0.7, a Real Time Strategy (RTS) game for the K Desktop
Environment.
</p>

<p class=\"announcement\">
Version 0.7 of Boson is primarily a port to OpenGL. This means the game must now
be hardware accelerated with a video card (assuming you want to actually play
the game and not admire how pretty it is). However, OpenGL also allows our
developers to further enhance the game with special effects.
</p>

<p class=\"announcement\">
Boson currently requires a minimum of 2 players due to lack of an artificial
intelligence and is not yet playable. We are in need of more developers and
graphics artists. If you have any spare time and the ability to code in
Qt/C++ or you know your way around a graphics editor, we would appreciate
the help.
</p>

<p class=\"announcement\">
The map editor has returned! It is still not fully functional but maps are
modifiable and you can add and delete units
and buildings.
</p>

<p class=\"announcement\">
List of major new features since <a href=\"boson-0.6.1.php\">0.6.1</a>:<br>
 - Units and buildings are now all drawn with OpenGL, each with their own 3d
   model.<br>
 - New particle system, which allows for explosions, fire and smoke<br>
 - Upgrade support (experimental)<br>
 - Multiple weapon per unit support (experimental)<br>
 - Visible missile support (experimental)<br>
 - Support for unit animation (currently only used for com-sat)<br>
 - Improved cursor configurability<br>
 - Pathfinding has been improved even more to make movement smarter<br>
 - Documentation has been greatly improved<br>
 - Many bug fixes<br />
</p>

<p class=\"announcement\">
To compile and install Boson, you will need the following:<br>
 - Gcc, Gnu make (<a href=\"http://www.gnu.org\">http://www.gnu.org</a>)<br>
 - Qt 3.0.x (<a href=\"http://www.trolltech.com\">http://www.trolltech.com</a>)<br>
 - KDE 3.x (<a href=\"http://www.kde.org\">http://www.kde.org</a>)<br>
 - lib3ds (<a href=\"http://lib3ds.sourceforge.net\">http://lib3ds.sourceforge.net</a>)<br>
</p>

<p class=\"announcement\">
note: You need at minimum kdelibs, kdebase, arts, kdegames, and kdemultimedia.
If you are using precompiled packages (such as rpms and debs),
make sure you have the devel- packages for each.
</p>

<p class=\"announcement\">
Installing Boson:<br>

There are two methods of installing Boson from tarballs. The easier way is to grab
<a href=\"http://prdownloads.sourceforge.net/boson/boson-all-0.7.tar.bz2?download\">boson-all-0.7.tar.bz2</a>.
However, that file is very big, and if you are on a slow connection, you may
wish to simply get the code and data packages. The required packages can be
found at:
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=121263\">http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=121263</a>
</p>

<p class=\"announcement\">
Let me say that again: download either boson-all or both boson-code and
boson-data. Do not waste your time downloading
all three!
</p>

<p class=\"announcement\">
If you decided to grab the individual packages, you can optionally download the
music files.
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
<pre>
File name               Size   MD5 Checksum
boson-all-0.7.tar.bz2   35 MB  ab46f3432d10d144a37348377030b590
boson-code-0.7.tar.bz2  675 kB df9a9ff12ebb2d0e0c0b3915757b5745
boson-data-0.7.tar.bz2  19 MB  53d5a058099ee75a1c93ad6dbd811688
boson-music-0.7.tar.bz2 16 MB  650a3fbda64d884b63fb2e139212a144
</pre>

<p class=\"announcement\">
note: To get the checksum of a file, type:<br />
<pre>% md5sum &lt;file&gt;</pre>
</p>
<p class=\"announcement\">
The Boson Team
</p>

<p class=\"announcement\">
The prologue:<br />
General Andreas and his minions Rivo, Felix, Ben, Thomas and Scott had narrowly escaped doom
(<a href=\"../stories/story-20021017.php\">../stories/story-20021017.php</a>) by somehow shutting down the Boson
Particle generator. No,
we don't know how either.
</p>

<p class=\"announcement\">
The scene:<br />
The good General is in the briefing room discussing the imminent launch of an assault on a nearby military base.
Currently present: Andi, Felix, Rivo, Timo and Scott.
</p>

<p class=\"announcement\">
* Andi joins #boson<br />
&lt;Andi&gt; How are we doing?<br />
&lt;Felix&gt; Sir! Both the eastern assault first and second wave divisions are ready to go.<br />
&lt;Andi&gt; And the defenses?<br />
&lt;Rivo&gt; Looking good sir.<br />
&lt;Andi&gt; Analysis of the enemy?<br />
&lt;Scott&gt; We can expect some light resistance from their sam sites and pill boxes but strangely their tanks don't
fire back.
</p>

<p class=\"announcement\">
A strange 'dying scream' sound comes from Timo's mouth. Everybody looks at him. Andi shakes his head.
</p>

<p class=\"announcement\">
&lt;Andi&gt; Ah yes, I keep meaning to add in support for that.
</p>

<p class=\"announcement\">
Everybody in the room nods knowingly.
</p>

<p class=\"announcement\">
&lt;Andi&gt; When can we proceed with the assault?<br />
&lt;Rivo&gt; We should be ready to head out on the 10th.<br />
&lt;Andi&gt; The day of the second year of the revival, how fitting.<br />
* Scott mutters \"Here it comes..\"<br />
&lt;Andi&gt; This time I *WILL* succeed! They *WILL* tremble at my feet!<br />
* Andi cackles like a madman... again<br />
&lt;Felix&gt; Déja vu...
</p>

<p class=\"announcement\">
&lt;Timo&gt; AH TTACKING!<br />
&lt;Felix&gt; Dude, will you shut up!?
</p>
");
draw_bigbox_end();

main_area_end();

end_page();


?>
