<?php
/*
    This file is part of the Boson game
    Copyright (C) 2005 The Boson Team (boson-devel@lists.sourceforge.net)

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


/*****  Variables  *****/
$filename="authors.php";

/*****  Some includes  *****/
include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");


/*****  Functions  *****/
function add_author($name, $link, $contributions)
{
  if(strlen($link) == 0)
  {
    // No link
    echo "
      $name - $contributions<br>";
  }
  else if(strncmp($link, "http://", 7) == 0)
  {
    // Link is url
    echo "
      <a href=\"$link\" target=\"_blank\">$name</a> - $contributions<br>";
  }
  else
  {
    // Link is email
    echo "
      <a href=\"mailto:", str_replace("@", "@__NOSPAM__", $link), "\">$name</a> - $contributions<br>";
  }
}

/*****  Start of main stuff  *****/

start_page("Authors");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Authors of Boson");

    echo "
    <tr><td width=\"100%\">
      So here you can see who are actually behind Boson.<br>
      You could be in this list too - write to
      <a href=\"mailto:boson-devel@__NO_SPAM__lists.sourceforge.net\">our
      mailing list</a> and join us. There's plenty of things that has to be
      done.<br><br>";

    add_author("Andreas Beckermann", "b_mann@gmx.de",
        "Core developer and maintainer, release coordinator");
    add_author("Rivo Laks", "rivolaks@hot.ee",
        "Core developer, homepage design");
    add_author("Benjamin Adler", "benadler@gmx.net",
        "Graphics of the main themes (earth, humans), homepage design");
    add_author("Felix Seeger", "felix.seeger@gmx.de",
        "Boson Handbook, testing & bugreporting");
    add_author("Christopher J. Jepson (Xenthorious)", "xenthorious@yahoo.com",
        "Modeling & texturing");
    add_author("Thomas Capricelli", "orzel@freehackers.org",
        "Initial design and implementation, initial homepage, mailing-list maintainer");
    add_author("Nils Trzebin", "",
        "Music (progressive, jungle themes) and sounds");
    add_author("Ludovic Grossard", "http://perso.libertysurf.fr/grossard/boson/",
        "Music ('soft' theme)");
    add_author("Timo Huebel", "t.h@gmxpro.de",
        "Sound effects");

    echo "
    </td></tr>";

draw_bigbox_end();

main_area_end();

end_page();


?>
