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
draw_bigbox_begin("Installing Boson");

draw_bigbox_text("Note that these instructions only apply if you're using a source code version.
	If you have a binary version, it should be installed like any other package for your distribution.<br><br>
	Before compiling, take a look at the <a href=\"info.php#compiler\">compiler requirements</a>
	and make sure you have <a href=\"download.php\">downloaded</a> at least the code and data packages
	OR the big all-in-one package.<br><br>
	Now you need to unpack, configure, compile and install them:<br><br>");

draw_bigbox_subheader("If you downloaded the big package");

draw_bigbox_text("<pre>$ tar xjvf boson-all-0.6.1.tar.bz2<br>$ cd boson<br>$ ./configure<br>$ make<br>$ su<br># make install</pre><br>");

draw_bigbox_subheader("If you downloaded code, data and music separately");

draw_bigbox_text("<pre>$ tar xjvf boson-code-0.6.1.tar.bz2<br>$ cd code<br>$ ./configure<br>$ make<br>$ su<br># make install</pre>
	<pre>$ tar xjvf boson-data-0.6.1.tar.bz2<br>$ cd data<br>$ ./configure<br>$ make<br>$ su<br># make install</pre>
	<pre>$ tar xjvf boson-music-0.6.1.tar.bz2<br>$ cd music<br>$ ./configure<br>$ make<br>$ su<br># make install</pre>
	After you've done it, Boson should be installed! Happy playing!");

draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
