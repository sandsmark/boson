<?php


/*****  Variables  *****/
$filename="stories.php";

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
html_print_header("Boson stories");
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
draw_bigbox_begin("Boson stories");
draw_bigbox_text("Boson stories:
    <ul>
    <li><a href=\"stories/story-20021017.php\">Story #1&nbsp;&nbsp;(Oct. 17, 2002)</a></li>
    <li><a href=\"stories/story-20021108.php\">Story #2&nbsp;&nbsp;(Nov. 08, 2002)</a></li>
    </ul>");
draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
