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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


/*****  Variables  *****/
$filename="announcements.php";
$basedir="";

/*****  Some includes  *****/
include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");

/*****  Start of main stuff  *****/
start_page("Announcements");

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
    </p>
    <ul>
    <li><a href=\"http://lists.kde.org/?l=kde-announce&m=93741789332505&amp;w=2\">Boson 0.1 announcement</a></li>
    <li><a href=\"http://lists.kde.org/?l=kde-announce&m=94069330417799&amp;w=2\">Boson 0.2 announcement</a></li>
    <li><a href=\"http://lists.kde.org/?l=kde-announce&m=94677285723944&amp;w=2\">Boson 0.3 announcement</a></li>
    <li><a href=\"http://lists.kde.org/?l=kde-announce&m=95700975705364&amp;w=2\">Boson 0.4 announcement</a></li>
    <li><a href=\"http://www.kdenews.org/972864519/\">Boson 0.5 announcement</a></li>
    <li><a href=\"announces/boson-0.6.php\">Boson 0.6 announcement</a></li>
    <li><a href=\"announces/boson-0.6.1.php\">Boson 0.6.1 announcement</a></li>
    <li><a href=\"announces/boson-0.7.php\">Boson 0.7 announcement</a></li>
    <li><a href=\"announces/boson-0.8.php\">Boson 0.8 announcement</a></li>
    <li><a href=\"announces/boson-0.9.php\">Boson 0.9 announcement</a></li>
    <li><a href=\"announces/boson-0.9.1.php\">Boson 0.9.1 announcement</a></li>
    <li><a href=\"announces/boson-0.10.php\">Boson 0.10 announcement</a></li>
    <li><a href=\"announces/boson-0.11.php\">Boson 0.11 announcement</a></li>
    <li><a href=\"announces/boson-0.12.php\">Boson 0.12 announcement</a></li>
    <li><a href=\"announces/boson-0.13.php\">Boson 0.13 announcement</a></li>
    </ul>
    <p class=\"bigboxtext\">");
draw_bigbox_end();

main_area_end();

end_page();


?>
