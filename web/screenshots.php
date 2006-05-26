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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


/*****  Variables  *****/
$filename="screenshots.php";
$screenshotsdir="shots/";

/*****  Some includes  *****/
include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");

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
    if($this->id < $ss_count - 1)
    {
      $nextid = $this->id + 1;
      echo "
      <a href=\"screenshots.php?show=$nextid\">&lt;&nbsp;Next</a>&nbsp;&nbsp;";
    }
    echo "
      <a href=\"screenshots.php\">Back to screenshots page</a>";
    if($this->id > 0)
    {
      $previd = $this->id - 1;
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
if(array_key_exists("show", $HTTP_GET_VARS))
{
  $show_ss=$HTTP_GET_VARS["show"];
}
// page
if(array_key_exists("page", $HTTP_GET_VARS))
{
  $page=$HTTP_GET_VARS["page"];
}

/** Add screenshots here!
**  To add screenshot, call
**  add_screenshot(<description>, <date added>, <thumbnail filename>, <big version filename>);
**  Add new calls to the _bottom_ of the list!!!
**  Note that filenames do not contain directory and that files are in JPG AND NOT IN PNG because of the size
**/


add_screenshot("This is Boson's startup screen. Currently, here are only
    buttons to start a new game or quit Boson.",
    "11. June 2002 (Boson 0.6)", "thumb2.jpg", "boson2.jpg");
add_screenshot("Screenshot of the \"Start new game\" page. You can choose your name,
    color, species and map here, add computer players and even chat with other connected players.",
    "11. June 2002 (Boson 0.6)", "thumb3.jpg", "boson3.jpg");
add_screenshot("This screenshot shows a running game in Boson. You can see many
    units and a commandframe on the left showing selected unit's properties.",
    "11. June 2002 (Boson 0.6)", "thumb1.jpg", "boson1.jpg");
add_screenshot("Another screenshot of a running game. You can see some moving
    units here. In the lower part of the window is a chat widget, you can chat
    with other players during the game.",
    "11. June 2002 (Boson 0.6)", "thumb4.jpg", "boson4.jpg");
add_screenshot("This shows the new OpenGL support and camera rotation.",
    "17. Aug. 2002 (CVS)", "thumb_gl1.jpg", "gl_boson1.jpg");
add_screenshot("Buildings are getting constructed.",
    "17. Aug. 2002 (CVS)", "thumb_gl2.jpg", "gl_boson2.jpg");
add_screenshot("Here you can see an attack of an aircraft while rotating the
    camera. Also, you can see the new particle effects.",
    "09. Sep. 2002 (CVS)", "thumb_war1.jpg", "war1.jpg");
add_screenshot("The Big War - showing more particle effects, units and gameplay.",
    "09. Sep. 2002 (CVS)", "thumb_war2.jpg", "war2.jpg");
add_screenshot("Blowing up Command center of the enemy",
    "10. Nov. 2002 (Boson 0.7)", "0.7-1-thumb.jpg", "0.7-1.jpg");
add_screenshot("Smoking wreckages after a failed attack",
    "10. Nov. 2002 (Boson 0.7)", "0.7-2-thumb.jpg", "0.7-2.jpg");
add_screenshot("Here you can see much smoke from wreckages and missiles and new camera system",
    "10. Nov. 2002 (Boson 0.7)", "0.7-3-thumb.jpg", "0.7-3.jpg");
add_screenshot("",
    "10. Nov. 2002 (Boson 0.7)", "0.7-4-thumb.jpg", "0.7-4.jpg");
add_screenshot("",
    "10. Nov. 2002 (Boson 0.7)", "0.7-5-thumb.jpg", "0.7-5.jpg");
add_screenshot("Outer defenses of an enemy being taken out",
    "10. Nov. 2002 (Boson 0.7)", "0.7-6-thumb.jpg", "0.7-6.jpg");
add_screenshot("The big war",
    "31. Dec. 2002 (CVS)", "big_war_thumb.jpg", "big_war.jpg");
add_screenshot("Smoking refineries",
    "31. Dec. 2002 (CVS)", "smoke_refineries_thumb.jpg", "smoke_refineries.jpg");
add_screenshot("0.8 includes new super-powerful DaisyCutter bomb",
    "1. April 2003 (Boson 0.8)", "0.8-1-thumb.jpg", "0.8-1.jpg");
add_screenshot("Tens of units fighting - that wasn't possible in 0.7",
    "1. April 2003 (Boson 0.8)", "0.8-2-thumb.jpg", "0.8-2.jpg");
add_screenshot("Big battle. Thanks to big performance improvements, it is
    possible to have much bigger battles in 0.8",
    "1. April 2003 (Boson 0.8)", "0.8-3-thumb.jpg", "0.8-3.jpg");
add_screenshot("Showing new terrain rendering and some units",
    "4. Aug. 2003 (CVS)", "terrain-1-thumb.jpg", "terrain-1.jpg");
add_screenshot("Another terrain shot, showing muddy coastline",
    "4. Aug. 2003 (CVS)", "terrain-2-thumb.jpg", "terrain-2.jpg");
add_screenshot("Some A-10 planes attacking enemy base",
    "4. Aug. 2003 (CVS)", "0308-1-thumb.jpg", "0308-1.jpg");
add_screenshot("A-10 blew up some tanks",
    "4. Aug. 2003 (CVS)", "0308-2-thumb.jpg", "0308-2.jpg");
add_screenshot("Bunch of enemies blown up",
    "4. Aug. 2003 (CVS)", "0308-3-thumb.jpg", "0308-3.jpg");
add_screenshot("Some Koyote choppers attacking enemy",
    "4. Aug. 2003 (CVS)", "0308-4-thumb.jpg", "0308-4.jpg");
add_screenshot("Leopard tank blown up",
    "4. Aug. 2003 (CVS)", "0308-5-thumb.jpg", "0308-5.jpg");
add_screenshot("Battle between Leopard tanks and Grizzlies",
    "4. Aug. 2003 (CVS)", "0308-6-thumb.jpg", "0308-6.jpg");
add_screenshot("War at night showing unit lighting. Smoke is probably too light,
    it should be darker.",
    "17. Oct. 2003 (CVS)", "0310-1-thumb.jpg", "0310-1.jpg");
add_screenshot("War at night. You can see units being lit correctly",
    "17. Oct. 2003 (CVS)", "0310-2-thumb.jpg", "0310-2.jpg");
add_screenshot("Looking down from the top of the mountain...",
    "17. Oct. 2003 (CVS)", "0310-3-thumb.jpg", "0310-3.jpg");
add_screenshot("New lighting and terrain rendering code in action. Note that
    camera is very high, so large area is visible",
    "17. Oct. 2003 (CVS)", "0310-4-thumb.jpg", "0310-4.jpg");
add_screenshot("Updated game starting page",
    "5. Nov. 2003 (Boson 0.9)", "0.9-1-thumb.jpg", "0.9-1.jpg");
add_screenshot("Some units attacking enemy",
    "5. Nov. 2003 (Boson 0.9)", "0.9-2-thumb.jpg", "0.9-2.jpg");
add_screenshot("Fragments flying around after an aircraft was blown up",
    "5. Nov. 2003 (Boson 0.9)", "0.9-3-thumb.jpg", "0.9-3.jpg");
add_screenshot("DaisyCutter bomb dropped by Transall bomber",
    "5. Nov. 2003 (Boson 0.9)", "0.9-4-thumb.jpg", "0.9-4.jpg");
add_screenshot("Close-up of some fighting units",
    "5. Nov. 2003 (Boson 0.9)", "0.9-5-thumb.jpg", "0.9-5.jpg");
add_screenshot("Many units fighting. You can also see tooltip, showing name and
    health of unit under cursor",
    "5. Nov. 2003 (Boson 0.9)", "0.9-6-thumb.jpg", "0.9-6.jpg");
add_screenshot("Neutral units such as trees are now supported. In the top-left
    corner, you can see new opengl minimap (it's showing only logo because
    player doesn't have radar)",
    "24. Dec. 2003 (CVS)", "0312-1-thumb.jpg", "0312-1.jpg");
add_screenshot("Lonely hut in the mountains, also some trees (including few
    broken ones) - they're also implemented as neutral units",
    "24. Dec. 2003 (CVS)", "0312-2-thumb.jpg", "0312-2.jpg");

add_screenshot("Harvesters are mining for minerals",
    "29. April 2004 (Boson 0.10-CVS)", "harvesting1-thumb.jpg", "harvesting1.jpg");
add_screenshot("Just a nice screenshot",
    "29. April 2004 (Boson 0.10-CVS)", "neutral1-thumb.jpg", "neutral1.jpg");

add_screenshot("Inside the enemy's base",
    "2. May 2004 (Boson 0.10)", "0.10-1-thumb.jpg", "0.10-1.jpg");
add_screenshot("An aircraft blown up by defenses on the Storm map",
    "2. May 2004 (Boson 0.10)", "0.10-2-thumb.jpg", "0.10-2.jpg");
add_screenshot("Some units are trying do destroy an enemy's tank",
    "2. May 2004 (Boson 0.10)", "0.10-3-thumb.jpg", "0.10-3.jpg");
add_screenshot("War between some units. Also note the new OpenGL minimap",
    "2. May 2004 (Boson 0.10)", "0.10-4-thumb.jpg", "0.10-4.jpg");

add_screenshot("Sam-site and an aircraft shooting each other",
    "23. December 2004 (CVS)", "0412-10-thumb.jpg", "0412-10.jpg");
add_screenshot("Some units attacking player's base",
    "23. December 2004 (CVS)", "0412-9-thumb.jpg", "0412-9.jpg");
add_screenshot("Close-up of some trees",
    "23. December 2004 (CVS)", "0412-8-thumb.jpg", "0412-8.jpg");
add_screenshot("New fog-of-war rendering method produces smooth transitions for terrain",
    "23. December 2004 (CVS)", "0412-7-thumb.jpg", "0412-7.jpg");
add_screenshot("Another screenshot of the water with some oil towers in the background",
    "23. December 2004 (CVS)", "0412-6-thumb.jpg", "0412-6.jpg");
add_screenshot("A guided missile flying towards a helicopter",
    "23. December 2004 (CVS)", "0412-5-thumb.jpg", "0412-5.jpg");
add_screenshot("A small lake with forest at one side",
    "23. December 2004 (CVS)", "0412-4-thumb.jpg", "0412-4.jpg");
add_screenshot("A burning house",
    "23. December 2004 (CVS)", "0412-3-thumb.jpg", "0412-3.jpg");
add_screenshot("New beautiful water with translucency, reflections and waves",
    "23. December 2004 (CVS)", "0412-2-thumb.jpg", "0412-2.jpg");
add_screenshot("A village conquered by enemy with some stuff burning and smoking",
    "23. December 2004 (CVS)", "0412-1-thumb.jpg", "0412-1.jpg");
add_screenshot("A village conquered by enemy with some stuff burning and smoking",
    "23. December 2004 (CVS)", "0412-1-thumb.jpg", "0412-1.jpg");


add_screenshot("And again, the new water rendering using shaders",
    "08. May 2005 (CVS)", "050508-5-thumb.jpg", "050508-5.jpg");
add_screenshot("Some selected units and water renderering using shaders",
    "08. May 2005 (CVS)", "050508-3-thumb.jpg", "050508-3.jpg");
add_screenshot("The new menu",
    "08. May 2005 (CVS)", "050508-4-thumb.jpg", "050508-4.jpg");
add_screenshot("The game is loading",
    "08. May 2005 (CVS)", "050508-2-thumb.jpg", "050508-2.jpg");
add_screenshot("The new 'start game' dialog",
    "08. May 2005 (CVS)", "050508-1-thumb.jpg", "050508-1.jpg");

add_screenshot("",
    "4. September 2005 (0.11)", "0.11-4-thumb.jpg", "0.11-4.jpg");
add_screenshot("",
    "4. September 2005 (0.11)", "0.11-3-thumb.jpg", "0.11-3.jpg");
add_screenshot("Enemy attacks",
    "3. September 2005 (0.11)", "0.11-2-thumb.jpg", "0.11-2.jpg");
add_screenshot("Some units fighting, also showing ground detail using shaders",
    "3. September 2005 (0.11)", "0.11-1-thumb.jpg", "0.11-1.jpg");

add_screenshot("New species",
    "8. February 2006 (CVS)", "0602-2-thumb.jpg", "0602-2.jpg");
add_screenshot("Some units from new species",
    "8. February 2006 (CVS)", "0602-1-thumb.jpg", "0602-1.jpg");

add_screenshot("Attack has begun",
    "<b>NEW:</b> 27. May 2006 (0.12)", "0.12-1-thumb.jpg", "0.12-1.jpg");
add_screenshot("Defending the base",
    "<b>NEW:</b> 27. May 2006 (0.12)", "0.12-2-thumb.jpg", "0.12-2.jpg");
add_screenshot("Specular highlights on units with high-quality shaders",
    "<b>NEW:</b> 27. May 2006 (0.12)", "0.12-3-thumb.jpg", "0.12-3.jpg");
add_screenshot("Attacking enemy's air units",
    "<b>NEW:</b> 27. May 2006 (0.12)", "0.12-4-thumb.jpg", "0.12-4.jpg");
add_screenshot("Battle for the oilfields... sounds familiar?",
    "<b>NEW:</b> 27. May 2006 (0.12)", "0.12-5-thumb.jpg", "0.12-5.jpg");
add_screenshot("Night is coming and the shadows are getting long",
    "<b>NEW:</b> 27. May 2006 (0.12)", "0.12-6-thumb.jpg", "0.12-6.jpg");

$ss_count = count($screens);

/*****  Start of main stuff  *****/

if($show_ss == -1)
{
    // Showing thumbs of all screenshots
    start_page("Screenshots");

    // Sidebar
    sidebar_begin();
      sidebar_links_box();
      sidebar_download_box();
      sidebar_stats_box();
    sidebar_end();

    main_area_begin();

    // Screenshots stuff
    $last = $ss_count - ($page - 1) * $max_screens_per_page;
    $first = $last - $max_screens_per_page + 1;
    if($first < 1)
    {
      $first = 1;
    }
    draw_bigbox_begin("Screenshots $first-$last of $ss_count");
    echo "
    <!-- Screenshots section -->";
    echo "
        <tr><td width=\"100%\">
          Click on the thumbnails to see bigger versions. Screenshots taken from CVS
          are from the development version.
          Unit models and other things are not final.<br><br>
        </td></tr>
        <tr><td><table width=\"100%\">";
    $index = $last - 1;
    while ($index >= ($first - 1))
    {
        echo "<tr>";
        $screens[$index]->draw_thumb();
        // If we have next screenshot, print this one, too
        if($screens[$index - 1])
        {
          $screens[$index - 1]->draw_thumb();
        }
        else
        {
          // Empty table cell
          echo "
    <td align=\"center\" valign=\"top\" width=\"50%\"></td>";
        }
        echo "</tr>";
        $index = $index - 2;
    }
    echo "</table></td></tr>";


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

    end_page();
}
else
{
    // Showing specified screenshot
    // Headers
    $ssnum = $show_ss + 1;
    // We're counting viewed screens as downloads
    counter2_download("Screenshot $ssnum");

    start_page("Screenshot $ssnum");

    // No sidebar to save space

    main_area_begin();

    // Screenshots stuff
    draw_bigbox_begin("Screenshot $ssnum");
    echo "
    <!-- Screenshot -->";

    $screens[$show_ss]->draw_big();

    draw_bigbox_end();

    main_area_end();

    end_page();
}

?>
