<?php

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
