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
$filename="all-news.php";
$basedir="";

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
html_print_header("All news");
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


news_box_begin();

$articles = get_article_array($all_news_max);
for ($i = 0; $i < count($articles); $i++)
{
   $articles[$i]->print_me($fd);
}
main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();



?>
