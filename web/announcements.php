<?php


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
    <ul><li><a href=\"announces/boson-0.6.php\">Boson 0.6 press release</a></li>
    <li><a href=\"announces/boson-0.6.1.php\">Boson 0.6.1 press release</a></li></ul>");
draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
