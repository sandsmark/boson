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


function sidebar_begin()
{
echo "
<!-- Begin sidebar -->
<td>
  <table width=\"200\" cellpadding=\"0\" cellspacing=\"0\" class=\"sidebar\">";
}

function sidebar_end()
{
echo "
<!-- End sidebar -->
  </table>
</td>";
}

function sidebar_box_begin($title)
{
echo "
<!-- Begin sidebar box with title $title -->
<tr><td>
  <table cellpadding=\"2\" cellspacing=\"1\" border=\"0\" width=\"100%\" class=\"sidebarbox\">
    <tr><td class=\"sidebarboxtitlecell\">
      <font class=\"sidebarboxtitle\">&nbsp;$title</font>
    </td></tr>
    <tr><td class=\"sidebarboxcell\">";
}

function sidebar_box_end()
{
echo "
    <!-- End sidebar box -->
    </td></tr>
  </table>
  <br>
</td></tr>";
}

function draw_link($title, $href, $style = "sidebarboxlink")
{
global $basedir;
echo "
&nbsp;&nbsp;<a class=\"$style\" href=\"$basedir$href\" $params>$title</a><br>";
}

function draw_extended_link($title, $href, $params, $style = "sidebarboxlink")
{
echo "
&nbsp;&nbsp;<a class=\"$style\" href=\"$href\" $params>$title</a><br>";
}

function sidebar_links_box()
{
sidebar_box_begin("Links");
draw_link("Main page", "index.php");
echo "<br>";
draw_link("All news", "all-news.php");
draw_link("Screenshots", "screenshots.php");
draw_link("Download", "download.php");
draw_link("Install", "install.php");
draw_link("More information", "info.php");
draw_extended_link("FAQ", "http://www.freehackers.org/boson/handbook/faq.html", "target=\"_blank\"");
echo "<br>";
draw_link("Announcements", "announcements.php");
draw_link("Stories", "stories.php");
echo "<br>";
draw_link("Status", "status.php");
draw_link("Contact us", "contact.php");
echo "<br>";
draw_link("Related links", "links.php");
draw_link("Change style", "options.php");
sidebar_box_end();
}

function sidebar_old_news()
{
sidebar_box_begin("Older News");
display_old_news();
sidebar_box_end();
}



function sidebar_stats_box()
{
global $lastupdate;
global $filename;
global $basedir;
global $style;

sidebar_box_begin("<a class=\"statslink\" href=\"${basedir}stats.php\">Statistics</a>");

// Last update
echo "
<font class=\"lastupdate\">This page was last updated on<br>
<font class=\"lastupdatevalue\">$lastupdate GMT</font>.<br></font>";

// Counter
counter();
counter2($filename);

// "Valid html" logo
/*echo "
<br>
<!-- We have Valid HTML 4.01! -->
<a href=\"http://www.sourceforge.net/projects/boson\" target=\"_blank\">
  <img style=\"border:0;width:114px;height:34px\"
       src=\"${basedir}pictures/sflogo.png\"
       alt=\"SF Project Home\"></a><br><br>
<a href=\"http://validator.w3.org/check/referer\" target=\"_blank\">
<img border=\"0\" src=\"${basedir}pictures/valid-html401.png\"
    alt=\"Valid HTML 4.01!\" height=\"31\" width=\"88\"></a><br>
<!-- We also have Valid CSS! -->
<a href=\"http://jigsaw.w3.org/css-validator/validator?uri=http://boson.sourceforge.net/style-${style}.css\" target=\"_blank\">
  <img style=\"border:0;width:88px;height:31px\"
       src=\"${basedir}pictures/valid-css.png\"
       alt=\"Valid CSS!\"></a>";*/

// Copyright
echo "
<br><br>
<font class=\"copyright\">(C) 2002-2003  <a class=\"copyright\" href=\"mailto:boson-devel@lists.sourceforge.net\">
The Boson Team</a></font><br>";

sidebar_box_end();
}

function sidebar_download_box()
{
global $latestversion, $latestversiondate;
global $basedir;
sidebar_box_begin("Download");
echo "
    Latest stable version: <font class=\"sidebardownloadversion\">$latestversion</font><br>
    Released: <font class=\"sidebardownloadversion\">$latestversiondate</font><br>
    <a class=\"sidebardownloadlink\" href=\"${basedir}download.php\">Download now!</a>";
sidebar_box_end();
}

function sidebar_announcements_box()
{
sidebar_box_begin("Announcements");
draw_link("Boson 0.6", "announces/boson-0.6.php");
draw_link("Boson 0.6.1", "announces/boson-0.6.1.php");
draw_link("Boson 0.7", "announces/boson-0.7.php");
draw_link("Boson 0.8", "announces/boson-0.8.php");
sidebar_box_end();
}

function sidebar_stories_box()
{
sidebar_box_begin("Boson stories");
draw_link("Story #1&nbsp;&nbsp;(Oct. 17, 2002)", "stories/story-20021017.php");
draw_link("Story #2&nbsp;&nbsp;(Nov. 08, 2002)", "stories/story-20021108.php");
draw_link("Story #3&nbsp;&nbsp;(March 31, 2003)", "stories/story-20030331.php");
sidebar_box_end();
}


?>
