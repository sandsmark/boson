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
draw_bigbox_text("Boson is currently still under heavy development.<br><br>In the latest CVS version, the biggest changes, compared to latest stable release ($latestversion), are OpenGL support (Boson is now 3D), support for upgrades, nicer commandframe and many other little modifications. Also, a large number of bugs have been fixed.<br><br>We're currently heading for release of version 0.7 which will probably be released late 2002 or early 2003 (Note that these dates are <b>not</b> official and may change).");
draw_bigbox_end();
//"

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
