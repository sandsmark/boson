package get_values;
use strict;

# Script to get the unit/facility information from the .desktop files.

sub getval() {
	# You need the facility type (fix / mod) the name and the filesystem level (root is the doc/language dir)
	my $self = shift;
	my $type = shift or die("Please define a facility type");	
	my $name = shift or die("Please define a facility name");
	my $level = shift or die("Please define your filesystem level");
	
	my $data_level_addon;
	my $doc_level_addon;

	# debug output
	#print "Facillity Type: $type\n";
	#print "Facility Name: $name\n";
	#print "Filesystem Level: $level\n";
	
	# I know that this is not the best way, but it is enough for now
	if ($level == 1) {
		$data_level_addon = "../../";
	}
	elsif ($level == 2) {
		$data_level_addon = "../../../";
	}
	elsif ($level == 3) {
		$data_level_addon = "../../../../";
	}
	
	my $path1;
	my $path2;
	$path1 = "$ENV{TOP_SRCDIR}/themes/species/human/units/$type\_$name/index.desktop";
	$path2 = "$ENV{TOP_SRCDIR}/doc/en/values.list";
	
	# Open index.destop and values.list
	open( FILE, "<$path1") || die "Cannot open $type\_$name/index.desktop: $ENV{TOP_SRCDIR}/themes/species/human/units/$type\_$name/index.desktop";
	open( FILE2, "<$path2") || die "Cannot open values.list: $ENV{TOP_SRCDIR}/doc/en/values.list";
	my @CONTENT;
	my @CONTENT2;
	
	@CONTENT = <FILE>;
	close(FILE);

	@CONTENT2 = <FILE2>;
	close(FILE2);

	print "<table border=\"1\">\n";

	my $line;
	my $line2;
	my $value;
	
	foreach $line (@CONTENT) {

		foreach $line2 (@CONTENT2) {
			# remove all spaces and \n
			$line =~ s/\s//g;
			$line2 =~ s/\s//g;
			$line2 =~ tr/\n//d;
			#print "trying: $line <--> $line2\n";
			if ($line =~ /^$line2=\s*(\w+)/ ) {
				$value = $1;
				print "<tr><td>$line2</td><td>$value</td></tr>\n";
			}
		}
	}
	print "</table>\n";}

1;
