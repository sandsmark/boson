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
draw_bigbox_text("Boson is currently still under heavy development.
	<br><br>In the latest CVS version, the biggest changes, compared to latest stable release ($latestversion),
	are OpenGL support (Boson is now 3D), support for upgrades, a nicer commandframe and many other little modifications.
	Also, a large number of bugs have been fixed.<br><br>We're currently heading for release of version 0.7
	which will probably be released late 2002 or early 2003 (Note that these
	dates are <b>not</b> official and may change).<br><br>");

draw_bigbox_subheader("Feature list");
draw_bigbox_text("This is a list of features that I consider as
	&quot;to-be-done&quot; before release. Note that this is <b>not</b>
	the list of features for the next release, but just a TODO list!
	<h2>TODO</h2>
	<ul>
	  <li>Rename all .desktop files to .boson</li>
	  <li>Make map/scenario editor work. Maps should at least be modifyable,
	  optionally you should be able to create new maps. Loading/Saving code
	  is necessary for this.</li>
	  <li>Use UnitProperties in bounit</li>
	  <li>Commandframe (un)-docking must be fixed. See #48058</li>
	  <li>All units must have their own 3D model</li>
	  <li>Generate the technology tree dynamically.</li>
	</ul>
	<h2>DONE</h2>
	<ul>
	  <li>Port to OpenGL</li>
	  <li>Full 3D support using .3ds models</li>
	  <li>Explosions and smoke using particle effects</li>
	  <li>Lots of good documentations. Facility values are dynamically integrated in the docs and there is also a technology tree now.</li>
	  <li>Improved pathfinding</li>
	  <li>Experimental support for visible missiles</li>
	  <li>Greatly improved configurability for the cursor (including GUI for the config files)</li>
	  <li>New OpenGL options in the config dialog.</li>
	  <li>Options dialog has an Apply button - configs change when they are
	  pplied only, not immediately. There are also defaults for every entry.</li>
	  <li>Experimental support for unit animations (currenlty comsat only)</li>
	</ul>
	");

draw_bigbox_end();
//"

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
