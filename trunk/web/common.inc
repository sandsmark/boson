<?php
/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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


/*****  Some variables  *****/
$latestversion="0.13";
$latestversiondate="1st October 2006";

$counter_file="counter";
$news_file="news.text";
$main_news_max=5;
$old_news_max=10;
$all_news_max=25;

$style="green";

$visitcount=0;
$lastupdate=0;

$startsec=0;
$startusec=0;

if(!isset($basedir))
{
  $basedir="";
}

/*****  Easy page start/end  *****/

function start_page($title)
{
  // Cookies, file modification date, etc
  do_start_stuff();
  // HTML header
  html_print_header($title);
  // Page header (logo)
  print_header();
  // Start of main table
  main_table_begin();
}

function end_page()
{
  main_table_end();
  // Footers
  print_footer();
  html_print_footer();
}


/*****  Header & footer  *****/

function print_footer()
{
// Generation timer stuff
// Note that actually it's not exact time, because html_print_footer is called
//  after it. But it's probably just a matter of few microseconds...
/*global $startsec, $startusec;
$timearray = gettimeofday();
$elapsedtime=($timearray["sec"] - $startsec) * 1000000 + ($timearray["usec"] - $startusec);
$elapsedtime /= 1000;
echo "
<font class=\"elapsedtime\">It took <font class=\"elapsedtimevalue\">$elapsedtime</font>
 msec to generate this page</font>";*/
}

function print_header()
{
$image="";
global $style, $basedir;
if($style == "blue") {
  $image="${basedir}pictures/boson.png";
} else {
  $image="${basedir}pictures/header.jpg";
}
echo "<div align=\"left\"><img src=\"$image\" alt=\"Boson logo\"></div><br>";
if($style=="green")
  echo "<div style='position:absolute;right:20px;top:115px;'><img src='${basedir}pictures/boson-logo.gif' alt=\"Boson logo\"></div><br><br>";
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
global $basedir;
/*global $startsec, $startusec;
$timearray = gettimeofday();
$startsec=$timearray["sec"];
$startusec=$timearray["usec"];*/

// We don't want anthing to cache this page, because it changes dynamically all the time (counter)
// This is taken from PHP Manual
header ("Expires: Mon, 26 Jul 1997 05:00:00 GMT");    // Date in the past
header ("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT"); // Always modified
header ("Cache-Control: no-cache, must-revalidate");  // HTTP/1.1
header ("Pragma: no-cache");                          // HTTP/1.0

// Select stylesheet
global $style;
$stylesheet="";
if($style == "blue") {
  $stylesheet="${basedir}style-blue.css";
} else {
  $stylesheet="${basedir}style-green.css";
}

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


/*****  "Main table" stuff  *****/

function main_table_begin()
{
echo "
<!-- Begin of main table -->
<table border=\"0\" cellpadding=\"3\" cellspacing=\"2\" width=\"100%\" class=\"main\">
  <tr valign=\"top\">";
}

function main_table_end()
{
echo "
<!-- End of main table -->
  </tr>
</table>";
}

function main_area_begin()
{
echo "
<!-- Begin of main area -->
<td width=\"100%\" class=\"mainarea\">";
}

function main_area_end()
{
echo "
<!-- End of main area -->
</td>";
}


/*****  Box drawing functions  *****/

/**
* Draws little box with $title in it
**/
function draw_box($title)
{
echo "
<table cellspacing=\"1\" cellpadding=\"2\" width=\"100%\" class=\"box\">
  <tr><td class=\"boxcell\">
    <font class=\"boxtitle\">&nbsp;&nbsp;$title</font>
  </td></tr>
</table>";
}

/**
* Draws beginning of big box, $title is title of box
**/
function draw_bigbox_begin($title)
{
echo "
<table cellspacing=\"1\" cellpadding=\"2\" width=\"100%\" class=\"bigbox\">
  <tr><td class=\"bigboxtitlecell\">
    <font class=\"bigboxtitle\">&nbsp;&nbsp;$title</font>
  </td></tr>
  <tr><td align=\"center\" class=\"bigboxcell\">
    <br>
    <table cellspacing=\"0\" cellpadding=\"0\" width=\"99%\" class=\"bigboxarea\">";
}

/**
* Draws end of big box
**/
function draw_bigbox_end()
{
echo "
    </table>
  </td></tr>
</table>
<br>";
}

/**
* Draws "subheader" (text on darker background) to big box
* $text is text of subheader
**/
function draw_bigbox_subheader($text, $style = "bigboxsubheader")
{
echo "
      <tr><td class=\"bigboxsubheadercell\">
        <font class=\"$style\">&nbsp;$text</font>
      </td></tr>";
}

/**
* Draws $text to big box. $text is simple text
**/
function draw_bigbox_text($text, $style = "bigboxtext")
{
echo "
      <tr><td>
        <p class=\"$style\">$text</p>
      </td></tr>";
}



?>
