<?php


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
