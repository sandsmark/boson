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
draw_box("Links");
echo "
    Here you will find some (hopefully) useful links:<br>
    <a href=\"http://www.kde.org/\">KDE homepage</a> Boson uses KDE<br>
    <a href=\"http://www.trolltech.com\">Trolltech (creators of QT)</a> Boson also uses Qt<br>
    <a href=\"http://www.freehackers.org/about/content.html\">Other free software</a><br>
    <a href=\"http://www.freecraft.org/\">Freecraft homepage</a> Freecraft is another RTS for Linux<br>
    <a href=\"http://sourceforge.net\">SF.net</a> Boson uses SourceForge.net's services<br>
";
main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
