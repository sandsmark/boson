<?php

function counter()
{
global $visitcount;
global $counter_file;
global $basedir;
$fp = fopen($basedir . $counter_file, "r");
$visits = (int)fgets($fp, 80);
fclose($fp);

$visits++;

$fp = fopen($basedir . $counter_file, "w");
fputs($fp, (string)$visits);
fclose($fp);

// For some reason, bmann doesn't like visitor counter so it's commented out for now
/*echo "
<!-- Counter -->
<font class=\"counter\">This page has been visited <font class=\"countervalue\">$visits</font>
times since 29th August 2002 16:43.<br>
You have been on this page <font class=\"countervalue\">$visitcount</font> times.</font><br>";*/
}

?>
