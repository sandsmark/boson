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


$basedir="";
$filename="stats.php";

include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");


start_page("Statistics");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

  if($show == "")
  {
    // show everything
    counter2_statspage($file);
  }
  else
  {
    // Show specific box

    // init counter2
    counter2_init();

    // get number of all clicks
    $query = "SELECT * FROM counter_total WHERE id = 'clicks'";
    $result = mysql_query($query);
    $row = mysql_fetch_assoc($result);
    $total = $row['count'];

    $options = "";

    // Items Per Page, maybe make configurable (from web page)
    if($ipp == "")
    {
      // If not given, fall back to default
      $ipp = 40;
    }
    else
    {
      $options .= "&ipp=$ipp";
    }

    // Current page
    if($page == "")
    {
      // If not given, fall back to default
      $page = 1;
    }

    if($show == "pages")
    {
      counter2_statsbox("Pages", "Page", "Views", "counter_pages", "page", "ORDER BY count DESC", $total,
          "${file}?show=pages${options}", $page, true, $ipp, ($page - 1) * $ipp);
    }
    else if($show == "days")
    {
      counter2_statsbox("Days", "Date", "Views", "counter_days", "date", "ORDER BY date DESC", $total,
          "${file}?show=days${options}", $page, false, $ipp, ($page - 1) * $ipp);
    }
    else if($show == "browsers")
    {
      counter2_statsbox("Browsers", "Browser", "Views", "counter_browser", "id", "ORDER BY count DESC", $total,
          "${file}?show=browsers${options}", $page, false, $ipp, ($page - 1) * $ipp);
    }
    else if($show == "os")
    {
      counter2_statsbox("Operating systems", "OS", "Views", "counter_os", "id", "ORDER BY count DESC", $total,
          "${file}?show=os${options}", $page, false, $ipp, ($page - 1) * $ipp);
    }
    else if($show == "downloads")
    {
      counter2_statsbox("Downloads", "File", "Downloads", "counter_download", "file", "ORDER BY count DESC", 0,
          "${file}?show=downloads${options}", $page, false, $ipp, ($page - 1) * $ipp);
    }

    // Show link to stats page
    echo "
    <a href=\"${basedir}stats.php\">All statistics</a>";
  }

main_area_end();

end_page();

?>
