<?php

function news_article($title, $date, $fulltext)
{
echo "
      <!-- News item with title $title -->";
echo "
      <tr><td bgcolor=\"#c0c0ff\">
        <font class=\"newstitle\">&nbsp;$title</font><font class=\"newsdate\">&nbsp;&nbsp;$date</font>
      </td></tr>";
draw_bigbox_text($fulltext, "newstext");
}

function news_box_begin()
{
echo "
<!-- News box -->";
draw_bigbox_begin("News");
}

function news_box_end()
{
draw_bigbox_end();
}

?>
