<?php

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
