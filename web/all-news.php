<?php
$filename="all-news.php";
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
