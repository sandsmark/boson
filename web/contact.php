<?php


/*****  Variables  *****/
$filename="contact.php";

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
html_print_header("Contact us");
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
draw_bigbox_begin("Contact us");
draw_bigbox_subheader("Mailing lists");
draw_bigbox_text("We have 3 mailing lists.<br>
    First one, <a href=\"mailto:boson-devel@lists.sourceforge.net\">boson-devel@lists.sourceforge.net</a>,
    is meant for users and developers. If you have a question about Boson or problem,
    you can always come there and ask.<br>
    Second mailing list is boson-cvs@lists.sourceforge.net. This is only meant
    for CVS logs, please do not write there.<br>
    Third is boson-bugs@lists.sourceforge.net. This is also not meant for
    writing, bug reports are sent there, but please do not write there either.
    See below on how to report bugs.");
draw_bigbox_subheader("IRC");
draw_bigbox_text("We also have an IRC channel. If you have questions/bugs/problems/compile errors
    you can just go to #boson on openprojects (e.g. irc.openprojects.net).");
draw_bigbox_subheader("Reporting bugs");
draw_bigbox_text("You can report bugs using <a href=\"http://bugs.kde.org/\">KDE's bug tracking system</a>.");
draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
