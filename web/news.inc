<?php
/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


include_once("common.inc");
/*
 * class Article - a class structure to hold information about a particular
 *       news item
 */
class Article
{
    var $date;
    var $title;
    var $author;
    var $authorlink;
    var $text;
    var $id;
    function Article()
    {
        $this->date="";
        $this->title="";
        $this->author="";
        $this->authorlink="";
        $this->text="";
        $this->id=-1;
    }

    /*
    function Article($title, $author, $authorlink, $date, $text)
    {
        $this->date=$date;
        $this->title=$title;
        $this->author=$author;
        $this->authorlink=$authorlink;
        $this->text=$text;
    }
    */
    function set_date($arg) { $this->date = $arg; }
    function set_title($arg) { $this->title = $arg; }
    function set_author($arg) { $this->author = $arg; }
    function set_authorlink($arg) { $this->authorlink = $arg; }
    function set_text($arg) { $this->text = $arg; }
    function set_id($arg) { $this->id = $arg; }
    function append_text($arg) { $this->text .= $arg; }
    function print_me()
    {
        echo " <!-- News item with title $this->title -->";
        echo "
            <tr>
                <td class=\"bigboxsubheadercell\">
                <a name=\"$this->id\"></a>
                    <font class=\"newstitle\">&nbsp;$this->title</font><font class=\"newsdate\">&nbsp;&nbsp;$this->date</font>";
        if($this->author != "")
        {
          if($this->authorlink != "")
          {
            echo "
                    <font class=\"newsauthor\">&nbsp;&nbsp;&nbsp;(posted by: <a href=\"$this->authorlink\" class=\"newsauthorlink\">$this->author</a>)</font>";
          }
          else
          {
            echo "
                    <font class=\"newsauthor\">&nbsp;&nbsp;&nbsp;(posted by: $this->author)</font>";
          }
        }
        echo "
                </td>
            </tr>
            <tr><td class=\"newstext\">
              $this->text
            </td></tr>";
    }
    function print_link()
    {
        echo "<a href='news.php#$this->id'><b>$this->title</b></a>
            <br><i>&nbsp;&nbsp;$this->date</i><br><br>
            ";
    }

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

function get_article_array($max_num_articles = 10)
{
    global $news_file;
    $articles[] = new Article();
    $index = -1;

    $fd = fopen($news_file, "r");
    $read_text = 0;
    //echo "retriving '$max_num_articles' number of articles\n";


    while (!feof ($fd))
    {
        $buffer = fgets($fd, 4096);
        if (preg_match("/\[(.*)\]/", $buffer, $matches))
        {
            $read_text = 0;
            $index++;
            //if ($index >= $main_news_max)
            if ($index >= $max_num_articles)
            {
                break;
            }
            $current_article = &$articles[$index];
            $current_article = new Article();
            $current_article->set_id($index);
        }
        else if ($split_arr = preg_split("/=/", $buffer))
        {
            if($split_arr[0] == "date")
            {
                $current_article->set_date(rtrim($split_arr[1]));
            }
            else if($split_arr[0] == "title")
            {
                $current_article->set_title(rtrim($split_arr[1]));
            }
            else if($split_arr[0] == "author")
            {
                $current_article->set_author(rtrim($split_arr[1]));
            }
            else if($split_arr[0] == "authorlink")
            {
                $current_article->set_authorlink(rtrim($split_arr[1]));
            }
            else if($split_arr[0] == "text")
            {
                $current_article->set_text($split_arr[1]);
                $read_text = 1;
            }
            else if($read_text)
            {
                $current_article->append_text($buffer);
            }
        }
    }
    fclose($fd);
    return $articles;

}

/*
 * display_main_nes() - displays news in the main web page
 */
function display_main_news()
{
    $articles[] = new Article();
    global $main_news_max, $old_news_max;

    $articles = get_article_array($main_news_max);
    for ($i = 0; $i < count($articles); $i++)
    {
        echo $articles[$i]->print_me();
        echo "\n";
    }
}

/*
 * display_old_news() - displays old news items in the side bar
 */
function display_old_news()
{
    $articles[] = new Article();
    global $main_news_max, $old_news_max;

    $articles = get_article_array(($main_news_max+$old_news_max));
    for ($i = $main_news_max; $i < count($articles); $i++)
    {
        $articles[$i]->print_link();
    }
}

?>
