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

$show_ss=-1;
$max_screens_per_page=10;
$page=1;

class Screenshot
{
  var $description;
  var $date;
  var $thumbfile;
  var $bigfile;
  var $id;

  function set_description($arg)  { $this->description = $arg; }
  function set_date($arg)  { $this->date = $arg; }
  function set_thumbfile($arg)  { $this->thumbfile = $arg; }
  function set_bigfile($arg)  { $this->bigfile = $arg; }
  function set_id($arg)  { $this->id = $arg; }

  function Screenshot()
  {
      $this->id = -1;
      $this->description = "";
      $this->date = "";
      $this->thumbfile = "";
      $this->big_file = "";
  }
  function print_info()
  {
      echo "<hr>";
      echo "desc = $this->description";
      echo "<br>";
      echo "date = $this->date";
      echo "<br>";
      echo "thumfule = $this->thumbfile";
      echo "<br>";
      echo "bigfile = $this->bigfile";
      echo "<br>";
      echo "id = $this->id";
      echo "<br>";
  }

  function draw_thumb()
  {
    global $screenshotsdir;
    $thumb=$screenshotsdir . $this->thumbfile;
    $big=$screenshotsdir . $this->bigfile;
    echo "
    <td align=\"center\" valign=\"top\" width=\"50%\">
      <a href=\"screenshots.php?show=$this->id\"><img border=\"0\" src=\"$thumb \" alt=\" $this->date \"></a><br>
      <font class=\"screenshotdate\">$this->date</font><br>
      <font class=\"screenshotdesc\">$this->description</font><br><br>
    </td>";
  }

  function draw_big()
  {
    global $screenshotsdir;
    global $ss_count;
    $big=$screenshotsdir . $this->bigfile;
    echo "
    <tr><td align=\"center\" width=\"100%\">
      <img border=\"0\" src=\"$big\" alt=\"$this->date\"><br>
      <font class=\"screenshotdate\">$this->date</font><br>
      <font class=\"screenshotdesc\">$this->description</font><br><br>";
    if($this->id > 0)
    {
      $nextid = $this->id - 1;
      echo "
      <a href=\"screenshots.php?show=$nextid\">&lt;&nbsp;Next</a>&nbsp;&nbsp;";
    }
    echo "
      <a href=\"screenshots.php\">Back to screenshots page</a>";
    if($this->id < $ss_count - 1)
    {
      $previd = $this->id + 1;
      echo "
      &nbsp;&nbsp;<a href=\"screenshots.php?show=$previd\">Previous&nbsp;&gt;</a>";
    }
    echo "
    </td></tr>";
  }
}

$screens[] = new Screenshot();
$ss_index = 0;

function add_screenshot($description, $date, $thumbfile, $bigfile)
{
  global $screens;
  global $ss_index;
  //$ss = &$screens[$ss_index];
  $ss = new Screenshot();
  $ss->set_description($description);
  $ss->set_date($date);
  $ss->set_thumbfile($thumbfile);
  $ss->set_bigfile($bigfile);
  $ss->set_id($ss_index);
  $screens[$ss_index++]=$ss;
}


/*****  Functions  *****/

// show
if($HTTP_GET_VARS["show"] != "")
{
  $show_ss=$HTTP_GET_VARS["show"];
}
// page
if($HTTP_GET_VARS["page"] != "")
{
  $page=$HTTP_GET_VARS["page"];
}

/** Add screenshots here!
**  To add screenshot, call
**  add_screenshot(<description>, <date added>, <thumbnail filename>, <big version filename>);
**  Note that filenames do not contain directory and that files are in JPG AND NOT IN PNG because of the size
**/
add_screenshot("Smoking refineries",
    "NEW: 31. Dec. 2002 (CVS)", "smoke_refineries_thumb.jpg", "smoke_refineries.jpg");
add_screenshot("The big war",
    "NEW: 31. Dec. 2002 (CVS)", "big_war_thumb.jpg", "big_war.jpg");
add_screenshot("Outer defenses of an enemy being taken out",
    "10. Nov. 2002 (Boson 0.7)", "0.7-6-thumb.jpg", "0.7-6.jpg");
add_screenshot("",
    "10. Nov. 2002 (Boson 0.7)", "0.7-5-thumb.jpg", "0.7-5.jpg");
add_screenshot("",
    "10. Nov. 2002 (Boson 0.7)", "0.7-4-thumb.jpg", "0.7-4.jpg");
add_screenshot("Here you can see much smoke from wreckages and missiles and new camera system",
    "10. Nov. 2002 (Boson 0.7)", "0.7-3-thumb.jpg", "0.7-3.jpg");
add_screenshot("Smoking wreckages after a failed attack",
    "10. Nov. 2002 (Boson 0.7)", "0.7-2-thumb.jpg", "0.7-2.jpg");
add_screenshot("Blowing up Command center of the enemy",
    "10. Nov. 2002 (Boson 0.7)", "0.7-1-thumb.jpg", "0.7-1.jpg");
add_screenshot("The Big War - showing more particle effects, units and gameplay.",
    "09. Sep. 2002 (CVS)", "thumb_war2.jpg", "war2.jpg");
add_screenshot("Here you can see an attack of an aircraft while rotating the
    camera. Also, you can see the new particle effects.",
    "09. Sep. 2002 (CVS)", "thumb_war1.jpg", "war1.jpg");
add_screenshot("Buildings are getting constructed.",
    "17. Aug. 2002 (CVS)", "thumb_gl2.jpg", "gl_boson2.jpg");
add_screenshot("This shows the new OpenGL support and camera rotation.",
    "17. Aug. 2002 (CVS)", "thumb_gl1.jpg", "gl_boson1.jpg");
add_screenshot("Another screenshot of a running game. You can see some moving
    units here. In the lower part of the window is a chat widget, you can chat
    with other players during the game.",
    "11. June 2002 (Boson 0.6)", "thumb4.jpg", "boson4.jpg");
add_screenshot("This screenshot shows a running game in Boson. You can see many
    units and a commandframe on the left showing selected unit's properties.",
    "11. June 2002 (Boson 0.6)", "thumb1.jpg", "boson1.jpg");
add_screenshot("Screenshot of the \"Start new game\" page. You can choose your name,
    color, species and map here, add computer players and even chat with other connected players.",
    "11. June 2002 (Boson 0.6)", "thumb3.jpg", "boson3.jpg");
add_screenshot("This is Boson's startup screen. Currently, here are only
    buttons to start a new game or quit Boson.",
    "11. June 2002 (Boson 0.6)", "thumb2.jpg", "boson2.jpg");

$ss_count = count($screens);

/*****  Start of main stuff  *****/

do_start_stuff();

if($show_ss == -1)
{
    // Showing thumbs of all screenshots
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
    $index = ($page - 1) * $max_screens_per_page;
    $last = $index + $max_screens_per_page;
    $firstnum = $index + 1;
    if($last > $ss_count)
    {
      $lastnum = $ss_count;
    }
    else
    {
      $lastnum = $last;
    }
    draw_bigbox_begin("Screenshots $firstnum-$lastnum of $ss_count");
    echo "
    <!-- Screenshots section -->";
    echo "
        <tr><td width=\"0%\">
          Click on the thumbnails to see bigger versions. Screenshots taken from CVS
          are from the development version.
          Unit models and other things are not final.<br><br>
        </td></tr>
        <tr><td><table width=\"100%\">";
    while ($index < sizeof($screens) and $index < $last)
    {
        echo "<tr>";
            $screens[$index + 1]->draw_thumb();
            $screens[$index]->draw_thumb();
        echo "</tr>";
        $index=$index+2;
    }
    echo "</table></tr></td>";

    echo "
      <tr><td width=\"100%\" align=\"right\"><br>Page:";
    $pageindex = 0;
    while ($pageindex * $max_screens_per_page < $ss_count)
    {
      $pagenum = $pageindex + 1;
      if($pagenum == $page)
      {
        echo "&nbsp;<b>$pagenum</b>";
      }
      else
      {
        echo "&nbsp;<b><a href=\"screenshots.php?page=$pagenum\">$pagenum</a></b>";
      }
      $pageindex++;
    }
    echo "
      </td></tr>";
    
    draw_bigbox_end();
    
    main_area_end();
    main_table_end();
    
    // Footers
    print_footer();
    html_print_footer();
}
else
{
    // Showing specified screenshot
    // Headers
    $ssnum = $show_ss + 1;
    html_print_header("Screenshot $ssnum");
    print_header();
    
    // Main table
    main_table_begin();
    
    // No sidebar to save space
    
    main_area_begin();
    
    // Screenshots stuff
    draw_bigbox_begin("Screenshot $ssnum");
    echo "
    <!-- Screenshot -->";
    
    $screens[$show_ss]->draw_big();
    
    draw_bigbox_end();
    
    main_area_end();
    main_table_end();
    
    // Footers
    print_footer();
    html_print_footer();
}

?>
