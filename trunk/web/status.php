<?php


/*****  Variables  *****/
$filename="status.php";

/*****  Some includes  *****/
include("common.php");
include("sidebar.php");
include("main.php");
include("counter.php");
include("boxes.php");
include("variables.php");

/*****  Start of main stuff  *****/

do_start_stuff();

// Headers
html_print_header("Status");
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
draw_bigbox_begin("Status");
draw_bigbox_text("Boson is currently still under heavy development.<br><br>
  We're heading for release of version 0.8, but it is currently
  unclear when it will be released.<br><br>");

draw_bigbox_subheader("<a name=\"features\"></a>Feature list");
draw_bigbox_text("This is a list of features that I consider as
  &quot;to-be-done&quot; before release. Note that this is <b>not</b>
  the list of features for the next release, but just a TODO list!
  <h2>TODO</h2>
  <ul>
    <li>Support for height maps in editor</li>
    <li>3d-terrains in map files</li>
  </ul>
  <h2>DONE</h2>
  <blockquote>
  <h4>Internal</h4>
  <ul>
    <li>Height map for 3d-terrain</li>
    <li>New map file format (.tar.gz instead of .gz)</li>
    <li>Preload map information on startup</li>
    <li>Decrease size of textures (fixes problems for 24bpp and 32bpp)</li>
    <li>Don't draw invisible particles</li>
    <li>Big performance improvements</li>
  </ul>
  <h4>Game</h4>
  <ul>
    <li>Technology upgrades are working now</li>
    <li>Added minimap image for disabled minimap</li>
    <li>Weapon specific sound support</li>
    <li>Smoke support for powerplant, oilrefinery, mineralrefinery etc.</li>
    <li>Particle system improvements</li>
    <li>Damaged units are getting darker</li>
    <li>Save screenshots as jpg and show the full path as a chat message</li>
    <li>New minimap zooming buttons</li>
  </ul>
  <h4>Data</h4>
  <ul>
    <li>New unit: Ferry</li>
    <li>Unit models improvements</li>
    <li>Warfactory is now Weapons Factory</li>
    <li>Handbook switched from png images to jpg</li>
  </ul>
  <h4>Editor</h4>
  <ul>
    <li>Some bugs are fixed</li>
    <li>Buildings are placed in constructed mode</li>
    <li>Better support for big maps</li>
    <li>Support for changing map name and description</li>
  </ul>
  
  
  
  </blockquote>
  ");
main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
