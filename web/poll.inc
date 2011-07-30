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



$poll_inited = false;
$poll_disable = true;

include_once("common.inc");

if(file_exists("${basedir}database.inc"))
{
  include_once("${basedir}database.inc");
}
else
{
  // No database
  $poll_disable = true;
}

function poll_show($id = -1)
{
  global $poll_disable;
  global $_COOKIE;

  if($poll_disable)
  {
    echo "Currently no poll.";
    return;
  }
  
  // init database connection
  poll_init();

  if($id == -1)
  {
    // Select most recent poll
    $query = "SELECT * FROM poll ORDER BY id DESC LIMIT 1";
  }
  else
  {
    $query = "SELECT * FROM poll WHERE id = '$id' LIMIT 1";
  }
  $result = mysql_query($query);
  if(mysql_num_rows($result) == 0)
  {
    // Poll doesn't exist! Evil!
    echo "No poll found with id $id";
  }
  else
  {
    $row = mysql_fetch_assoc($result);
    $id = $row['id'];

    // Question
    echo "
    <font class=\"pollquestion\">".$row['question']."</font><br>
    <form method=\"post\" action=\"poll.php\">";
    $i = 1;
    while($i <= 10 && $row['answer'.$i] != "")
    {
      echo "
      <input type=\"radio\" name=\"answer\" value=\"$i\"><font class=\"pollanswer\">".stripslashes($row['answer'.$i])."</font><br>";
      $i++;
    }
    echo "
      <input type=\"hidden\" name=\"id\" value=\"".$id."\">
      <input type=\"hidden\" name=\"action\" value=\"vote\">
      <input type=\"submit\" value=\"Vote\">
    </form>
    <a class=\"pollresults\" href=\"${basedir}poll.php?action=results&amp;id=$id\">Results</a><br>";
  }
}

function poll_vote($id, $answer)
{
  global $poll_disable, $basedir;
  if($poll_disable)
  {
    return;
  }

  // init database connection
  poll_init();

  // database lock file
  $lockfile = "${basedir}.poll.lock";
  // acquire database lock
  $fp = fopen($lockfile, "w");
  flock($fp, LOCK_EX) or die ("Failure accesing $lockfile");

  $query = "SELECT * FROM poll WHERE id = '$id' LIMIT 1";
  $result = mysql_query($query);
  if(mysql_num_rows($result) == 0)
  {
    // Release database lock
    flock($fp, LOCK_UN) or die ("Cannot release lock file");
    fclose($fp);
    return ""; // Error
  }
  else
  {
    $row = mysql_fetch_assoc($result);
    $count = $row['count'.$answer] + 1;

    $query = "UPDATE poll SET count${answer} = '$count' WHERE id = '$id'";

    mysql_query($query);

    // Release database lock
    flock($fp, LOCK_UN) or die ("Cannot release lock file");
    fclose($fp);

    return $row['answer'.$answer];  // Return selected answer
  }
}

function poll_results($id = -1)
{
  global $poll_disable, $style;
  if($poll_disable)
  {
    return;
  }

  // init database connection
  poll_init();


  if($id == -1)
  {
    // Select most recent poll
    $query = "SELECT * FROM poll ORDER BY id DESC LIMIT 1";
  }
  else
  {
    $query = "SELECT * FROM poll WHERE id = '$id' LIMIT 1";
  }
  $result = mysql_query($query);
  if(mysql_num_rows($result) == 0)
  {
    echo "No poll found with id $id";
  }
  else
  {
    $row = mysql_fetch_assoc($result);

    // Question
    echo "
    <font class=\"pollquestion\">".$row['question']."</font><br>
    <table>";
    // Count all answers
    $i = 1;
    $total = 0;
    $max = 0;
    while($i <= 10)
    {
      $total = $total + $row['count'.$i];
      if($row['count'.$i] > $max)
      {
        $max = $row['count'.$i];
      }
      $i++;
    }
    // Display results
    $i = 1;
    while($i <= 10 && $row['answer'.$i] != "")
    {
      if($row['count'.$i] == 0)
      {
        $width = 0;
        $prct = 0;
      }
      else
      {
        $width = ceil($row['count'.$i] * 100 / $max);
        $prct = ceil($row['count'.$i] * 100 / $total);
      }
      echo "
      <tr>
        <td width=\"200\"><font class=\"pollanswer\">".stripslashes($row['answer'.$i])."</font></td>
        <td width=\"100\">".$row['count'.$i]."  ($prct %)</td>
        <td width=\"100\"><img class=\"statsbar\" src=\"stats-bar-$style.png\" width=\"$width\" height=\"10\" alt=\"\"></td>
      </tr>
      ";
      $i++;
    }
    echo "
      </table><br>";
    echo "
      Total votes: $total<br>
      <br>
      <a href=\"${basedir}poll.php?action=list\">Old polls</a><br>";

  }
}

function poll_results_small($id = -1)
{
  global $poll_disable, $style;
  if($poll_disable)
  {
    return;
  }

  // init database connection
  poll_init();

  if($id == -1)
  {
    // Select most recent poll
    $query = "SELECT * FROM poll ORDER BY id DESC LIMIT 1";
  }
  else
  {
    $query = "SELECT * FROM poll WHERE id = '$id' LIMIT 1";
  }
  $result = mysql_query($query);
  if(mysql_num_rows($result) == 0)
  {
    echo "No poll found with id $id";
  }
  else
  {
    $row = mysql_fetch_assoc($result);

    // Question
    if (strlen($row['question']) >= 60)
    {
      $small_row = substr($row['question'], 0, 60)."...";
    }
    else
    {
      $small_row = $row['question'];
    }
    echo "
    <font class=\"smallpollanswer\">Results for:</font><br>
    <font class=\"smallpollquestion\">$small_row</font><br>";
    // Count all answers
    $i = 1;
    $total = 0;
    $max = 0;
    while($i <= 10)
    {
      $total = $total + $row['count'.$i];
      if($row['count'.$i] > $max)
      {
        $max = $row['count'.$i];
      }
      $i++;
    }
    // Display results
    $i = 1;
    while($i <= 10 && $row['answer'.$i] != "")
    {
      if($row['count'.$i] == 0)
      {
        $width = 0;
        $prct = 0;
      }
      else
      {
        $width = ceil($row['count'.$i] * 100 / $max);
        $prct = ceil($row['count'.$i] * 100 / $total);
      }
      // Make some things smaller
      $small_answer = stripslashes($row['answer'.$i]);
      if (strlen($small_answer) >= 30)
      {
        $small_answer = substr($small_answer, 0, 30)."...";
      }

      echo "
      <font class=\"smallpollanswer\">".$small_answer."</font><br>
      &nbsp;&nbsp;&nbsp;<img class=\"statsbar\" src=\"stats-bar-$style.png\" width=\"$width\" height=\"8\" alt=\"\">
      <font class=\"smallpollanswer\">"." $prct %</font><br>";
      $i++;
    }
    echo "
      <br>
      <font class=\"smallpollanswer\">Total votes: $total<br>
      <br>
      <a href=\"${basedir}poll.php?action=results&id=$id\">Details</a></font><br>";

  }
}


function poll_list()
{
  global $poll_disable;
  if($poll_disable)
  {
    return;
  }

  // init database connection
  poll_init();


  // Select most recent poll
  $query = "SELECT * FROM poll ORDER BY id DESC";
  $result = mysql_query($query);
  if(mysql_num_rows($result) == 0)
  {
    echo "No polls!";
  }
  else
  {
    echo "
    All polls:<br><br>";
    while ($row = mysql_fetch_assoc($result))
    {
      $time = date("Y/m/d H:i", $row['timestamp']);
      echo "
      <a href=\"${basedir}poll.php?action=results&amp;id=".$row['id']."\">".$row['question']."</a>&nbsp;&nbsp;($time)<br>";
    }
  }
}

function poll_create()
{
  echo "
  <form method=\"post\" action=\"poll.php\"><table>
    <tr>
      <td width=\"200\"><b>Question:</b></td>
      <td><input type=\"text\" name=\"question\" maxlenght=\"60\"></td>
    </tr>
    <tr>
      <td width=\"200\"><br></td>
      <td><br></td>
    </tr>
    <tr>
      <td width=\"200\"><b>Answer 1:</b></td>
      <td><input type=\"text\" name=\"answer1\" maxlenght=\"30\"></td>
    </tr>
    <tr>
      <td width=\"200\"><b>Answer 2:</b></td>
      <td><input type=\"text\" name=\"answer2\" maxlenght=\"30\"></td>
    </tr>
    <tr>
      <td width=\"200\"><b>Answer 3:</b></td>
      <td><input type=\"text\" name=\"answer3\" maxlenght=\"30\"></td>
    </tr>
    <tr>
      <td width=\"200\"><b>Answer 4:</b></td>
      <td><input type=\"text\" name=\"answer4\" maxlenght=\"30\"></td>
    </tr>
    <tr>
      <td width=\"200\"><b>Answer 5:</b></td>
      <td><input type=\"text\" name=\"answer5\" maxlenght=\"30\"></td>
    </tr>
    <tr>
      <td width=\"200\"><b>Answer 6:</b></td>
      <td><input type=\"text\" name=\"answer6\" maxlenght=\"30\"></td>
    </tr>
    <tr>
      <td width=\"200\"><b>Answer 7:</b></td>
      <td><input type=\"text\" name=\"answer7\" maxlenght=\"30\"></td>
    </tr>
    <tr>
      <td width=\"200\"><b>Answer 8:</b></td>
      <td><input type=\"text\" name=\"answer8\" maxlenght=\"30\"></td>
    </tr>
    <tr>
      <td width=\"200\"><b>Answer 9:</b></td>
      <td><input type=\"text\" name=\"answer9\" maxlenght=\"30\"></td>
    </tr>
    <tr>
      <td width=\"200\"><b>Answer 10:</b></td>
      <td><input type=\"text\" name=\"answer10\" maxlenght=\"30\"></td>
    </tr>
  </table>
  <input type=\"hidden\" name=\"action\" value=\"add\">
  <br>
  <input type=\"submit\" value=\"Add poll\">
  </form>";
}

function poll_add($question, $answer1, $answer2, $answer3 = "", $answer4 = "", $answer5 = "",
    $answer6 = "", $answer7 = "", $answer8 = "", $answer9 = "", $answer10 = "")
{
  global $poll_disable, $basedir;
  if($poll_disable)
  {
    return;
  }

  // init database connection
  poll_init();

  // database lock file
  $lockfile = "${basedir}.poll.lock";

  if(is_writeable($lockfile))
  {
    $fp = fopen($lockfile, "w");
    flock($fp, LOCK_EX) or die ("Failure accesing $lockfile");
  }
  else
  {
    echo "<br>Local developer mode.<br>
    Can't write to lockfile.<br><br>";
    return;
  }

  $now = gmdate("U");  // Time ATM in seconds
  $query = "INSERT INTO poll VALUES (0, '$question', $now,
      '".addslashes($answer1)."', '".addslashes($answer2)."', '".addslashes($answer3)."', '".addslashes($answer4)."',
      '".addslashes($answer5)."', '".addslashes($answer6)."', '".addslashes($answer7)."',
      '".addslashes($answer8)."', '".addslashes($answer9)."', '".addslashes($answer10)."',
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0)";
  mysql_query($query) or die("Can't add new poll!");

  // Release database lock
  flock($fp, LOCK_UN) or die ("Cannot release lock file");
  fclose($fp);
}

function poll_last_id()
{
  global $poll_disable;
  if($poll_disable)
  {
    return;
  }

  // init database connection
  poll_init();

  $query = "SELECT id FROM poll ORDER BY id DESC LIMIT 1";
  $result = mysql_query($query);
  if(mysql_num_rows($result) == 0)
  {
    // Poll doesn't exist! Evil!
    return 0;
  }
  else
  {
    $row = mysql_fetch_assoc($result);
    return $row['id'];
  }
}


function poll_init()
{
  global $poll_inited;
  if($poll_inited)
  {
    return;
  }

  database_connect();
  $poll_inited = true;

  if(!poll_check_database())
  {
    poll_create_database();
  }
}

function poll_check_database()
{
  global $database;
  $tables = mysql_list_tables($database);
  while($table = mysql_fetch_row($tables))
  {
    if($table[0] == "poll")
    {
      mysql_free_result($tables);
      return true;
    }
  }
  mysql_free_result($tables);
  return false;
}

function poll_create_database()
{
  // Create tables
  mysql_query("CREATE TABLE poll (id INT AUTO_INCREMENT PRIMARY KEY NOT NULL, question TEXT, timestamp INT,
      answer1 TEXT, answer2 TEXT, answer3 TEXT, answer4 TEXT, answer5 TEXT, answer6 TEXT, answer7 TEXT, answer8 TEXT, answer9 TEXT, answer10 TEXT,
      count1 INT, count2 INT, count3 INT, count4 INT, count5 INT, count6 INT, count7 INT, count8 INT, count9 INT, count10 INT)")
       or die("Can't create table poll!");

  // Make first "test poll"
/*  mysql_query("INSERT INTO poll VALUES (0, 'Should this sample poll be replaced?',
      'Yes', 'No', 'What?', 'Who cares?', '5th option',
      0, 0, 0, 0, 0)");*/
  poll_add("Should this sample poll be replaced?",
      "Yes", "No", "There's a poll! Cool!", "What?", "Who cares?", "6th option");
}


?>
