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
$filename="index.php";

/*****  Some includes  *****/
include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");
include_once("news.inc");
include_once("poll.inc");

/*****  Start of main stuff  *****/

start_page("Main page");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
    sidebar_box_begin("Poll");
      $id = poll_last_id();
      if($HTTP_COOKIE_VARS["voted-$id"] == "yes")
      {
        // User has already voted, show small version of results
        poll_results_small($id);
      }
      else
      {
        poll_show($id);
      }
    sidebar_box_end();
  sidebar_old_news();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// About
draw_bigbox_begin("About Boson");
echo "<tr><td>Boson is an OpenGL real-time strategy game.
  It is designed to run on Unix (Linux) computers, and is built on top of the
  KDE, Qt and kdegames libraries.</td></tr>";
draw_bigbox_end();

// About
draw_bigbox_begin("Help wanted!!!");
echo "<tr><td>We are in need of additional team members, especially 3d designers.<br>
  If you think you can help, please
  <a href=\"mailto:boson-devel@__NO_SPAM__lists.sourceforge.net\">contact us</a>.<br>
  You can find a list of jobs <a href=\"/wiki/Main/Jobs\">in our wiki</a>.</td></tr>";
draw_bigbox_end();

/*// We need you!
draw_bigbox_begin("We need you!");
echo "<tr><td>If you have any spare time and are willing to help us, please
    <a href=\"contact.php\">contact us</a>. We are sure we can find job for
    you!<br>
    We are especially in need of <b>coders</b> (who can code in C++ and advisably can
    use Qt and KDE) and <b>artists</b> (who can draw textures and make 3d models), but
    we can also make use of documentation writers, translators and, of course,
    testers.<br><br></td></tr>";
draw_bigbox_end();*/

// Print news
news_box_begin();
/**
** If you want to add news article, add it to the beginning of news.text.
**/
display_main_news();
news_box_end();

main_area_end();

end_page();


?>
