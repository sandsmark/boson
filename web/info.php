<?php


/*****  Variables  *****/
$filename="info.php";

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
html_print_header("More information");
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
draw_bigbox_begin("More information");

draw_bigbox_subheader("About");
draw_bigbox_text("Boson is an OpenGL real-time strategy game, with the feeling of Command&amp;Conquer(tm) or StarCraft(tm) and OpenGL support. It is designed to run on Unix (Linux) computers, and is built on top of the libkdegames, kde and Qt libraries.<br>A minimum of two players is required, since there is no artificial intelligence yet.<br><br>");

draw_bigbox_subheader("Licencing");
draw_bigbox_text("Boson's code, graphics and sounds are published under the GNU General Public License (GPL).<br><br>");

draw_bigbox_subheader("Requirements");
draw_bigbox_text("<b>Notice:</b> These requirements apply to current development (CVS) version.<br><br>

    Latest stable release ($latestversion) probably runs on slower systems and 
    it doesn't require a 3D accelerator card. Also, it doesn't need lib3ds or OpenGL.<br><br>

    <b>Minimum hardware requirements:</b><br>
    Note: If you have an older graphics card like a TNT2 you will need a fast CPU.<br>
    * 500 MHz ix86 Processor (don't know anything about other machines)<br>
    * 256 MB RAM<br>
    * 3D accelerator card (this is the most important part!!)<br>
    * Optional: Sound card<br>
    <br>
    <b>Minimum software requirements:</b><br>
    * XFree 4.x server (with OpenGL support) - see <a href=\"http://www.xfree86.org/\">http://www.xfree86.org/</a><br>
    * Qt 3.0.3 or better - see <a href=\"http://www.trolltech.com/\">http://www.trolltech.com/</a><br>
    * KDE 3.0.0 or better (at least 3.0.1 recommended) - see <a href=\"http://www.kde.org/\">http://www.kde.org/</a><br>
    * kdegames/libkdegames - usually shipped with KDE<br>
    * kdemultimedia - also shipped with KDE<br>
    * libvorbis (for sound and music) - see <a href=\"http://www.vorbis.com/\">http://www.vorbis.com/</a><br>
    * lib3ds (Units and building rendering) - see <a href=\"http://lib3ds.sf.net/\">http://lib3ds.sf.net/</a><br>
    * OpenGL - The OpenGL library (should be included in your XFree Server or graphics card driver)<br><br>");

draw_bigbox_subheader("<a name=\"compiler\"></a>Compiler requirements");
draw_bigbox_text("To compile Boson (you don't need a compiler if you're downloading a binary version),
    you need <a href=\"http://gcc.gnu.org/\">Gcc</a> version 2.95.3 or 3.1.x or 3.2.
    <b>Gcc 3.0.x</b> is untested and <b>may not work</b>. <b>Gcc 2.96 won't work</b> either."); //'

draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
