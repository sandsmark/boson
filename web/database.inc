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


/*$hostname="mysql.sourceforge.net";
$username="boson";
$database="boson";
$password="Black_hOLE-7";*/
$hostname="localhost";
$username="rivo";
$password="mypass";
$database="php";

$link;

function database_connect()
{
  global $link;
  global $hostname, $username, $password, $database;
  $link = mysql_connect($hostname, $username, $password) or die ("Problem with SQL connection");
  mysql_select_db($database) or die ("Cannot select database");
}

?>
