<?php


/*****  Variables  *****/
$filename="download.php";

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
html_print_header("Downloads");
print_header();

// Main table
main_table_begin();

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Downloads stuff
draw_bigbox_begin("Downloads");
echo "
    <tr><td>
      <b>Boson $latestversion (released on $latestversiondate) is
      currently latest version available.</b><br><br>
    </td></tr>";
draw_bigbox_subheader("All-in-one package");
draw_bigbox_text("This is a big package that contains source code and data files
    and music. You do not need to download any package below if you download this
    one<br>
    You can get it from <a href=\"http://prdownloads.sourceforge.net/boson/boson-all-0.6.1.tar.bz2?download\">here</a>
    (27 266 KB) or via SF.net's web interface from
    <a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=98577\">here</a><br>");
draw_bigbox_subheader("Source code");
draw_bigbox_text("You can download tarball (.tar.bz2) with the code from
    <a href=\"http://prdownloads.sourceforge.net/boson/boson-code-0.6.1.tar.bz2?download\">here</a> (487 KB) or via SF.net's web
    interface from <a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=98577\">here</a><br>
    Note that you also need to download at least data package to play Boson<br>");
draw_bigbox_subheader("Data package");
draw_bigbox_text("This tarball contains data files needed for playing Boson.<br>
    You can download it from
    <a href=\"http://prdownloads.sourceforge.net/boson/boson-data-0.6.1.tar.bz2?download\">here</a>
    (11 257 KB) or via SF.net's web interface from
    <a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=98577\">here</a><br>");
draw_bigbox_subheader("Music");
draw_bigbox_text("You don't need music package to play Boson, but it's
    recommended to also download this :-)<br>
    You can download tarball from
    <a href=\"http://prdownloads.sourceforge.net/boson/boson-music-0.6.1.tar.bz2?download\">here</a>
    (17 004 KB) or via SF.net's web interface from
    <a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=98577\">here</a><br>");

draw_bigbox_subheader("Gentoo");
draw_bigbox_text("A boson ebuild is available in the standard gentoo portage and is tested
    against gcc-2.95.3 (gentoo 1.2 and previous) and gcc-3.2 (gentoo 1.4 and following).
    Please just issue a 
    <pre> emerge boson </pre>
    as root on your gentoo box to get boson downloaded, compiled and installed");

draw_bigbox_subheader("RPM/binary releases");
draw_bigbox_text("Note that RPM packages are not provided by Boson team and are
    inofficial.<br>
    If you feel like building a RPM for boson (for any platform), we'll be happy
    to put a link here and/or add them to our site.<br>
    You can get RPM package of Boson from
    <a href=\"http://prdownloads.sourceforge.net/boson/boson-0.6.1-1.i386.rpm?download\">here</a>
    (27 929 KB) or from <a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=98577\">
    SF.net</a>. This package is compiled for Red Hat 7.3 on i386.<br>
    We also have RPM source package which is like all-in-one package, just that
    it's packaged with RPM. You can get it from
    <a href=\"http://prdownloads.sourceforge.net/boson/boson-0.6.1-1.src.rpm?download\">here</a>
    (27 271 KB) or from <a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=98577\">
    SF.net</a> again.<br>
    These package were provided by <a href=\"mailto:krzyko@users.sourceforge.net\">Krzysztof Kosz</a><br>");
draw_bigbox_subheader("Older versions");
draw_bigbox_text("Currently, you can download old releases only via
    <a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087\">SF.net's web interface</a>.<br>
    Available versions are
    <a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=93970\">Boson 0.6</a>
    (released on 10th June 2002) and
    <a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&release_id=17261\">Boson 0.5</a>
    (released on 30th October 2000).");
draw_bigbox_subheader("CVS - Bleeding edge development version");
draw_bigbox_text("You can get the code from CVS using these commands:
  <pre>  cvs -d:pserver:anonymous@cvs.boson.sourceforge.net:/cvsroot/boson login
  cvs -z3 -d:pserver:anonymous@cvs.boson.sourceforge.net:/cvsroot/boson co -P code</pre>
    Please just press the Enter key when you are prompted for a password. As soon
    as you have checked out the sources you can simply do a
  <pre>  cd code
  cvs -z3 up -Pd</pre>
    to update the sources. No need to login again.<br>
    You have to also get data files if you want to play:
  <pre>  cvs -z3 -d:pserver:anonymous@cvs.boson.sourceforge.net:/cvsroot/boson co -P data</pre>
    And then you need to link the admin dir from the code directory:
  <pre>  cd data && ln -s ../code/admin admin</pre>
    And then you can configure, compile and install them just like normal release (see
    <a href=\"install.php\">installation page</a>) except that you can skip unpacking
    command (tar) and you have to execute
  <pre>  make -f Makefile.cvs</pre>
    before configuring.");
draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
