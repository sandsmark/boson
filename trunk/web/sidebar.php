<?php

function sidebar_begin()
{
echo "
<!-- Begin sidebar -->
<td>
  <table width=\"200\" cellpadding=\"0\" cellspacing=\"0\" class=\"sidebar\">";
}

function sidebar_end()
{
echo "
<!-- End sidebar -->
  </table>
</td>";
}

function sidebar_box_begin($title)
{
echo "
<!-- Begin sidebar box with title $title -->
<tr><td>
  <table cellpadding=\"2\" cellspacing=\"1\" border=\"0\" width=\"100%\" class=\"sidebarbox\">
    <tr><td class=\"sidebarboxtitlecell\">
      <font class=\"sidebarboxtitle\">&nbsp;$title</font>
    </td></tr>
    <tr><td class=\"sidebarboxcell\">";
}

function sidebar_box_end()
{
echo "
    <!-- End sidebar box -->
    </td></tr>
  </table>
  <br>
</td></tr>";
}

function draw_link($title, $href, $style = "sidebarboxlink")
{
echo "
&nbsp;&nbsp;<a class=\"$style\" href=\"$href\">$title</a><br>";
}

function sidebar_links_box()
{
sidebar_box_begin("Links");
draw_link("Main page", "index.php");
draw_link("Screenshots", "screenshots.php");
draw_link("Download", "download.php");
draw_link("Install", "install.php");
draw_link("More information", "info.php");
draw_link("Status", "status.php");
draw_link("Contact us", "contact.php");
draw_link("Related links", "links.php");
sidebar_box_end();
}

function sidebar_stats_box()
{
sidebar_box_begin("Statistics");

// Last update
global $lastupdate;
global $filename;
echo "
<font class=\"lastupdate\">This page was last updated on<br>
<font class=\"lastupdatevalue\">$lastupdate GMT</font>.<br></font>";

// Counter
counter();

// "Valid html" logo
echo "
<br>
<!-- We have Valid HTML 4.01! -->
<a href=\"http://validator.w3.org/check/referer\"><img border=\"0\"
    src=\"pictures/valid-html401.png\"
    alt=\"Valid HTML 4.01!\" height=\"31\" width=\"88\"></a><br>
<!-- We also have Valid CSS! -->
<a href=\"http://jigsaw.w3.org/css-validator/\">
  <img style=\"border:0;width:88px;height:31px\"
       src=\"pictures/valid-css.png\"
       alt=\"Valid CSS!\">
 </a>";

sidebar_box_end();
}

function sidebar_download_box()
{
global $latestversion, $latestversiondate;
sidebar_box_begin("Download");
echo "
    Latest stable version: <font class=\"sidebardownloadversion\">$latestversion</font><br>
    Released: <font class=\"sidebardownloadversion\">$latestversiondate</font><br>
    <a class=\"sidebardownloadlink\" href=\"download.php\">Download now!</a>";
sidebar_box_end();
}


?>
