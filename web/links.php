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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


/*****  Variables  *****/
$filename="links.php";

/*****  Some includes  *****/
include_once("common.inc");
include_once("sidebar.inc");
include_once("counter.inc");

/*****  Start of main stuff  *****/

start_page("Links");

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Links");
draw_bigbox_text("Here you will find some (hopefully) useful links<br><br>");

draw_bigbox_subheader("Misc");
draw_bigbox_text("
  <a href=\"http://sf.net/projects/boson/\" target=\"_blank\">Boson at SourceForge</a> Boson uses SourceForge.net's services<br>
  <a href=\"http://boson.halux2001.de/handbook/en\" target=\"_blank\"> The Boson handbook</a><br>
  <a href=\"http://www.kde.org/\" target=\"_blank\">KDE homepage</a> Boson uses KDE<br>
  <a href=\"http://www.trolltech.com\" target=\"_blank\">Trolltech (creators of QT)</a> Boson also uses Qt<br>
  <a href=\"http://www.freehackers.org/about/content.html\" target=\"_blank\">freehackers.org</a> Other free software<br>
  <a href=\"http://sourceforge.net\" target=\"_blank\">SF.net</a> SourceForge.net<br>
  <br>");
draw_bigbox_subheader("Games");
draw_bigbox_text("
  <a href=\"http://www.nongnu.org/stratagus/\" target=\"_blank\">Stratagus</a> an real-time strategy gaming engine, successor of FreeCraft<br>
  <a href=\"http://www.shadowconflict.com/projectinferno/\" target=\"_blank\">Project Inferno</a> an Open Source Real Time Strategy Engine<br>
  <a href=\"http://freecnc-sf.holarse.net/\" target=\"_blank\">FreeCNC</a> Command & Conquer clone for Unices<br>
  <a href=\"http://www.asc-hq.org/\" target=\"_blank\">ASC</a> Turn based strategy game<br>
  <a href=\"http://vegastrike.sourceforge.net/\" target=\"_blank\">Vegastrike</a> Open Source 3D space simulator. Not really related to boson (no RTS or so), but I consider this very important, so...<br>
  <a href=\"http://www.freeciv.org/\" target=\"_blank\">Freeciv</a> Strategy game. Neither real time nor OpenGL, but it is playable and since there aren't so many &quot;big&quot; free games that are playable it is listed here.<br>
  <a href=\"http://www.glest.org/\" target=\"_blank\">Glest</a> Real time 3D strategy game for windows.<br>
  <a href=\"http://taspring.clan-sy.com/\" target=\"_blank\">TA: Spring</a> Real time 3D strategy game for windows. A linux port is being worked on.<br>
  <a href=\"http://www.cs.ualberta.ca/~mburo/orts/orts.html\" target=\"_blank\">ORTS</a> Real time 3D strategy game environment. ORTS is primarily meant for studying real-time AI problems.<br>
  <br>");
draw_bigbox_subheader("Development");
draw_bigbox_text("
  <a href=\"http://plib.sourceforge.net\" target=\"_blank\">PLIB</a> A OpenGL widget/font/scenegraph library. Also includes a library for sound and network (and more).<br>
  <a href=\"http://www.libsdl.org/\" target=\"_blank\">SDL</a> Probably the most important unix game library<br>
  <a href=\"http://crystal.sourceforge.net/\" target=\"_blank\">Crystal Space</a> Portable 3D Game Development Kit<br>
  <a href=\"http://www.talula.demon.co.uk/allegro/\" target=\"_blank\">Allegro</a> A game programming library for C/C++ developers</br>
  <a href=\"http://www.gamasutra.com/\" target=\"_blank\">Gamasutra</a> A LOT of high quality articles related to game programming. Usually you need to search in the archives a while to find what you can use.<br>
  <a href=\"http://www-cs-students.stanford.edu/~amitp/gameprog.html\" target=\"_blank\">Amits Game Programming Information</a> You can find answers to general questions on game programming here, but more important is probably the collection of links on AI and pathfinding.<br>
  <a href=\"http://www.planetquake.com/gg/fmp/fga.html\" target=\"_blank\">Free game arts</a> Collection of textures, models and so on. Not all are free (as in speech)!<br>
  <a href=\"http://www.gamedev.net/\" target=\"_blank\">Gamedev.net</a> Many very useful articles and tutorials here<br>
  <br>");
draw_bigbox_end();

main_area_end();

end_page();


?>
