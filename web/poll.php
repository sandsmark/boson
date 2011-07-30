<?php
/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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


$basedir="";
$filename="poll.php";

include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");
include_once("poll.inc");


if($action == "vote" && $answer != "")
{
  // Vote. This cannot be done later as it uses cookies
  if($_COOKIE["voted-$id"] == "yes")
  {
    // User has already voted
    $votetext = "\nYou have already voted!<br>
    You cannot vote the same poll several times!";
  }
  else
  {
    $voteresult = poll_vote($id, $answer);
    if($voteresult == "")
    {
      // Error. Probably poll wasn't found
      $votetext = "\nPoll with id $id was not found!";
    }
    else
    {
      $votetext = "\nYou voted for '<font class=\"pollyourvote\">".$voteresult."</font>'";
      // Set cookie to prevent user from voting again
      setcookie("voted-$id", "yes", time() + 3600 * 24 * 365 * 25);
    }
  }
}

start_page("Poll");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

  draw_bigbox_begin("Poll");
  echo "\n<tr><td>";

  if($id == "")
  {
    $id = poll_last_id();
  }

  if($action == "vote")
  {
    // Vote
    if($answer == "")
    {
      // show poll
      poll_show($id);
    }
    else
    {
      // poll_vote has already been called. Display result
      echo "$votetext<br><br>";
      poll_results($id);
    }
  }
  else if($action == "results")
  {
    poll_results($id);
  }
  else if($action == "list")
  {
    poll_list();
  }
  else if($action == "create")
  {
    poll_create();
  }
  else if($action == "add")
  {
    poll_add($question, $answer1, $answer2, $answer3, $answer4, $answer5, $answer6, $answer7, $answer8, $answer9, $answer10);
    echo "
    <b>Poll added successfully</b>";
  }
  else
  {
    // show poll
    if($_COOKIE["voted-$id"] == "yes")
    {
      // User has already voted, show results
      poll_results($id);
    }
    else
    {
      // Show poll
      poll_show($id);
    }
  }

  echo "\n<td></tr>";
  draw_bigbox_end();

main_area_end();

end_page();

?>
