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
$filename="boson-0.8.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson 0.8 announcement");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson 0.8 announcement");
draw_bigbox_text("
<p class=\"announcement\">
Release of Boson 0.8, a Real Time Strategy (RTS) game for the K Desktop
Environment.
</p>

<p class=\"announcement\">
Since our last release we have mainly been focussing on improving speed and the
addition of 3d-terrain. Beside that, we ported the savegame format to XML which
enables us to support this format in future boson releases, too.
</p>

<p class=\"announcement\">
Unfortunately we had to disable sound for this release. You can re-enable it by
starting boson from the command line with the --sound switch.
This limitation was necessary, because arts causes major performance problems on
runtime, making the game hardly playable.
We hope that we will be able to re-enable sound for the next release. 
</p>

<p class=\"announcement\">
On the compilation front we are happy to announce two things:<br>
First of all the prefix problem that many users experienced (data files were
missing, although installed) has been resolved.<br>
Second we are proud to report that boson also compiles and runs under FreeBSD.
We would be happy to hear from a user who has hardware-accelerated drivers
installed, as our FreeBSD addict uses software rendering only, which means that
boson runs very slowly for him.
</p>

<p class=\"announcement\">
List of major new features since <a href=\"boson-0.7.php\">0.7</a>:<br>
- Height map for 3d-terrain<br>
- New map file format (.tar.gz instead of .gz)<br>
- Preload map information on startup<br>
- Big performance improvements<br>
- New save game format<br>
- Smoke support for power plant, oil refinery, mineral refinery etc (see our 
<a href=\"../screenshots.php\">screenshots</a>)<br>
- Unit model improvements<br>
- Editor: Buildings are placed in constructed mode<br>
- Editor: Better support for big maps<br>
- of course a big list of smaller features/fixes<br>
</p>

<p class=\"announcement\">
To compile and install Boson, you will need the following:<br>
 - Gcc, Gnu make (<a href=\"http://www.gnu.org\">http://www.gnu.org</a>)<br>
 - Perl (<a href=\"http://www.perl.org\">http://www.perl.org</a>)<br>
 - Qt 3.0.x (<a href=\"http://www.trolltech.com\">http://www.trolltech.com</a>)<br>
 - KDE 3.x (<a href=\"http://www.kde.org\">http://www.kde.org</a>)<br>
 - OpenGL (should be included in your XFree Server or graphics card driver)<br>
 - lib3ds (<a href=\"http://lib3ds.sourceforge.net\">http://lib3ds.sourceforge.net</a>)<br>
 - wml (<a href=\"http://www.thewml.org\">http://www.thewml.org</a>)<br>
 - See also our <a href=\"info.php\">info page</a><br>
Even though Qt/KDE 3.0 are supported, we recommend Qt/KDE 3.1
</p>

<p class=\"announcement\">
Note: You need at minimum arts, kdelibs, and kdegames.
If you are using precompiled packages (such as rpms and debs), make sure you
have the -devel packages for each. 
</p>

<p class=\"announcement\">
Installing Boson:
There are two methods of installing Boson from tarballs. The easier way is to
grab boson-all-0.8.tar.bz2. However, that file is very big, and if you are on 
a slow connection, you may wish to simply get the code and data packages. The
required packages can be found at: 
http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=149745
</p>

<p class=\"announcement\">
Let me say that again: download either boson-all or both boson-code and
boson-data. Do not waste your time downloading all three! 
</p>

<p class=\"announcement\">
If you decided to grab the individual packages, you can optionally download
the music files (however note that sound is disabled by default in this
version of boson!).
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
boson-all-0.8.tar.bz2   31MB   0b0b090993e215941abd37f08ce3b781
boson-code-0.8.tar.bz2  808kB  5ea18f832ded8eddb28c3fc6d9168c87
boson-data-0.8.tar.bz2  15MB   e4a821c4550d92d9d3094ee163407a03
boson-music-0.8.tar.bz2 16MB   afc341cf89bff25b52979eef3547dd65
</pre>

<p class=\"announcement\">
note: To get the checksum of a file, type:<br>
<pre>% md5sum &lt;file&gt;</pre>
</p>

<p class=\"announcement\">
The Boson Team<br>
http://boson.eu.org<br><br>
</p>

<p class=\"announcement\">
The scene: Timo and Rivo are sitting in the war room discussing the seemingly
endless war effort. Andi walks in..<br>
Andi: Moin, how is everything today?<br>
Rivo: Re! Other than a few minor problems with the artillery and the planes,
everything is going good<br>
Andi: problems?<br>
Rivo: yes.. our tanks arent used to hills and the planes are turning out to be a
one shot deal.<br>
</p>

<p class=\"announcement\">
Rivo shudders, suddenly recalling the events of the day before in the Air
Control Tower..<br>
Felix, piloting one of the new planes: Sir! We are in trouble!<br>
Rivo: What do you mean.. trouble?<br>
Felix: Sir, we are under attack and my plane refuses to land!<br>
* The sound of gunfire is heard over the radio<br>
Rivo: Well shoot back at them!<br>
Felix: I can't! I am completely out of ammo<br>
Rivo, to Thomas: What does he mean, he can't land?<br>
Thomas: Well, the planes don't have that ability yet.<br>
Rivo: What!? What's the point of flying them yet?<br>
Thomas: Ah, good question.<br>
Felix: Help! A missile is coming right at me!<br>
</p>

<p class=\"announcement\">
Andi: I see.. any word from CINCPROP?<br>
Timo: No sir, the Commander In Chief of Propaganda is still missing<br>
Andi: Hasn't it been like 4 months now?<br>
Timo: Yes sir<br>
</p>

<p class=\"announcement\">
Andi: So you are saying that the tanks keep crashing, the planes are useless,
CINCPROP is missing and, as if that wasn't enough, my Lieutentant isn't even
paying attention to me?<br>
* Rivo snaps out of his daydream and blushes<br>
Rivo: Yes sir.<br>
Andi: I see. Is there any good news at all?<br>
Rivo: Well, our best pilot was saved from imminent doom because of a small
glitch in the enemie's missiles<br>
</p>
");
draw_bigbox_end();

main_area_end();

end_page();


?>
