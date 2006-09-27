package make_tech_tree;
use strict;

# usage: command path_to_facilities [fixed]
#
# If you add "fixed" mobile units will be ignored

# AB: there might be problems with this script if the index.unit files were
# stored in dos mode!!
#
# FS:
# - there are problems if there are recursive dependences between the facilities..
# - print will print into the html file.
# - use print_mesg(message) for debug output.
# - don't print outside a sub.
# - exit code !=0 will give a wml error during make but no make error.


sub draw_map() {

if (eval {require Image::Magick})   {
    }
else    {
    # we need exit code 0, this way wml will not print an error during make and the user will get a nice note on the html page
    print "You need to install the perl \"image magick\" module to get this image generated !";
    exit(0);    
}

my $self = shift;
my $unittype = shift;

#change this to 1 to enable debug output
my $debug = 0;
#my $unitpath = $ARGV[0];
my $unitpath = "$ENV{TOP_SRCDIR}/themes/species/human/units/";



my @units;
opendir(UNITDIR, $unitpath) or die ("Oops - can\'t open dir $unitpath\n");

foreach my $entry (readdir(UNITDIR)) {
    if ($unittype eq "fixed") {
        next if ($entry eq "CVS" || $entry =~ /^\./ || $entry =~ /^mob.*/);
        }
    else {
        next if ($entry eq "CVS" || $entry =~ /^\./);
        }

	my $dir = $unitpath."/".$entry;
	if (-d $dir && -f $dir."/index.unit") {
		push(@units, $dir."/index.unit");
	}
}
closedir(UNITDIR);
if (@units < 1) {
	die ("Could not find units!");
}

my @idlist;
my %requirements;
my %names;
foreach my $unit (@units) {
    if ($unit =~ /prison|barracks/) { next; }
    my @file;
    open(FILE, "< $unit") or die ("Unable to open $unit");
    my $requirement;
    my $id;
    my $name;
    my $has_group = 0;
    my $eof = 0;
    while (<FILE>) {
        next if ($eof);
        chomp;
        $_ =~ s/\034//g; # continuation lines
        if (!$has_group) {
            if ($_ =~ /\[Boson Unit\]/) {
                $has_group = 1;
            }
            next;
        } elsif ($has_group && $_ =~ /\[.*\]/) {
            $eof = 1;
        }
        if ($_ =~ /^Requirements\s*=\s*/) {
            $_ =~ s/^Requirements\s*=\s*//;
            $requirement = $_;
        } elsif ($_ =~ /^Id\s*=\s*/) {
            $_ =~ s/^Id\s*=\s*//;
            $id = $_;
        } elsif ($_ =~ /^Name\s*=\s*/) {
            $_ =~ s/^Name\s*=\s*//;
            $name = $_;
        }
    }
    close(FILE);
    if ($id && $name) {
        push(@idlist, $id);
        $names{$id} = $name;
        if ($requirement) {
            $requirements{$id} = $requirement;
        }
    } else {
        print_mesg("No unit found in $unit\n");
    }
}


my $count = 0; # fallback, in case a unit can't be painted at all (e.g. recursive requirements)
my $maxcount = @idlist;
my @done_list; # list of already painted, i.e. completed units
my %num_per_line; # number of units/boxes of a line (aka width)
my $max_num_per_line = 0;

# first we calculate the width and height of the image
while ($count < $maxcount && @done_list < @idlist) {
	my @found = find_paintable(@idlist);
	my @done;
	foreach my $id (@found) {
		next if (find_value($id, \@done_list) > -1);
		push(@done, $id);
	}
	push(@done_list, @done);
	my $c = @done;
	if ($c > $max_num_per_line) {
		$max_num_per_line = $c;
	}
	$num_per_line{$count} = $c;
	$count++;
}
my $lines = keys(%num_per_line); # number of lines

# we add 5 pixels distance per unit. we need this for the lines... note that the
# distance to the top and to bottom of the image should be a different value!
# (but it is not yet)
my $ydistance = $max_num_per_line * 5; # distance between rects in y
my $imagewidth='800';
my $imageheight=$lines * 25 + ($lines + 1) * $ydistance; # 25 is a pretty much random value. we need textheight/boxheight here!
my $rect_inside_spacing = 3; # spacing from rect to the text inside the rect
@done_list = ();
$count = 0;

my $image = Image::Magick->new(size=>$imagewidth."x".$imageheight);
$image->Read('xc:black');

my %xposition; # id of unit -> x position of units' rectangle
my %yposition; # id of unit -> y position of bottom of units' rectangle
my %boxwidth;  # id of unit -> width of the drawn box/rectangle
my $y = $ydistance;
while ($count < $maxcount && @done_list < @idlist) {
	print_mesg("------------------count:$count---------------------\n");
	my @found = find_paintable(@idlist); # also returns all in @done_list!
	my @done;
	my $rectheight = 0;
	my @localnames;
	foreach (@found) {
		next if (find_value($_, \@done_list) > -1);
		push(@localnames, $names{$_});
	}
	my $complete_width = complete_text_width(\@localnames); # the width of all boxes in one line
	if ($complete_width > $imagewidth) {
		print_mesg("WARNING: complete text of this line is too wide for image!\n");
		$complete_width = $imagewidth;
	}
	my $xdistance = ($imagewidth - $complete_width) / (@found - @done_list + 1); # rest_width / (elements+1)
	my $x = $xdistance;
	foreach my $id (@found) {
		next if (find_value($id, \@done_list) > -1);

		push(@done, $id);
		paint_unit($id, $names{$id});
		print_mesg("   ");
		my $ret;

		my ($x_ppem, $y_ppem, $ascender, $descender, $textwidth, $textheight, $max_advance) = $image->QueryFontMetrics(text=>$names{$id});
		my $rectwidth = $textwidth + 2 * $rect_inside_spacing;
		$rectheight = $textheight + 2 * $rect_inside_spacing;

		my $x2 = $x + $rectwidth;
		my $y2 = $y + $rectheight;
		my $size = "$x,$y $x2,$y2";
		$ret = $image->Draw(primitive=>'rectangle',fill=>'lightblue',points=>$size);
		$xposition{$id} = $x;
		$yposition{$id} = $y2;
		$boxwidth{$id} = $rectwidth;
		if ($ret) {
			print_mesg($ret."\n");
			die;
		}
		my $xtextpos = $x + $rect_inside_spacing;
		my $ytextpos = $y + $rectheight - (($rectheight - 2 * $rect_inside_spacing) / 2);
		$ret = $image->Annotate(text=>$names{$id}, x=>$xtextpos, y=>$ytextpos);
		if ($ret) {
			print_mesg($ret."\n");
			die;
		}

		# now we draw the lines to the requirements. this is the tricky
		# part...
#		if ($count < 2) {
		my @req = split(/,/, $requirements{$id});
		print_mesg("for $id: @req\n");
		foreach (@req) {
#			draw_line($x, $rectwidth, $y - $rectheight, $_);
			draw_line($x, $rectwidth, $y, $_);
		}
#		}


		# increase x for the next rect
		$x = $x + $rectwidth + $xdistance;
	}
	if (@found > 0) {
		$y = $y + $rectheight + $ydistance;
		print_mesg("\n");
		push(@done_list, @done);
	}
	$count++;
}

my $file = "map.jpg";
my $small_file = "map_small.jpg";
print_mesg("Write to $file\n");
$image->Write($file);

$image->Resize(geometry=>'130');
$image->Write($small_file);



sub complete_text_width($)
{
	my ($array) = @_;
	my $width = 0;
	foreach (@$array) {
		my ($x_ppem, $y_ppem, $ascender, $descender, $textwidth, $textheight, $max_advance) = $image->QueryFontMetrics(text=>$_);
		$width = $width + $textwidth + 2 * $rect_inside_spacing;
	}
	return $width;
}
sub paint_unit($$)
{
	my $id = $_[0];
	my $name = $_[1];
	if (!$id) {
		print_mesg("paint_unit: oops!!\n");
		return;
	}
	print_mesg($id."=".$name);
}

sub find_value($$)
{
	my ($value, $list) = @_;
	my $i = 0;
	foreach my $entry(@$list) {
		if ($entry eq $value) {
			return $i;
		}
		$i++;
	}
	return -1;
}
sub find_paintable($)
{
	my @ids = @_;
	my @found_list;
	foreach my $id (sort @ids) {
		my @req = split(/,/, $requirements{$id});
		my $completed = 1;
		foreach my $r (@req) {
			if (find_value($r, \@done_list) == -1) {
				$completed = 0;
			}
		}
		if ($completed) {
			push(@found_list, $id);
		}
	}
	return @found_list;
}
sub draw_line($$$$)
{
	my ($fromx, $fromwidth, $fromy, $to_id) = @_;
	my $tox = $xposition{$to_id};
	my $toy = $yposition{$to_id};
	my $towidth = $boxwidth{$to_id};
	$fromx = $fromx + $fromwidth/2;
	$tox = $tox + $towidth/2;
	$image->Draw(primitive=>'line', stroke=>'lawngreen', points=>"$fromx,$fromy $tox,$toy");
}

sub print_mesg()
{
    if ($debug == 1) {
        print @_;
        }
}

}
1;

__END__
