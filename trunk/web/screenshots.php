<?php


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
**  Note that filenames do not contain directory
**/
draw_screenshot("Here you can see an attack of an aircraft while rotating the
    camera. Also, you can see the new particle effects.",
    "NEW: 09. Sep. 2002 (CVS)", "thumb_war1.png", "war1.png");
draw_screenshot("The Big War - showing more particle effects, units and gameplay.",
    "NEW: 09. Sep. 2002 (CVS)", "thumb_war2.png", "war2.png");
echo "
</tr>
<tr>";
draw_screenshot("This shows the new OpenGL support and camera rotation.",
    "17. Aug. 2002 (CVS)", "thumb_gl1.png", "gl_boson1.png");
draw_screenshot("Buildings are getting constructed.",
    "17. Aug. 2002 (CVS)", "thumb_gl2.png", "gl_boson2.png");
echo "
</tr>
<tr>";
draw_screenshot("This screenshot shows a running game in Boson. You can see many
    units and a commandframe on the left showing selected unit's properties.",
    "11. June 2002 (Boson 0.6)", "thumb1.png", "boson1.png");
draw_screenshot("This is Boson's startup screen. Currently, here are only
    buttons to start a new game or quit Boson.",
    "11. June 2002 (Boson 0.6)", "thumb2.png", "boson2.png");
echo "
</tr>
<tr>";
draw_screenshot("Screenshot of the \"Start new game\" page. You can choose your name,
    color, species and map here, add computer players and even chat with other connected players.",
    "11. June 2002 (Boson 0.6)", "thumb3.png", "boson3.png");
draw_screenshot("Another screenshot of a running game. You can see some moving
    units here. In the lower part of the window is a chat widget, you can chat
    with other players during the game.",
    "11. June 2002 (Boson 0.6)", "thumb4.png", "boson4.png");
echo "
</tr>";
draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
