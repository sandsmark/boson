<?php

/**
* Draws little box with $title in it
**/
function draw_box($title)
{
echo "
<table cellspacing=\"1\" cellpadding=\"2\" bgcolor=\"#8080ff\" width=\"100%\">
  <tr><td bgcolor=\"#b0b0ff\">
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
<table cellspacing=\"1\" cellpadding=\"2\" bgcolor=\"#8080ff\" width=\"100%\">
  <tr><td bgcolor=\"#b0b0ff\">
    <font class=\"bigboxtitle\">&nbsp;&nbsp;$title</font>
  </td></tr>
  <tr><td bgcolor=\"#e0e0ff\" align=\"center\">
    <br>
    <table cellspacing=\"0\" cellpadding=\"0\" bgcolor=\"#e0e0ff\" width=\"99%\">";
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
      <tr><td bgcolor=\"#c0c0ff\">
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
        <br>
        <font class=\"$style\">$text</font>
        <br><br>
      </td></tr>";
}

?>
