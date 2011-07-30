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


function counter($file = "")
{
global $counter_file;
if($file == "") {
  $file = $counter_file;
}
global $visitcount;
global $basedir;
$fp = fopen($basedir . $file, "r+");
flock($fp, LOCK_EX) or die ("Failure accesing $file");
$visits = (int)fgets($fp, 80);
$visits++;
ftruncate($fp, 0);
fseek($fp, 0);
fputs($fp, (string)$visits);
flock($fp, LOCK_UN) or die ("Cannot release lock file");
fclose($fp);

// For some reason, bmann doesn't like visitor counter so it's commented out for now
/*echo "
<!-- Counter -->
<font class=\"counter\">This page has been visited <font class=\"countervalue\">$visits</font>
times since 29th August 2002 16:43.<br>
You have been on this page <font class=\"countervalue\">$visitcount</font> times.</font><br>";*/
}


// New counter

$counter2_inited = false;
$counter2_disable = true;

if(file_exists("${basedir}database.inc"))
{
  include("${basedir}database.inc");
}
else
{
  // No database
  $counter2_disable = true;
}

/**
 * Main function
 * Counts visitor and saves data to the database
 *
 * @param page  filename of the visited page (with full path)
 **/
function counter2($page)
{
  global $_SERVER, $REMOTE_ADDR;
  global $basedir;

  global $counter2_disable;
  if($counter2_disable)
  {
    return;
  }

  // visit timeout (in seconds)
  $visit_timeout = 60 * 60;  // 60 minutes
  // database lock file
  $lockfile = "${basedir}.counter.lock";


  // acquire database lock
  if(is_writeable($lockfile))
  {
    $fp = fopen($lockfile, "w");
    flock($fp, LOCK_EX) or die ("Failure accesing " . $lockfile);
  }
  else
  {
    echo "<br>Local developer mode.<br>
    Can't write to lockfile.<br><br>";
    return;
  }

  // init database connection
  counter2_init();

  // counter_ip
  // Check if visitor has been here earlier and if yes, then how long time ago
  $query = "SELECT * FROM counter_ip WHERE ip = '$REMOTE_ADDR'";
  $result = mysql_query($query);
  $new_visitor = false;
  $new_visit = false;
  $now = gmdate("U");  // Time ATM in seconds
  if(mysql_num_rows($result) == 0)
  {
    // No earlier entry - new visitor
    $new_visitor = true;
    $new_visit = true;
    $query = "INSERT INTO counter_ip VALUES ('$REMOTE_ADDR', '$now')";
    mysql_query($query);
  }
  else
  {
    $row = mysql_fetch_assoc($result);
    if($row['timestamp'] + $visit_timeout < $now)
    {
      // More than $visit_timeout seconds have elapsed since last click - new visit
      $new_visit = true;
    }
    // Update last click time
    $query = "UPDATE counter_ip SET timestamp = '$now' WHERE ip = '$REMOTE_ADDR'";
    mysql_query($query);
  }


  // counter_total
  //  clicks
  // NOTE: no 'if(mysql_num_rows($result) == 0)' checks in many places because
  //  tables should already be initialized
  $query = "SELECT * FROM counter_total WHERE id = 'clicks'";
  $result = mysql_query($query);
  $totalcount = 0;
  $row = mysql_fetch_assoc($result);
  $totalcount = $row['count'] + 1;

  $query = "UPDATE counter_total SET count = '$totalcount' WHERE id = 'clicks'";
  mysql_query($query);

  //  visits
  $query = "SELECT * FROM counter_total WHERE id = 'visits'";
  $result = mysql_query($query);
  $visits = 0;
  $row = mysql_fetch_assoc($result);
  $visits = $row['count'];
  if($new_visit)
  {
    $visits++;

    $query = "UPDATE counter_total SET count = '$visits' WHERE id = 'visits'";
    mysql_query($query);
  }

  //  visitors
  $query = "SELECT * FROM counter_total WHERE id = 'visitors'";
  $result = mysql_query($query);
  $visitors = 0;
  $row = mysql_fetch_assoc($result);
  $visitors = $row['count'];
  if($new_visitor)
  {
    $visitors++;

    $query = "UPDATE counter_total SET count = '$visitors' WHERE id = 'visitors'";
    mysql_query($query);
  }


  // counter_pages
  $query = "SELECT * FROM counter_pages WHERE page = '$page'";
  $result = mysql_query($query);
  $pagecount = 0;
  if(mysql_num_rows($result) == 0)
  {
    // First time this page is viewed
    $pagecount++;
    $query = "INSERT INTO counter_pages VALUES ('$page', '$pagecount')";
    mysql_query($query);
  }
  else
  {
    $row = mysql_fetch_assoc($result);
    $pagecount = $row['count'] + 1;

    $query = "UPDATE counter_pages SET count = '$pagecount' WHERE page = '$page'";
    mysql_query($query);
  }


  // counter_days
  $date = gmdate("Y-m-d");
  $datecount = 0;
  $query = "SELECT * FROM counter_days WHERE date = '$date'";
  $result = mysql_query($query);
  if(mysql_num_rows($result) == 0)
  {
    // First view today
    $datecount++;
    $query = "INSERT INTO counter_days VALUES ('$date', '$datecount')";
    mysql_query($query);
  }
  else
  {
    $row = mysql_fetch_assoc($result);
    $datecount = $row['count'] + 1;

    $query = "UPDATE counter_days SET count = '$datecount' WHERE date = '$date'";
    mysql_query($query);
  }


  // counter_weekdays
  $weekday = gmdate("w") - 1;
  if($weekday == -1)
  {
    $weekday = 6;
  }
  // weekday is now 0 (Monday) - 6 (Sunday)
  $weekdaystr = gmdate("l");
  $weekdaycount = 0;
  $query = "SELECT * FROM counter_weekdays WHERE day = '$weekday'";
  $result = mysql_query($query);
  $row = mysql_fetch_assoc($result);
  $weekdaycount = $row['count'] + 1;

  $query = "UPDATE counter_weekdays SET count = '$weekdaycount' WHERE day = '$weekday'";
  mysql_query($query);


  // counter_time
  $hour = gmdate("H");
  $hourcount = 0;
  $query = "SELECT * FROM counter_time WHERE hour = '$hour'";
  $result = mysql_query($query);
  $row = mysql_fetch_assoc($result);
  $hourcount = $row['count'] + 1;

  $query = "UPDATE counter_time SET count = '$hourcount' WHERE hour = '$hour'";
  mysql_query($query);


  // counter_browser
  $browser = counter2_get_browser($_SERVER['HTTP_USER_AGENT']);
  $browsercount = 0;
  $query = "SELECT * FROM counter_browser WHERE id = '$browser'";
  $result = mysql_query($query);
  if(mysql_num_rows($result) == 0)
  {
    // First view with this browser
    $browsercount++;
    $query = "INSERT INTO counter_browser VALUES ('$browser', '$browsercount')";
    mysql_query($query);
  }
  else
  {
    $row = mysql_fetch_assoc($result);
    $browsercount = $row['count'] + 1;

    $query = "UPDATE counter_browser SET count = '$browsercount' WHERE id = '$browser'";
    mysql_query($query);
  }


  // counter_os
  $os = counter2_get_os($_SERVER['HTTP_USER_AGENT']);
  $oscount = 0;
  $query = "SELECT * FROM counter_os WHERE id = '$os'";
  $result = mysql_query($query);
  if(mysql_num_rows($result) == 0)
  {
    // First view with this os
    $oscount++;
    $query = "INSERT INTO counter_os VALUES ('$os', '$oscount')";
    mysql_query($query);
  }
  else
  {
    $row = mysql_fetch_assoc($result);
    $oscount = $row['count'] + 1;

    $query = "UPDATE counter_os SET count = '$oscount' WHERE id = '$os'";
    mysql_query($query);
  }

  // Log unknown user agents to a file so we can improve browser/os detection code
  // AB: currently not functional, so we disable this.
  /*
  if($browser == "Unknown")
  {
    $file = fopen($basedir . "unknown_browser", "a");
    fputs($file, date("Y/m/d H:i:s")." : $REMOTE_ADDR : $_SERVER['HTTP_USER_AGENT']\n");
    fclose($file);
  }
  if($os == "Unknown")
  {
    $file = fopen($basedir . "unknown_os", "a");
    fputs($file, date("Y/m/d H:i:s")." : $REMOTE_ADDR : $_SERVER['HTTP_USER_AGENT']\n");
    fclose($file);
  }
  */

  // Print some (all) stats
  /*echo "
  <a href=\"${basedir}stats.php\">This page: ".$pagecount."<br>
  Total today (".$date."): ".$datecount."<br>
  On ".$weekdaystr.": ".$weekdaycount."<br>
  On ".$hour.". hour: ".$hourcount."<br>
  With ".$browser." browser: ".$browsercount."<br>
  With ".$os." OS: ".$oscount."<br>
  Visits: ".$visits."; visitors: ".$visitors."<br>
  Total views: ".$totalcount."</a>";*/


  // Release database lock
  flock($fp, LOCK_UN) or die ("Cannot release lock file");
  fclose($fp);
}

function counter2_download($file)
{
  global $counter2_disable;
  if($counter2_disable)
  {
    return;
  }

  counter2_init();

  // counter_downloads
  $query = "SELECT * FROM counter_download WHERE file = '$file'";
  $result = mysql_query($query);
  $downloads = 0;
  if(mysql_num_rows($result) == 0)
  {
    $downloads++;
    $query = "INSERT INTO counter_download VALUES ('$file', '$downloads')";
    mysql_query($query);
  }
  else
  {
    $row = mysql_fetch_assoc($result);
    $downloads = $row['count'] + 1;

    $query = "UPDATE counter_download SET count = '$downloads' WHERE file = '$file'";
    mysql_query($query);
  }
}

function counter2_get_pageviews($page)
{
  global $counter2_disable;
  if($counter2_disable)
  {
    return 0;
  }

  counter2_init();

  $query = "SELECT * FROM counter_pages WHERE page = '$page'";
  $result = mysql_query($query);
  if(mysql_num_rows($result) == 0)
  {
    // Page hasn't been viewed
    return 0;
  }
  else
  {
    $row = mysql_fetch_assoc($result);
    return $row['count'];
  }
}
function counter2_get_downloads($file)
{
  global $counter2_disable;
  if($counter2_disable)
  {
    return 0;
  }

  counter2_init();

  $query = "SELECT * FROM counter_download WHERE file = '$file'";
  $result = mysql_query($query);
  if(mysql_num_rows($result) == 0)
  {
    // File hasn't been downloaded
    return 0;
  }
  else
  {
    $row = mysql_fetch_assoc($result);
    return $row['count'];
  }
}

$barwidth = 100;
$barheight = 10;

function counter2_statspage($pagelink)
{
  global $counter2_disable;
  if($counter2_disable)
  {
    return;
  }

  counter2_init();

  $daylimit = 20;

  $query = "SELECT * FROM counter_total WHERE id = 'clicks'";
  $result = mysql_query($query);
  $row = mysql_fetch_assoc($result);
  $total = $row['count'];
  $dayscount = (time() - mktime(0, 0, 0, 8, 2, 2003)) / 86400;  // Number of days passed since counter2 was installed
  echo "\n<font class=\"stats_views\">Total views: ".$total."</font>
    <font class=\"stats_views_average\">&nbsp;&nbsp;(average is ".(int)($total / $dayscount)." visits per day)</font><br>";

  $visitors = mysql_fetch_assoc(mysql_query("SELECT * FROM counter_total WHERE id = 'visitors'"));
  $visits = mysql_fetch_assoc(mysql_query("SELECT * FROM counter_total WHERE id = 'visits'"));
  echo "\n<font class=\"stats_visits\">Visitors: ".$visitors['count']."<br>
    Visits: ".$visits['count']."<br>
    (Since 2nd August 2003)</font><br><br>";

  counter2_statsbox("Pages", "Page", "Views", "counter_pages", "page", "ORDER BY count DESC", $total,
      "$page?show=pages", 0, true);
  counter2_statsbox("Days", "Date", "Views", "counter_days", "date", "ORDER BY date DESC", $total,
      "$page?show=days");
  $weekdays = array("Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday");
  counter2_statsbox("Weekdays", "Day", "Views", "counter_weekdays", "day", "ORDER BY day", $total,
      false, 0, false, 7, 0, $weekdays);
  counter2_statsbox("Hours", "Hour", "Views", "counter_time", "hour", "ORDER BY hour", $total,
      false, 0, false, 24);
  counter2_statsbox("Browsers", "Browser", "Views", "counter_browser", "id", "ORDER BY count DESC", $total,
      "$page?show=browsers");
  counter2_statsbox("Operating systems", "OS", "Views", "counter_os", "id", "ORDER BY count DESC", $total,
      "$page?show=os");
  counter2_statsbox("Downloads", "File", "Downloads", "counter_download", "file", "ORDER BY count DESC", 0,
      "$page?show=downloads");
}

/**
 * Function which displays box with stats from a single mysql table
 *
 * @param boxname  name of the box (header)
 * @param col1name  caption of the first column
 * @param col2name  caption of the second column
 * @param sql_table  name of mysql table where data will be taken from
 * @param sql_order  sql order string
 * @param total  number of total views, used for percentage and bar display. If
 *    it's 0, percentage and bar will not be shown
 * @param boxlink  if not false, header of the box will be link to this location
 * @param links  if true, links will be shown in first column
 * @param limit  at most how man rows to show
 * @param offset  first row to be shown
 * @param itemmap  here you can give map for names of first column
 **/
function counter2_statsbox($boxname, $col1name, $col2name, $sql_table, $sql_col1, $sql_order, $total,
    $boxlink = false, $currentpage = 0, $links = false, $limit = 15, $offset = 0, $itemmap = false)
{
  // FIXME: LESS ARGUMENTS!!!  MAKE THIS FUNCTION MORE CLEAR/READABLE!!!

  // size of percent bar and width of count cell
  global $barwidth, $barheight, $style;
  $viewwidth = 100;

  // Draw start (header) of the box
  draw_bigbox_begin($boxname);

  // Draw column headers
    echo "
      <tr>
        <td><b>$col1name</b></td>
        <td width=\"$viewwidth\"><b>$col2name</b></td>
        <td width=\"$barwidth\"></td>
      </tr>";

    // Make sql query
    $query = "SELECT * FROM $sql_table $sql_order LIMIT $offset,$limit";
    $result = mysql_query($query);

    $tot = 0;  // DEBUG only: sum of all counts
    $i = 0;
    $max = 0;  // max count of all items
    $pages = array();  // array with names (items) for column 1
    $counts = array();  // array with counts (for column 2)

    // Get all items and counts, put them to arrays and find max count
    while ($row = mysql_fetch_assoc($result))
    {
      $tot += $row['count'];
      $pages[$i] = $row["$sql_col1"];
      $counts[$i] = $row['count'];
      if($max < $row['count'])
      {
        $max = $row['count'];
      }
      $i++;
    }

    // Draw rows
    $evenrow = false;  // Used for two-color rows
    $rowstyle;  // CSS style of current row
    // Main drawing loop
    for($j = 0; $j < $i; $j++)
    {
      // Find out style for the row
      if($evenrow)
      {
        $rowstyle = "statscell";
      }
      else
      {
        $rowstyle = "statscell2";
      }

      // Find out item's name. If itemmap is used, get name from there
      if($itemmap)
      {
        $item = $itemmap[$pages[$j]];
      }
      else
      {
        $item = $pages[$j];
      }

      // Find out text to put to item table cell
      // If links are used, text includes item's name and link to it, otherwise
      //  just name
      if($links)
      {
        $itemtext = "<a class=\"statsitem\" href=\"".$item."\">".$item."</a>";
      }
      else
      {
        $itemtext = "<font class=\"statsitem\">".$item."</font>";
      }

      // Write actual html code
      echo "
      <tr>
        <td class=\"$rowstyle\">$itemtext</td>";
      if($total)
      {
        // If total count was given, print count for that item, percentage of
        //  all counts and show the bar
        $prct = ceil($counts[$j] * $barwidth / $max);
        echo "
        <td class=\"$rowstyle\" width=\"$viewwidth\">".$counts[$j]."  (".number_format($counts[$j] * 100 / $total, 1)." %)</td>
        <td class=\"$rowstyle\" width=\"$barwidth\"><img class=\"statsbar\" src=\"stats-bar-$style.png\" width=\"$prct\" height=\"$barheight\" alt=\"\"></td>";
      }
      else
      {
        // Otherwise just print count
        echo "
        <td class=\"$rowstyle\" width=\"$viewwidth\">".$counts[$j]."</td>
        <td class=\"$rowstyle\" width=\"$barwidth\"></td>";
      }
      echo "
      </tr>";
      $evenrow = !$evenrow;
    }

    // Postprocessing
    if(($i == $limit && $boxlink) || $currentpage > 1)
    {
      // If $limit rows were displayed, then it is possible that some rows were
      //  not displayed. Check it
      $query = "SELECT COUNT(*) FROM $sql_table";
      $result = mysql_query($query);
      $row = mysql_fetch_assoc($result);
      $count = $row['COUNT(*)'];
      if($count > $limit || $currentpage > 1)
      {
        // There are more rows than were shown
        if($currentpage == 0)
        {
          // Not showing specific page
          // Just show "More..." link
          echo "
        <tr>
          <td><br><a href=\"$boxlink\">More...</a></td>
          <td></td>
          <td></td>
        </tr>";
        }
        else
        {
          // Showing specific page
          // Show links to all pages
          $pages = "";  // String containing links
          $page = 0;  // Current page
          while($page * $limit < $count)
          {
            $page++;
            if($page == $currentpage)
            {
              // Current page is in bold text and isn't a link
              $pages .= " <b>$page</b>";
            }
            else
            {
              $pages .= " <a href=\"${boxlink}&page=$page\">$page</a>";
            }
          }
          // Show links to all pages
          echo "
      <tr>
        <td>Pages: $pages</td>
        <td></td>
        <td></td>
      </tr>";
        }
      }
      else
      {
      }
    }

  // Draw end of the box
  draw_bigbox_end();
}


// BELOW: non-public functions (you shouldn't use them)

function counter2_init()
{
  global $counter2_inited;
  if($counter2_inited)
  {
    return;
  }

  database_connect();
  if(!counter2_check_database())
  {
    counter2_create_database();
  }

  $counter2_inited = true;
}

function counter2_check_database()
{
  global $database;
  $tables = mysql_list_tables($database);
  while($table = mysql_fetch_row($tables))
  {
    if($table[0] == "counter_browser")
    {
      mysql_free_result($tables);
      return true;
    }
  }
  mysql_free_result($tables);
  return false;
}

function counter2_create_database()
{
  // Create tables
  mysql_query("CREATE TABLE counter_days (date DATE, count INT)")
       or die("Can't create table counter_days!");
  mysql_query("CREATE TABLE counter_weekdays (day INT, count INT)")
       or die("Can't create table counter_weekdays!");
  mysql_query("CREATE TABLE counter_time (hour INT, count INT)")
       or die("Can't create table counter_time!");
  mysql_query("CREATE TABLE counter_browser (id TEXT, count INT)")
       or die("Can't create table counter_browser!");
  mysql_query("CREATE TABLE counter_os (id TEXT, count INT)")
       or die("Can't create table counter_os!");
  mysql_query("CREATE TABLE counter_pages (page TEXT, count INT)")
       or die("Can't create table counter_pages!");
  mysql_query("CREATE TABLE counter_download (file TEXT, count INT)")
       or die("Can't create table counter_download!");
  mysql_query("CREATE TABLE counter_total (id TEXT, count INT)")
       or die("Can't create table counter_total!");
  mysql_query("CREATE TABLE counter_ip (ip TEXT, timestamp INT)")
       or die("Can't create table counter_ip!");

  // Init tables
  for($i = 0; $i < 7; $i++)
  {
    mysql_query("INSERT INTO counter_weekdays VALUES ('$i', 0)");
  }
  for($i = 0; $i < 24; $i++)
  {
    mysql_query("INSERT INTO counter_time VALUES ('$i', 0)");
  }
  mysql_query("INSERT INTO counter_total VALUES ('clicks', 0)");
  mysql_query("INSERT INTO counter_total VALUES ('visits', 0)");
  mysql_query("INSERT INTO counter_total VALUES ('visitors', 0)");
}


function counter2_get_browser($ua)
{
  if(eregi("Opera", $ua))
  {
    return "Opera";
  }
  else if(eregi("Galeon", $ua))
  {
    return "Galeon";
  }
  else if(eregi("Lynx", $ua))
  {
    return "Lynx";
  }
  else if(eregi("Firebird", $ua))
  {
    return "Mozilla Firebird";
  }
  else if(eregi("Firefox", $ua))
  {
    return "Firefox";
  }
  else if(eregi("Konqueror", $ua))
  {
    return "Konqueror";
  }
  else if(eregi("Safari", $ua))
  {
    return "Safari";
  }
  else if(eregi("(rv:0.9)|(rv:1.0)|(rv:1.1)|(rv:1.2)|(rv:1.3)|(rv:1.4)|(rv:1.5)|(rv:1.6)|(rv:1.7)|(rv:1.8)|(rv:1.)", $ua))
  {
    return "Mozilla";
  }
  else if(eregi("Links", $ua))
  {
    return "Links";
  }
  else if(eregi("Java", $ua))
  {
    return "Java";
  }
  else if(eregi("(libwww-perl)|(LWP::Simple)", $ua))
  {
    return "libwww-perl";
  }
  else if(eregi("Wget", $ua))
  {
    return "wget";
  }
  else if(eregi("Dillo", $ua))
  {
    return "Dillo";
  }
  else if(eregi("(nuhk)|(Googlebot)|(Mediapartners-Google)|(msnbot)|(Yammybot)|(Openbot)|(Slurp)|(crawler)|(ContentHarvester)|(ia_archiver)|(Sniffer)|(grub)|(Pompos)|(SpiderKU)|(ZyBorg)|(Bot)|(Microsoft URL Control)|(Ask Jeeves)|(Infoseek SideWinder)|(Scooter)|(WebCopier)|(W3C_Validator)|(Zeus)|(Baiduspider)|(lwp-trivial)|(Digger)|(larbin)|(QuepasaCreep)|(Ocelli)|(spider)", $ua))
  {
    return "SearchBot";
  }
  else if(eregi("(MSIE 3)|(MSIE 4)|(MSIE 5.0)|(MSIE 5.1)|(MSIE 5.2)|(MSIE 5.5)|(MSIE 6.0)", $ua))
  {
    return "Internet Explorer";
  }
  else if(eregi("(Mozilla/3)|(Mozilla/4.0)|(Mozilla/4.08)|(Mozilla/4.5)|(Mozilla/4.6)|(Mozilla/4.7)|(Mozilla/4.75)|(Mozilla/4.79)|(Netscape6)|(Netscape/7)|(Netscape)", $ua))
  {
    return "Netscape";
  }
  else
  {
    return "Unknown";
  }
}

function counter2_get_os($ua)
{
  if(eregi("Win16", $ua))
  {
    return "Windows 3.11";
  }
  else if(eregi("(Windows 95)|(Win95)|(Windows_95)", $ua))
  {
    return "Windows 95";
  }
  else if(eregi("(Windows 98)|(Win98)|(Win 9x)|(Win32)", $ua))
  {
    return "Windows 98";  // Note: fallback for Win 9x and Win32
  }
  else if(eregi("(Windows NT 5.0)|(Windows 2000)", $ua))
  {
    return "Windows 2000";
  }
  else if(eregi("(Windows NT 4.0)|(WinNT4.0)|(WinNT)|(Windows NT)", $ua))
  {
    return "Windows NT 4.0";
  }
  else if(eregi("Windows ME", $ua))
  {
    return "Windows ME";
  }
  else if(eregi("(Windows NT 5.1)|(Windows XP)|(Windows)", $ua))
  {
    return "Windows XP";  // Note: fallback for just Windows
  }
  else if(eregi("OpenBSD", $ua))
  {
    return "OpenBSD";
  }
  else if(eregi("FreeBSD", $ua))
  {
    return "FreeBSD";
  }
  else if(eregi("NetBSD", $ua))
  {
    return "NetBSD";
  }
  else if(eregi("SunOS", $ua))
  {
    return "Sun OS";
  }
  else if(eregi("(Linux)|(X11)", $ua))
  {
    return "Linux";
  }
  else if(eregi("(Mac_PowerPC)|(Macintosh)", $ua))
  {
    return "Macintosh";
  }
  else if(eregi("QNX", $ua))
  {
    return "QNX";
  }
  else if(eregi("BeOS", $ua))
  {
    return "BeOS";
  }
  else if(eregi("OS/2", $ua))
  {
    return "OS/2";
  }
  else if(eregi("Amiga", $ua))
  {
    return "Amiga";
  }
  else if(eregi("UNIX", $ua))
  {
    return "UNIX";
  }
  else if(eregi("(nuhk)|(Googlebot)|(Mediapartners-Google)|(msnbot)|(Yammybot)|(Openbot)|(Slurp)|(crawler)|(ContentHarvester)|(ia_archiver)|(Sniffer)|(grub)|(Pompos)|(SpiderKU)|(ZyBorg)|(Bot)|(Microsoft URL Control)|(Ask Jeeves)|(Infoseek SideWinder)|(Scooter)|(WebCopier)|(W3C_Validator)|(Zeus)|(Baiduspider)|(lwp-trivial)|(Digger)|(larbin)|(QuepasaCreep)|(Ocelli)|(spider)", $ua))
  {
    return "SearchBot";
  }
  else
  {
    return "Unknown";
  }
}

function counter2_drawviews($views, $total, $max)
{
  global $barwidth, $barheight;
  global $style;

  $prct = ceil($views * $barwidth / $max);
  return $views."  (".number_format($views * 100 / $total, 1)." %)
      <img class=\"statsbar\" src=\"stats-bar-$style.png\" width=\"$prct\" height=\"$barheight\" alt=\"\" align=\"right\">";
}


?>
