<?php


/*****  Variables  *****/
$filename="options.php";

$change_style="";

/*****  Some includes  *****/
include("common.php");
include("sidebar.php");
include("main.php");
include("counter.php");
include("boxes.php");
include("variables.php");

/*****  Start of main stuff  *****/

function draw_option($value, $desc)
{
global $style;
$isselected;
if($style == $value)
  $isselected=" selected";
echo "
    <option value=\"$value\"$isselected>$desc</option>";
}


//$style;
if($HTTP_GET_VARS["style"] != "")
{
  $change_style=$HTTP_GET_VARS["style"];
  setcookie("Style", $change_style, time() + 3600 * 24 * 365 * 25); // Expires after 25 years
}

do_start_stuff();

if($change_style != "")
  $style=$change_style;

// Headers
html_print_header("Style options");
print_header();

// Main table
main_table_begin();

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_box("Options");
echo "
    Here you can change style, or color scheme, of this homepage.<br>
    Currently, there are 2 different styles: blue, which looks
    like style of KDE homepages and green (default) which has green and black colors and
    looks cooler.<br>
    Note that your browser must have cookies enabled for this site for this to
    work.<br>
    Your style: <form class=\"style\" action=\"$filename\" method=\"get\">
    <select size=\"1\" name=\"style\">";
draw_option("green", "Green/black style (default)");
draw_option("blue", "Blue style");
echo "
    </select>
    &nbsp;&nbsp;
    <input type=\"submit\" value=\"Change!\">

";
main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
