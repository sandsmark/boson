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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


/*****  Variables  *****/
$filename="links.php";

/*****  Some includes  *****/
include("common.php");
include("sidebar.php");
include("main.php");
include("counter.php");
include("boxes.php");
include("variables.php");

/*****  Start of main stuff  *****/

do_start_stuff();

// Headers
html_print_header("Links");
print_header();

// Main table
main_table_begin();

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Links");
draw_bigbox_text("Here you will find some (hopefully) useful links:<br><br>
  <a href=\"http://sf.net/projects/boson/\" target=\"_blank\">Boson at SourceForge</a> Boson uses SourceForge.net's services<br>
  <a href=\"http://www.freehackers.org/boson/handbook/\" target=\"_blank\">The Boson handbook</a> Daily generated from cvs<br>
  <a href=\"http://www.kde.org/\" target=\"_blank\">KDE homepage</a> Boson uses KDE<br>
  <a href=\"http://www.trolltech.com\" target=\"_blank\">Trolltech (creators of QT)</a> Boson also uses Qt<br>
  <a href=\"http://www.freehackers.org/about/content.html\" target=\"_blank\">freehackers.org</a>Other free software<br>
  <a href=\"http://www.freecraft.org/\" target=\"_blank\">Freecraft homepage</a> Freecraft is another RTS for Linux<br>
  <a href=\"http://sourceforge.net\" target=\"_blank\">SF.net</a> SourceForge.net<br>");
draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
