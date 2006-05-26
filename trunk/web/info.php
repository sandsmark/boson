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
$filename="info.php";

/*****  Some includes  *****/
include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");

/*****  Start of main stuff  *****/

start_page("More information");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("More information");

draw_bigbox_subheader("Licencing");
draw_bigbox_text("Boson's code, graphics and sounds are published under the GNU
  General Public License (GPL).<br><br>");

draw_bigbox_subheader("Requirements");
draw_bigbox_text("These requirements apply to the latest stable release
    ($latestversion)<br><br>

    <b>Minimum hardware requirements:</b><br>
    Note: If you have an older graphics card like a TNT2 you will need a fast CPU.<br>
    * 1 GHz x86 Processor (don't know anything about other machines). AMD 64 is also known to work<br>
    * 256 MB RAM (512 MB is recommended)<br>
    * 3D accelerator card (modern card with DirectX 9 support is recommended)<br>
    * Optional: Sound card<br>
    <br>
    <b>Minimum software requirements:</b><br>
    * XFree 4.x or X.org server (with OpenGL support) - see <a href=\"http://www.xfree86.org/\" target=\"_blank\">http://www.xfree86.org/</a><br>
    * OpenGL 1.2 - The OpenGL library (should be included in your XFree Server or graphics card driver)<br>
    * Qt 3.1 or better - see <a href=\"http://www.trolltech.com/\" target=\"_blank\">http://www.trolltech.com/</a><br>
    * kdelibs 3.1 or better (3.0 is untested) - see <a href=\"http://www.kde.org/\" target=\"_blank\">http://www.kde.org/</a><br>
    * Python 2.0 or better (2.3 is recommended) - see <a href=\"http://www.python.org/\" target=\"_blank\">http://www.python.org/</a><br>
    * libvorbis (for sound and music) - see <a href=\"http://www.vorbis.com/\" target=\"_blank\">http://www.vorbis.com/</a><br>
    * OpenAL (audio library) - see <a href=\"http://www.openal.org/\"target=\"_blank\">http://www.openal.org/</a><br>
    <br>");
//    * WML and Perl  - For the handbook - see <a href=\"http://www.thewml.org\" target=\"_blank\">http://www.thewml.org</a><br>

draw_bigbox_subheader("<a name=\"compiler\"></a>Compiler requirements");
draw_bigbox_text("To compile Boson (you don't need a compiler if you're downloading a binary version),
    you need <a href=\"http://gcc.gnu.org/\" target=\"_blank\">Gcc</a> version 3.3 or better.");

draw_bigbox_end();

main_area_end();

end_page();

?>
