package get_values;
use strict;

# Script to get the unit/facility information from the .unit files.

sub getval() {
        # You need the facility type (fix / mod) the name and the filesystem level (root is the doc/language dir)
        my $self = shift;
        my $type = shift or die("Please define a facility type");	
        my $name = shift or die("Please define a facility name");
        # this must be removed, but every facility file has to be changed too

        # debug output
        #print "Facillity Type: $type\n";
        #print "Facility Name: $name\n";

        my $path1;
        my $path2;
        $path1 = "$ENV{TOP_SRCDIR}/themes/species/human/units/$type\_$name/index.unit";
        $path2 = "$ENV{TOP_SRCDIR}/doc/en/values.list";

        # Open index.destop and values.list
        open( FILE, "<$path1") || die "Cannot open $type\_$name/index.unit $ENV{TOP_SRCDIR}/themes/species/human/units/$type\_$name/index.unit";
        open( FILE2, "<$path2") || die "Cannot open values.list: $ENV{TOP_SRCDIR}/doc/en/values.list";
        my @CONTENT;
        my @CONTENT2;

        @CONTENT = <FILE>;
        close(FILE);

        @CONTENT2 = <FILE2>;
        close(FILE2);

        print "<table border=\"1\" style=\"border-width:0;\" cellpadding=\"15\"><tr valign=\"top\"><td><table border=\"0\">\n";

        my $line;
        my $line2;
        my $value;
        my $weapon;
        my $upgrade;

        foreach $line (@CONTENT) {
                # parse [Boson Units]
                if ($line =~ /^\[Boson\sUnit\]\s*/) { print "<tr><td colspan=\"2\"><b>Unit Properties<b></td></tr>\n"; }
                #if ($line =~ /^\[Boson\sMobile\sUnit\]\s*/) { print "<tr><td colspan=\"2\">Boson Mobile Unit</td></tr>\n"; }
                
                if ($line =~ /^\[Weapon_\s*(\w+)/) {
                        $weapon = $1;
                        print "</table></td><td>\n";
                        print "<table border=\"0\">\n";
                        print "<tr><td colspan=\"2\"><b>Weapon $weapon</b></td></tr>\n";
                }
                
                if ($line =~ /^\[Upgrade_\s*(\w+)/) {
                        $upgrade = $1;
                        print "</table></td><td>\n";
                        print "<table border=\"0\">\n";
                        print "<tr><td colspan=\"2\"><b>Upgrade $upgrade<b></td></tr>\n";
                }
                
                foreach $line2 (@CONTENT2) {
                        # remove all spaces and \n
                        $line =~ s/\s//g;
                        $line2 =~ s/\s//g;
                        $line2 =~ tr/\n//d;
                        #print "trying: $line <--> $line2\n";
                        if ($line =~ /^$line2=\s*(\w+)/ ) {
                                $value = $1;
                                print "<tr><td width=\"110\">$line2</td><td>$value</td></tr>\n";
                                }
                        }
                }
        print "</td></tr></table></table>\n";}

1;
