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
draw_bigbox_begin("Options");
draw_bigbox_text("Here you can change the color scheme of this homepage.
	Currently, there are two different styles: blue, which looks like the KDE homepages and green (default)
	which has green and black colors and looks cooler.<br><br>
	Note that your browser must have cookies enabled if you want a different style for the homepage.<br><br>");

draw_bigbox_text("Your style:");

draw_bigbox_text(" <form class=\"style\" action=\"$filename\" method=\"get\"><select size=\"1\" name=\"style\"><option value='green'>Green/black style (default)</option><option value='blue'>Blue style</option></select><input type=\"submit\" value=\"Change!\">");

draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
