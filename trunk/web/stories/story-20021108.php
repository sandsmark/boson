<?php


/*****  Variables  *****/
$filename="story-20021108.php";
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
html_print_header("Boson story: November 8, 2002");
print_header();

// Main table
main_table_begin();

// Sidebar
sidebar_begin();
  sidebar_links_box();
  sidebar_stories_box();
  sidebar_download_box();
  sidebar_stats_box();
sidebar_end();

main_area_begin();

// Contacts
draw_bigbox_begin("Boson story: November 8, 2002");
draw_bigbox_text("
<p class=\"announcement\">
The prologue:
General Andreas and his minions Rivo, Felix, Ben, Thomas and Scott had narrowly escaped doom
(<a href=\"../stories/story-20021017.php\">../stories/story-20021017.php</a>) by somehow shutting down the Boson Particle generator. No,
we don't know how either.
</p>
<p class=\"announcement\">
The scene:<br />
The good General is discussing the imminent launch of an assault on a nearby military base. Currently present: Andi,
Felix, Rivo, Timo and Scott.
</p>
<p class=\"announcement\">
* Andi joins #boson<br />
&lt;Andi&gt; How are we doing?<br />
&lt;Felix&gt; Sir! Both the eastern assault first and second wave divisions are ready to go.<br />
&lt;Andi&gt; And the defenses?<br />
&lt;Rivo&gt; Looking good sir.<br />
&lt;Andi&gt; Analysis of the enemy?<br />
&lt;Scott&gt; We can expect some light resistance from their sam sites and pill boxes but strangely their tanks don't
fire back.
</p>
<p class=\"announcement\">
A strange 'dying scream' sound comes from Timo's mouth. Everybody looks at him. Andi shakes his head.
</p>
<p class=\"announcement\">
&lt;Andi&gt; Ah yes, I keep meaning to add in support for that.
</p>
<p class=\"announcement\">
Everybody in the room nods knowingly.
</p>
<p class=\"announcement\">
&lt;Andi&gt; When can we proceed with the assault?<br />
&lt;Rivo&gt; We should be ready to head out on the 10th.<br />
&lt;Andi&gt; The day of the second year of the revival, how fitting.<br />
* Scott mutters \"Here it comes..\"<br />
&lt;Andi&gt; This time I *WILL* succeed! They *WILL* tremble at my feet!<br />
* Andi cackles like a madman... again<br />
&lt;Felix&gt; Déja vu...
</p>
<p class=\"announcement\">
&lt;Timo&gt; AH TTACKING!<br />
&lt;Felix&gt; Dude, will you shut up!?
</p>
");
draw_bigbox_end();

main_area_end();
main_table_end();

// Footers
print_footer();
html_print_footer();


?>
