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
draw_bigbox_begin("About Boson");
echo "<tr><td>Boson is an OpenGL real-time strategy game, with the
	feeling of Command&amp;Conquer(tm) or StarCraft(tm).
	It is designed to run on Unix (Linux) computers, and is built on top of the 
	KDE, Qt and kdegames libraries.<br> A minimum of two players is required,
	since there is no artificial intelligence yet.<br><br></td></tr>";
draw_bigbox_end();

// Print news
news_box_begin();
/** Add news articles here!
**  To add an article, call
**  news_article(<title>, <date added>, <full text>);
**/
news_article("Another update", "October 14th 2002 17:54",
    "Again, much has changed during last weeks. I wanted to take some
    screenshots and update our screenshots page, but things just kept changing
    (and they still do :-)) and I didn't want outdated screenies, so they'll
    come later ;-)<br>
    So here is list of updated stuff:
    <ul>
    <li>Missiles are properly rotated</li>
    <li>New map: Basic 2 which is modified version of Basic map</li>
    <li>Added menu entry and shortcut to grab screenshots. Just press Ctrl+G</li>
    <li>.desktop files were renamed. They now have better suffixes like .boson
      and .unit</li>
    <li>New camera system.</li>
    <li>New loading/saving widget</li>
    <li>Updated units README and added README about particle systems</li>
    <li>Some new units and some updated unit models</li>
    <li>and, of course, bugfixes and many smaller things</li>
    </ul>
    And finally, some statistics: since 1st September or so, our homepage has
    been visited almost 9500 times, more than 200 times a day on average :-)
    <br><br>");
news_article("Updates", "September 28th 2002 19:55",
    "Pretty much has happened during last few weeks.<br>
    Andi added <a href=\"status.php#features\">TODO list to the status page</a>, you
    can now check if your favourite feature will be included in next release.
    (But note that this is an incomplete list and merely shows what needs to be done
    for the next release).
    If it isn't there, you can tell
    us about your wishes via <a href=\"contact.php#bugs\">our bug tracking
    system</a> or <a href=\"contact.php#mail\">write to our mailing list</a>!<br>
    About new features, we now have configurable texture filters (you can choose
    good compromise between speed and quality), shots/missiles support (you can
    see them flying!) and today I added support to configurable particle
    systems, weapons and shots. Everyone can now edit them in configuration
    files, just like units!<br><br>");
news_article("More eyecandy", "September 10th 2002 20:00",
    "Boson now has particle effects! This means that shots and explosions work
    again! Also, we have smoke clouds for destroyed units and flames for
    half-destroyed facilities. Go to <a href=\"screenshots.php\">screenshots
    page</a> and check out the new screenshots showing explosions and smoke! Or even
    better, update your CVS copy. If you don't have a CVS copy, go to
    <a href=\"download.php#cvs\">the download page</a> for instructions on how to get one!<br>
    Also, here are some updates to this page (mostly style) by Ben.<br>
    We hope you like it!<br><br>");
news_article("New features!", "September 2nd 2002 18:08",
    "I promised to add new features and here comes the first one: you can now
    change the style of this homepage. Just go to the <a href=\"options.php\">options
    page</a> and select the style you like most!<br>
    Currently, there are only two different styles, but if you want more, you
    can write them yourself! All you need to do is to write a new stylesheet that
    specifies the used colors. It's not hard! You can use style-blue.css as
    template. When you're done, send them to
    <a href=\"mailto:rivolaks@hot.ee\">me</a>, I'm happy to put them up here!<br><br>");
news_article("New homepage", "September 1st 2002 19:22",
    "Boson has a new homepage! This homepage should be much better than the old
    one, but it's new, so it may contain some errors (I hope it doesn't though).
    If you find an error, please report it to <a href=\"mailto:boson-devel@lists.sourceforge.net\">our mailinglist</a><br>
    This is not a modification of our old homepage, it's written from scratch in
    PHP, which hopefully makes it easier to maintain than the old one.<br>
    What has changed compared to the old page? It has a new look which reminds
    the look of the <a href=\"http://www.kde.org/\">KDE homepage</a>. Also, we have
    a couple of new pages: <br>
    <ul><li><a href=\"install.php\">Install</a></li>
    <li><a href=\"info.php\">More information</a></li>
    <li><a href=\"links.php\">Related links</a></li></ul>
    and all pages now have valid HTML and CSS.<br>
    This page is still under development and we're adding new features, so come
    back soon!<br><br>");
news_box_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
