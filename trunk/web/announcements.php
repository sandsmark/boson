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
$filename="announcements.php";

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
html_print_header("Announcements");
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
draw_bigbox_begin("Announcements");
draw_bigbox_text("All Boson announcements:
    <ul>
    <li><a href=\"http://lists.kde.org/?l=kde-announce&m=93741789332505&w=2\">Boson 0.1 announcement</a></li>
    <li><a href=\"http://lists.kde.org/?l=kde-announce&m=94069330417799&w=2\">Boson 0.2 announcement</a></li>
    <li><a href=\"http://lists.kde.org/?l=kde-announce&m=94677285723944&w=2\">Boson 0.3 announcement</a></li>
    <li><a href=\"http://lists.kde.org/?l=kde-announce&m=95700975705364&w=2\">Boson 0.4 announcement</a></li>
    <li><a href=\"http://www.kdenews.org/972864519/\">Boson 0.5 announcement</a></li>
    <li><a href=\"announces/boson-0.6.php\">Boson 0.6 press release</a></li>
    <li><a href=\"announces/boson-0.6.1.php\">Boson 0.6.1 press release</a></li>
    <li><a href=\"announces/boson-0.7.php\">Boson 0.7 press release</a></li>
    <li><a href=\"announces/boson-0.8.php\">Boson 0.8 press release</a></li>
    </ul>");
draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
