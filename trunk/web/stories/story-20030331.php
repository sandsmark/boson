<?php
/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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


/*****  Variables  *****/
$filename="story-20021108.php";
$basedir="../";

/*****  Some includes  *****/
include_once("${basedir}common.inc");
include_once("${basedir}sidebar.inc");
include_once("${basedir}counter.inc");

/*****  Start of main stuff  *****/

start_page("Boson story: March 31, 2003");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_stories_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson story: March 31, 2003");
draw_bigbox_text("
<p class=\"announcement\">
The scene: Timo and Rivo are sitting in the war room discussing the seemingly
endless war effort. Andi walks in..<br>
Andi: Moin, how is everything today?<br>
Rivo: Re! Other than a few minor problems with the artillery and the planes,
everything is going good<br>
Andi: problems?<br>
Rivo: yes.. our tanks arent used to hills and the planes are turning out to be a
one shot deal.<br>
</p>

<p class=\"announcement\">
Rivo shudders, suddenly recalling the events of the day before in the Air
Control Tower..<br>
Felix, piloting one of the new planes: Sir! We are in trouble!<br>
Rivo: What do you mean.. trouble?<br>
Felix: Sir, we are under attack and my plane refuses to land!<br>
* The sound of gunfire is heard over the radio<br>
Rivo: Well shoot back at them!<br>
Felix: I can't! I am completely out of ammo<br>
Rivo, to Thomas: What does he mean, he can't land?<br>
Thomas: Well, the planes don't have that ability yet.<br>
Rivo: What!? What's the point of flying them yet?<br>
Thomas: Ah, good question.<br>
Felix: Help! A missile is coming right at me!<br>
</p>

<p class=\"announcement\">
Andi: I see.. any word from CINCPROP?<br>
Timo: No sir, the Commander In Chief of Propaganda is still missing<br>
Andi: Hasn't it been like 4 months now?<br>
Timo: Yes sir<br>
</p>

<p class=\"announcement\">
Andi: So you are saying that the tanks keep crashing, the planes are useless,
CINCPROP is missing and, as if that wasn't enough, my Lieutentant isn't even
paying attention to me?<br>
* Rivo snaps out of his daydream and blushes<br>
Rivo: Yes sir.<br>
Andi: I see. Is there any good news at all?<br>
Rivo: Well, our best pilot was saved from imminent doom because of a small
glitch in the enemie's missiles<br>
");
draw_bigbox_end();

main_area_end();

end_page();


?>
