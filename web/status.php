<?php


/*****  Variables  *****/
$filename="status.php";

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
html_print_header("Status");
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
draw_bigbox_begin("Status");
draw_bigbox_text("Boson is currently still under heavy development.<br><br>
  In the latest CVS version, there are no significant changes, compared
  to the latest stable release ($latestversion).<br><br>
  We're currently heading for release of version 0.8, but it is currently
  unclear when it will be released.<br><br>");

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
