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
$filename="index.php";

/*****  Some includes  *****/
include("common.php");
include("sidebar.php");
include("main.php");
include("counter.php");
include("boxes.php");
include("variables.php");
include("news.php");

/*****  Start of main stuff  *****/

do_start_stuff();

// Headers
html_print_header("Main page");
print_header();

// Main table
main_table_begin();

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_old_news();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// About
draw_bigbox_begin("About Boson");
echo "<tr><td>Boson is an OpenGL real-time strategy game, with the
	feeling of Command&amp;Conquer(tm) or StarCraft(tm).
	It is designed to run on Unix (Linux) computers, and is built on top of the 
	KDE, Qt and kdegames libraries.<br> A minimum of two players is required,
	since there is no artificial intelligence yet.<br><br></td></tr>";
draw_bigbox_end();

// Print news
news_box_begin();
/**
** If you want to add news article, add it to the beginning of news.text.
**/
display_main_news();
news_box_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
