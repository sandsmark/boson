<?php
/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
$filename="install.php";

/*****  Some includes  *****/
include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");

/*****  Start of main stuff  *****/

start_page("Installing");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Installing Boson");

draw_bigbox_text("Note that these instructions only apply if you're using a source code version.
	If you have a binary version, it should be installed like any other package for your distribution.<br><br>
	Before compiling, take a look at the <a href=\"info.php#compiler\">compiler requirements</a>
	and make sure you have <a href=\"download.php\">downloaded</a> at least the code and data packages
	OR the big all-in-one package.<br><br>
	Now you need to unpack, configure, compile and install them:<br><br>");

draw_bigbox_subheader("If you downloaded the big package");

draw_bigbox_text("</p><pre>$ tar xjvf boson-all-0.12.tar.bz2
$ cd boson-all-0.12
$ mkdir build
$ cd build
$ cmake ..
$ make
$ su
# make install</pre><br>
Note that if you want to install at a custom prefix, use
'cmake -DCMAKE_INSTALL_PREFIX=/install/boson/to ..' instead of 'cmake ..'.<br>
<p class=\"bigboxtext\">");

draw_bigbox_subheader("If you downloaded code, data and music separately do all of the following");

draw_bigbox_text("</p><pre>$ tar xjvf boson-code-0.12.tar.bz2
$ cd code
$ mkdir build
$ cd build
$ cmake ..
$ make
$ su
# make install</pre>
  <pre>$ tar xjvf boson-data-0.12.tar.bz2
$ cd data
$ mkdir build
$ cd build
$ cmake ..
$ make
$ su
# make install</pre>
  <pre>$ tar xjvf boson-music-0.12.tar.bz2
$ cd music
$ mkdir build
$ cd build
$ cmake ..
$ make
$ su
# make install</pre><p class=\"bigboxtext\">
  After you've done it, Boson should be installed! Happy playing!<br><br>");

draw_bigbox_end();

main_area_end();

end_page();


?>
