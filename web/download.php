<?php
/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


/*****  Variables  *****/
$filename="download.php";

/*****  Some includes  *****/
include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");

/*****  Start of main stuff  *****/

if(array_key_exists("dl", $_GET))
{
  // User wanted to download something.
  // Count download and redirect browser
  $dl = $_GET['dl'];
  counter2_download($dl);
  header("Location: http://prdownloads.sourceforge.net/boson/".$dl."?download");
  exit;
}

start_page("Downloads");

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

/*draw_bigbox_subheader("Installer");
draw_bigbox_text("This is the recommended way of getting Boson.<br>
It includes mostly static binary of Boson which means you won't have to
install anything else.<br>
You ca download it from
<a href=\"download.php?dl=boson-0.11-x86.run\">here</a>
(79 164 KB) or via SF.net's web interface from
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=353607\">here</a>.<br><br>");
*/

draw_bigbox_subheader("All-in-one package");
draw_bigbox_text("This is a big package that contains source code, data files
and music. You do not need to download any package below if you download this
one. You can get it from
<a href=\"download.php?dl=boson-all-0.13.tar.bz2\">here</a>
(36 177 KB) or via SF.net's web interface from
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&package_id=123796release_id=451731\">here</a>.<br><br>");

draw_bigbox_subheader("Source code");
draw_bigbox_text("You can download a tarball (.tar.bz2) with the code from
<a href=\"download.php?dl=boson-code-0.13.tar.bz2\">here</a>
(1795 KB) or via SF.net's web interface from
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&package_id=12379&release_id=451731\">here</a>.
Note that you also need to download the data package to play Boson.<br><br>");

draw_bigbox_subheader("Data package");
draw_bigbox_text("This tarball contains the data files needed for playing Boson.
You can download it from
<a href=\"download.php?dl=boson-data-0.13.tar.bz2\">here</a>
(18 080 KB) or via SF.net's web interface from
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&package_id=12379&release_id=451731\">here</a>.<br><br>");

draw_bigbox_subheader("Music");
draw_bigbox_text("You don't need the music package to play Boson, but it's
recommended. :-) You can download the tarball from
<a href=\"download.php?dl=boson-music-0.13.tar.bz2\">here</a>
(16 320 KB) or via SF.net's web interface from
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&package_id=12379&release_id=451731\">here</a>.<br><br>");

/*draw_bigbox_subheader("Gentoo");
draw_bigbox_text("Boson-0.9.1 is in the official portage tree, unmasked.<br>
To get boson downloaded, compiled and installed, please issue a
<pre>emerge sync; emerge boson</pre>
If you're not familiar with <i>emerge</i>, see
<a href=\"http://www.gentoo.org/doc/en/portage-user.xml\">http://www.gentoo.org/doc/en/portage-user.xml</a>
for more info.<br><br>");*/

/*draw_bigbox_subheader("Debian");
draw_bigbox_text("To install Boson on Debian, do this:
<pre>apt-get update; apt-get install boson</pre>
This will download and install Boson's binary packages, so you'll have Boson up
and running in just few minutes.<br>
Note that you should also have something like
<pre>deb http://ftp2.de.debian.org/debian sid main</pre>
in your <i>/etc/apt/sources.list</i> file.<br><br>");*/

/*draw_bigbox_subheader("Redhat");
draw_bigbox_text("RPM packages are available for Redhat.<br>
There are separate packages for code, data and music; you need at least code
and data to be able to run Boson.
You can download those packages from
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&package_id=12379&release_id=197716\">SF.net download page</a>
(files with .rpm extension).
<br><br>");*/

/*draw_bigbox_subheader("RPM/binary releases");
draw_bigbox_text("Note that RPM packages are not provided by the Boson team and
are inofficial. If you feel like building a RPM for boson (for any platform),
we'll be happy to put a link here and/or add them to our site.<br><br>You can
get a RPM package of Boson from
<a href=\"http://prdownloads.sourceforge.net/boson/boson-0.7-1.i386.rpm?download\">here</a>
(38 140 KB) or from
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=121263\">SF.net</a>.
This package is compiled for Red Hat 7.3 on i386.<br><br>
We also have a RPM source package which is like all-in-one package, just that
it's packaged with RPM. You can get it from
<a href=\"http://prdownloads.sourceforge.net/boson/boson-0.7-1.src.rpm?download\">here</a>
(36 293 KB) or from
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=121263\">SF.net</a>
again. These package were provided by
<a href=\"mailto:krzyko@users.sourceforge.net\">Krzysztof Kosz</a><br><br>");*/

draw_bigbox_subheader("Older versions");
draw_bigbox_text("Currently, you can download old releases only via
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087\">SF.net's web interface</a>.<br>
Available versions are:<br>
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=419522\">Boson 0.12</a>
(released on 27th May 2006),<br>
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=353607\">Boson 0.11</a>
(released on 3rd September 2005),<br>
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=235412\">Boson 0.10</a>
(released on 2nd May 2004),<br>
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=197716\">Boson 0.9.1</a>
(released on 16th November 2003),<br>
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=194601\">Boson 0.9</a>
(released on 3rd November 2003),<br>
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=149745\">Boson 0.8</a>
(released on 31st March 2003),<br>
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=121263\">Boson 0.7</a>
(released on 10th November 2002),<br>
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=98577\">Boson 0.6.1</a>
(released on 9th July 2002),<br>
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=93970\">Boson 0.6</a>
(released on 10th June 2002) and<br>
<a href=\"http://sourceforge.net/project/showfiles.php?group_id=15087&amp;release_id=17261\">Boson 0.5</a>
(released on 30th October 2000).<br><br>");

draw_bigbox_subheader("<a name=\"cvs\"></a><a name=\"svn\"></a>SVN - Bleeding edge development version");
draw_bigbox_text("You can get both code and data from SVN using the following commands:
<pre>
$ svn co https://svn.sourceforge.net/svnroot/boson/trunk/code code
$ svn co https://svn.sourceforge.net/svnroot/boson/trunk/data data
</pre>
You need <a href=\"http://www.cmake.org\">CMake</a> for compilation. In both code/ and
data/ directory, use these commands:
<pre>
$ mkdir build
$ cd build
$ cmake ..
$ make
$ su
# make install
</pre>
When configuring fails, check whether you have all the
<a href=\"info.php\">software dependencies</a> installed.
<br>To stay up to date, you can update both code and data directories later on
by executing
<pre>$ svn up</pre><br>");

draw_bigbox_end();

main_area_end();

end_page();


?>
