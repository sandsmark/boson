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


function counter()
{
global $visitcount;
global $counter_file;
global $basedir;
$fp = fopen($basedir . $counter_file, "r+");
flock($fp, LOCK_EX) or die ("Failure accesing $counter_file");
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

?>
