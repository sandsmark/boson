<?php
/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
$filename="movies.php";

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


start_page("Movies");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("The movies");
draw_bigbox_subheader("WARNING");
draw_bigbox_text("Those movies are more than two years old and thus do NOT
    reflect Boson's cirrent state.<br>
    The <a href=\"screenshots.php\">screenshots</a> give a better idea of
    Boson's current look.
    <br><br>");

draw_bigbox_text("You can download the Boson-movies here.<br>
    They are rendered completely in Boson, except for intro/outro sequences<br>
    They do not show actual gameplay, as scripts were used to make them, but
    the game itself looks very similar.<br>
    <br>
    The movies come in three different flavors:<br>
    <ul>
      <li>High quality - has 800x600 resolution and best quality, but is
        also quite big and requires decent computer to play. Recommended for
        users with fast internet connections.</li>
      <li>Medium quality - resolution is 640x480, but it doesn't look
        much worse than high quality one.</li>
      <li>Low quality - with 320x240 resolution, quality isn't the best,
        but it's good for modem users and for previewing before downloading the
        high-quality version.</li>
    </ul>
    All movies are encoded using <a href=\"http://www.xvid.org\">XViD codec</a>.<br>
    Unfortunately, the movies have no sound.
    <br><br>");

draw_bigbox_subheader("Movie 1");
draw_bigbox_text("This is the first movie<br>
    It's lenght is 1:39 and it consists of two scenes. First scene shows an
    attack on enemy's base in desert and second one shows deadly Transall bomber
    blowing up another little base.<br>
    It's rendered with pre-0.9 version of Boson from the CVS and released on
    15th September 2003.<br>
    Select one of three versions to download:<br>
    <ul>
      <li><a href=\"download.php?dl=movie1-high.avi\">Download high-quality version</a> (40878 KB)</li>
      <li><a href=\"download.php?dl=movie1-medium.avi\">Download medium-quality version</a> (26179 KB)</li>
      <li><a href=\"download.php?dl=movie1-low.avi\">Download low-quality version</a> (6578 KB)</li>
    </ul>
    Screenshots (click on thumbnails for full versions):<br>
    &nbsp;&nbsp;
    <a href=\"shots/movie1-1.jpg\"><img border=\"0\" src=\"shots/movie1-1-thumb.jpg\" alt=\"Screenshot 1\"></a>
    <a href=\"shots/movie1-2.jpg\"><img border=\"0\" src=\"shots/movie1-2-thumb.jpg\" alt=\"Screenshot 2\"></a><br>
    <br>");
draw_bigbox_subheader("Movie 2");
draw_bigbox_text("This is the second movie<br>
    Lenght is 2:56. It shows a big attack to the enemy's base with
    tens of fighting units (nice big war ;-)).<br>
    It's rendered with pre-0.9 version of Boson from the CVS and released on
    15th September 2003.<br>
    Select one of three versions to download:<br>
    <ul>
      <li><a href=\"download.php?dl=movie2-high.avi\">Download high-quality version</a> (82613 KB)</li>
      <li><a href=\"download.php?dl=movie2-medium.avi\">Download medium-quality version</a> (52893 KB)</li>
      <li><a href=\"download.php?dl=movie2-low.avi\">Download low-quality version</a> (13283 KB)</li>
    </ul>
    Screenshots (click on thumbnails for full versions):<br>
    &nbsp;&nbsp;
    <a href=\"shots/movie2-1.jpg\"><img border=\"0\" src=\"shots/movie2-1-thumb.jpg\" alt=\"Screenshot 1\"></a>
    <a href=\"shots/movie2-2.jpg\"><img border=\"0\" src=\"shots/movie2-2-thumb.jpg\" alt=\"Screenshot 2\"></a><br>
    ");
draw_bigbox_end();

main_area_end();

end_page();


?>
