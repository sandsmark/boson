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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


/*****  Variables  *****/
$filename="screenshots.php";
$screenshotsdir="shots/";

/*****  Some includes  *****/
include("common.php");
include("sidebar.php");
include("main.php");
include("counter.php");
include("boxes.php");
include("variables.php");

/*****  Functions  *****/
function draw_screenshot($description, $date, $thumbfile, $bigfile)
{
global $screenshotsdir;
$thumb=$screenshotsdir . $thumbfile;
$big=$screenshotsdir . $bigfile;
echo "
<td align=\"center\" width=\"50%\">
  <a href=\"$big\"><img border=\"0\" src=\"$thumb\" alt=\"$date\"></a><br>
  <font class=\"screenshotdate\">$date</font><br>
  <font class=\"screenshotdesc\">$description</font><br><br>
</td>";
}

/*****  Start of main stuff  *****/

do_start_stuff();

// Headers
html_print_header("Screenshots");
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

// Screenshots stuff
draw_bigbox_begin("Screenshots");
echo "
<!-- Screenshots section -->
<tr>";
echo "
    <tr><td>
      Click on the thumbnails to see bigger versions. Screenshots taken from CVS
      are from the development version.
      Unit models and other things are not final.<br><br>
    </td><td></td></tr>";

/** Add screenshots here!
**  To add screenshot, call
**  draw_screenshot(<description>, <date added>, <thumbnail filename>, <big version filename>);
**  Note that filenames do not contain directory and that files are in JPG AND NOT IN PNG because of the size
**/
echo "<tr>";

draw_screenshot("Smoking refineries",
    "NEW: 31. Dec. 2002 (CVS)", "smoke_refineries_thumb.jpg", "smoke_refineries.jpg");
draw_screenshot("The big war",
    "NEW: 31. Dec. 2002 (CVS)", "big_war_thumb.jpg", "big_war.jpg");
echo "
</tr>
<tr>";
draw_screenshot("Blowing up Command center of the enemy",
    "10. Nov. 2002 (Boson 0.7)", "0.7-1-thumb.jpg", "0.7-1.jpg");
draw_screenshot("Smoking wreckages after a failed attack",
    "10. Nov. 2002 (Boson 0.7)", "0.7-2-thumb.jpg", "0.7-2.jpg");
echo "
</tr>
<tr>";
draw_screenshot("Here you can see much smoke from wreckages and missiles and new camera system",
    "10. Nov. 2002 (Boson 0.7)", "0.7-3-thumb.jpg", "0.7-3.jpg");
draw_screenshot("",
    "10. Nov. 2002 (Boson 0.7)", "0.7-4-thumb.jpg", "0.7-4.jpg");
echo "
</tr>
<tr>";
draw_screenshot("",
    "10. Nov. 2002 (Boson 0.7)", "0.7-5-thumb.jpg", "0.7-5.jpg");
draw_screenshot("Outer defenses of an enemy being taken out",
    "10. Nov. 2002 (Boson 0.7)", "0.7-6-thumb.jpg", "0.7-6.jpg");
echo "
</tr>
<tr>";
draw_screenshot("Here you can see an attack of an aircraft while rotating the
    camera. Also, you can see the new particle effects.",
    "09. Sep. 2002 (CVS)", "thumb_war1.jpg", "war1.jpg");
draw_screenshot("The Big War - showing more particle effects, units and gameplay.",
    "09. Sep. 2002 (CVS)", "thumb_war2.jpg", "war2.jpg");
echo "
</tr>
<tr>";
draw_screenshot("This shows the new OpenGL support and camera rotation.",
    "17. Aug. 2002 (CVS)", "thumb_gl1.jpg", "gl_boson1.jpg");
draw_screenshot("Buildings are getting constructed.",
    "17. Aug. 2002 (CVS)", "thumb_gl2.jpg", "gl_boson2.jpg");
echo "
</tr>
<tr>";
draw_screenshot("This screenshot shows a running game in Boson. You can see many
    units and a commandframe on the left showing selected unit's properties.",
    "11. June 2002 (Boson 0.6)", "thumb1.jpg", "boson1.jpg");
draw_screenshot("This is Boson's startup screen. Currently, here are only
    buttons to start a new game or quit Boson.",
    "11. June 2002 (Boson 0.6)", "thumb2.jpg", "boson2.jpg");
echo "
</tr>
<tr>";
draw_screenshot("Screenshot of the \"Start new game\" page. You can choose your name,
    color, species and map here, add computer players and even chat with other connected players.",
    "11. June 2002 (Boson 0.6)", "thumb3.jpg", "boson3.jpg");
draw_screenshot("Another screenshot of a running game. You can see some moving
    units here. In the lower part of the window is a chat widget, you can chat
    with other players during the game.",
    "11. June 2002 (Boson 0.6)", "thumb4.jpg", "boson4.jpg");
echo "
</tr>";
draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
