<?php


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
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// About
draw_box("About Boson");
echo "
Boson is an OpenGL real-time strategy game, with the feeling of
Command&amp;Conquer(tm) or StarCraft(tm) and OpenGL support. It is designed to run
on Unix (Linux) computers, and is built on top of the libkdegames, kde and Qt
libraries.<br>
A minimum of two players is required, since there is no artificial intelligence yet.<br>
<br>";

// Print news
news_box_begin();
/** Add news articles here!
**  To add an article, call
**  news_article(<title>, <date added>, <full text>);
**/
news_article("New features!", "2nd September 2002 18:08",
    "I promised to add new features and here comes the first one: you can now
    change style of this homepage. Just go to <a href=\"options.php\">options
    page</a> and select style you like most!<br>
    Currently, there are only two different styles, but if you want more, you
    can write one yourself! All you need to do is to write new stylesheet that
    specifies colors used. It's not hard! You can use style-blue.css as
    template. When you're done, send them to
    <a href=\"mailto:rivolaks@hot.ee\">me</a>, I'm happy to put them up here!");
news_article("New homepage", "1st September 2002 19:22",
    "Boson has now new homepage! This homepage should be much better than old
    one, but it's new, so it may contain some errors (I hope it doesn't though).
    If you find an error, please report it to
    <a href=\"mailto:boson-devel@lists.sourceforge.net\">our mailinglist</a><br>
    This is not modification of our old homepage, it's written from scratch in
    PHP, which hopefully makes it easier to maintain than old one.<br>
    What has changed compared to the old page? It has new look which reminds
    look of <a href=\"http://www.kde.org/\">KDE homepage</a>. Also, we have
    couple of new pages (<a href=\"install.php\">page with installation
    instructions</a>, <a href=\"info.php\">information page</a> and
    <a href=\"links.php\">links page</a>) and all pages have now valid HTML and
    CSS.<br>
    This page is still under development and we're adding new features, so come
    back soon!");//'
news_box_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
