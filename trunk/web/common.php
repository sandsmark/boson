<?php

$startsec;
$startusec;

/*****  Header & footer  *****/

function print_footer()
{
// Copyright
echo "
<hr>
<font class=\"copyright\">(C) 2002  <a class=\"copyright\" href=\"mailto:boson-devel@lists.sourceforge.net\">
The Boson Team</a></font><br>";

// Generation timer stuff
// Note that actually it's not exact time, because html_print_footer is called
//  after it. But it's probably just a matter of few microseconds...
global $startsec, $startusec;
$timearray = gettimeofday();
$elapsedtime=($timearray["sec"] - $startsec) * 1000000 + ($timearray["usec"] - $startusec);
$elapsedtime /= 1000;
echo "
<font class=\"elapsedtime\">It took <font class=\"elapsedtimevalue\">$elapsedtime</font>
 msec to generate this page</font>";
}

function print_header()
{
$image;
global $style;
if($style == "green")
  $image="pictures/header.jpg";
else
  $image="pictures/boson.png";
echo "
<h1 align=\"center\"><img src=\"$image\" alt=\"Boson logo\"></h1><br>";
}

/*****  HTML header & footer  *****/

function html_print_footer()
{
echo "
  </body>
</html>";
}

// title : Title of the page
function html_print_header($title)
{
global $startsec, $startusec;
$timearray = gettimeofday();
$startsec=$timearray["sec"];
$startusec=$timearray["usec"];

// We don't want anthing to cache this page, because it changes dynamically all the time (counter)
// This is taken from PHP Manual
header ("Expires: Mon, 26 Jul 1997 05:00:00 GMT");    // Date in the past
header ("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT"); // Always modified
header ("Cache-Control: no-cache, must-revalidate");  // HTTP/1.1
header ("Pragma: no-cache");                          // HTTP/1.0

// Select stylesheet
global $style;
$stylesheet;
if($style == "green")
  $stylesheet="style-green.css";
else
  $stylesheet="style-blue.css";

// HTML header
echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">
<html>
  <head>
    <title>Boson homepage: $title</title>
    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">
    <meta name=\"Title\" content=\"Boson: real-time strategy game for KDE\">
    <meta name=\"description\" content=\"Boson is a free real-time strategy game for KDE\">
    <meta name=\"keywords\" content=\"Boson,real-time,realtime,real time,strategy,game,KDE,Linux,X11,Qt,OpenGL,3d\">
    <link rel=\"stylesheet\" type=\"text/css\" href=\"$stylesheet\">
  </head>
  <body>";
  // "
}

/*****  Start stuff  *****/

function do_start_stuff()
{
// Visit cookie stuff
global $visitcount, $HTTP_COOKIE_VARS;
$visitcount=(int)$HTTP_COOKIE_VARS["Visits"];
$visitcount++;
setcookie("Visits", (string)$visitcount, time() + 3600 * 24 * 365 * 25); // Expires after 25 years

// Style cookie stuff
global $style;
if($HTTP_COOKIE_VARS["Style"] != "")
  $style=$HTTP_COOKIE_VARS["Style"];

// Last modification stuff
global $filename, $lastupdate;
$lastupdate=gmdate("jS F Y H:i:s", filemtime($filename));
}

?>
