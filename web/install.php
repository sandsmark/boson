<?php


/*****  Variables  *****/
$filename="install.php";

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
html_print_header("Installing");
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
draw_box("Installing Boson");
echo "How to install Boson? Note that these instructions only apply if you're
    using source code version. If you have binary version, it should be
    installed like any other package for your distribution.<br>
    Also, before compiling, take a look at <a href=\"info.php#compiler\">compiler
    requirements</a><br>
    First, make sure you have <a href=\"download.php\">downloaded</a>
    at least code and data packages OR big all-in-one package.<br>
    Then you need to unpack, configure, compile and install them:<br><br>
    <pre>  tar xjvf boson-all-0.6.1.tar.bz2
  cd boson
  ./configure
  make
  su
  make install</pre><br>
    If you didn't download all-in-one package, then you need to repeat this for
    all packages.<br>
    Replace \"all\" in tar command and \"boson\" in cd command with name of the
    package unless your using all-in-one package. For source code package,
    replace it with \"code\", for data package with \"data\" and for music
    package with \"music\" (suprise ;-)).<br>
    After you've done it, Boson should be installed! Happy playing!";

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
