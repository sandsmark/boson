<?php


/*****  Variables  *****/
$filename="story-20021017.php";
$basedir="../";

/*****  Some includes  *****/
include("${basedir}common.php");
include("${basedir}sidebar.php");
include("${basedir}main.php");
include("${basedir}counter.php");
include("${basedir}boxes.php");
include("${basedir}variables.php");

/*****  Start of main stuff  *****/

do_start_stuff();

// Headers
html_print_header("Boson story: October 17, 2002");
print_header();

// Main table
main_table_begin();

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_announcements_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson story: October 17, 2002");
draw_bigbox_text("
<p class=\"announcement\">
The scene:
</p>
<p class=\"announcement\">
Rivo is sitting at a computer, programming the finishing touches on Boson's 
particle system. Felix is sitting at the debug console, monitoring the 
output. General Andreas is pacing the room. Ben is working on a 3d model for 
something. Scott and Thomas are standing back, bemused.
</p>
<p class=\"announcement\">
&lt;Rivo&gt; General, the particle system is complete!<br />
&lt;Andi&gt; Excellent! Test it immediately!<br />
&lt;Rivo&gt; Yes sir.
</p>
<p class=\"announcement\">
The room goes silent as Rivo flips the switch. An extremely bright light is 
seen behind the plexiglass containment section of the room. Everyone stares 
in awe at the magnificat beauty.
</p>
<p class=\"announcement\">
Nobody hears Andi mutter to himself:<br />
&lt;Andi&gt; The power! It's mine! MINE! The world will bow to ME!
</p>
<p class=\"announcement\">
&lt;Rivo&gt; We have reached maximum Boson particle output. 40 frames per second and 
holding.<br />
* Felix glances at his screen<br />
&lt;Felix&gt; General! The particles! They are cascading! We have created a quantum
singularity!<br />
&lt;Ben&gt; How the hell am I supposed to skin that!?<br />
* Andi cackles like a madman
</p>
<p class=\"announcement\">
Small things start moving towards the containment area. The plexiglass starts warping under the extreme gravitational
stress.
</p>
<p class=\"announcement\">
&lt;Felix&gt; Rivo! Shut it down!<br />
&lt;Rivo&gt; I can't! It's too late!<br />
&lt;Scott&gt; Oh... shit
</p>
");
draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
