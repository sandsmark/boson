package get_values;
use strict;

# Script to get the unit/facility information from the .unit files.

sub getval() {
        # You need the facility type (fix / mod) and the name 
        my $self = shift;
        my $type = shift or die("Please define a facility type");	
        my $name = shift or die("Please define a facility name");
        
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
        my @CONTENT_SORTED;

        @CONTENT = <FILE>;
        close(FILE);

        @CONTENT2 = <FILE2>;
        close(FILE2);

        print "<table border=\"0\"><tr valign=\"top\"><td><table border=\"0\" class=\"values\">\n";

        my $line;
        my $line2;
        my $value;
        my $weapon;
        my $upgrade;
        my $passed;

        my $in_loop;
        my @boson_unit;
        my @weapon1;
        my @weapon2;
        my @weapon3;

        foreach $line (@CONTENT) {
            if ($line =~ /^\[Boson\sMobile\sUnit\]\s*/ or $in_loop eq 1) {
                if ($line =~ /^\[.*/ and $in_loop eq 1) { last; }
                if ($in_loop eq 1) {
                    @boson_unit = (@boson_unit, "$line\n");
                }
                $in_loop = 1;
            }
        }
        $in_loop = 0;
        foreach $line (@CONTENT) {
            if ($line =~ /^\[Boson\sUnit\]\s*/ or $in_loop eq 1) {
                if ($line =~ /^\[.*/ and $in_loop eq 1) { last; }
                if ($in_loop eq 1) {
                    @boson_unit = (@boson_unit, "$line\n");
                }
                $in_loop = 1;
            }
        }
        $in_loop = 0;
        foreach $line (@CONTENT) {
            if ($line =~ /^\[Weapon_0.*\]\s*/ or $in_loop eq 1) {
                if ($line =~ /^\[.*/ and $in_loop eq 1) { last; }
                if ($in_loop eq 1) {
                    @weapon1 = (@weapon1, "$line\n");
                }
                $in_loop = 1;
            }
        }
        $in_loop = 0;
        foreach $line (@CONTENT) {
            if ($line =~ /^\[Weapon_1.*\]\s*/ or $in_loop eq 1) {
                if ($line =~ /^\[.*/ and $in_loop eq 1) { last; }
                if ($in_loop eq 1) {
                    @weapon2 = (@weapon2, "$line\n");
                }
                $in_loop = 1;
            }
        }
        $in_loop = 0;
        foreach $line (@CONTENT) {
            if ($line =~ /^\[Weapon_2.*\]\s*/ or $in_loop eq 1) {
                if ($line =~ /^\[.*/ and $in_loop eq 1) { last; }
                if ($in_loop eq 1) {
                    @weapon3 = (@weapon3, "$line\n");
                }
                $in_loop = 1;
            }
        }
        #print @boson_unit;
        #print @weapon1;
        #print @weapon2;
        #print @weapon3;


        my %boson_unit_hash;
        my %weapon1_hash;
        my %weapon2_hash;
        my %weapon3_hash;
        
        if (@boson_unit >= 1) {
            print "<tr><td colspan=\"2\" class=\"thumb_header\"><b>Unit Properties</b></td></tr>\n";
            foreach $line (@boson_unit) {
                foreach $line2 (@CONTENT2) {
                    $line2 =~ s/\s//g;
                    $line2 =~ tr/\n//d;
                    if ($line =~ /^$line2=\s*(.*)/ ) {
                        $value = $1;
                        if ($value ne "") {
                            %boson_unit_hash = (%boson_unit_hash, $line2, $value);
                        }
                    }
                }
            }
            foreach $line2 (@CONTENT2) {
                if ($boson_unit_hash{$line2} ne "") {
                    print "<tr><td width=\"110\">$line2</td><td>$boson_unit_hash{$line2}</td></tr>\n";
                }
            }
        }

        if (@weapon1 >= 1) {
            print "</table><td><table class=\"values\">";
            print "<tr><td colspan=\"2\" class=\"thumb_header\"><b>Weapon 1</b></td></tr>\n";
            foreach $line (@weapon1) {
                foreach $line2 (@CONTENT2) {
                    $line2 =~ s/\s//g;
                    $line2 =~ tr/\n//d;
                    if ($line =~ /^$line2=\s*(.*)/ ) {
                        $value = $1;
                        if ($value ne "") {
                            %weapon1_hash = (%weapon1_hash, $line2, $value);
                        }
                    }
                }
            }
            foreach $line2 (@CONTENT2) {
                if ($weapon1_hash{$line2} ne "") {
                    print "<tr><td width=\"110\">$line2</td><td>$weapon1_hash{$line2}</td></tr>\n";
                }
            }
        }

        if (@weapon2 >= 1) {
            print "</table><td><table class=\"values\">";
            print "<tr><td colspan=\"2\" class=\"thumb_header\"><b>Weapon 2</b></td></tr>\n";
            foreach $line (@weapon2) {
                foreach $line2 (@CONTENT2) {
                    $line2 =~ s/\s//g;
                    $line2 =~ tr/\n//d;
                    if ($line =~ /^$line2=\s*(.*)/ ) {
                        $value = $1;
                        if ($value ne "") {
                            %weapon2_hash = (%weapon2_hash, $line2, $value);
                        }
                    }
                }
            }
            foreach $line2 (@CONTENT2) {
                if ($weapon2_hash{$line2} ne "") {
                    print "<tr><td width=\"110\">$line2</td><td>$weapon2_hash{$line2}</td></tr>\n";
                }
            }
        }

        if (@weapon3 >= 1) {
            print "</table><td><table class=\"values\">";
            print "<tr><td colspan=\"2\" class=\"thumb_header\"><b>Weapon 3</b></td></tr>\n";
            foreach $line (@weapon3) {
                foreach $line2 (@CONTENT2) {
                    $line2 =~ s/\s//g;
                    $line2 =~ tr/\n//d;
                    if ($line =~ /^$line2=\s*(.*)/ ) {
                        $value = $1;
                        if ($value ne "") {
                            %weapon3_hash = (%weapon3_hash, $line2, $value);
                        }
                    }
                }
            }
            foreach $line2 (@CONTENT2) {
                if ($weapon3_hash{$line2} ne "") {
                    print "<tr><td width=\"110\">$line2</td><td>$weapon3_hash{$line2}</td></tr>\n";
                }
            }
        }

print "</table></td></tr></table>";

}                


sub getIndexVals() {
        # You need the facility type (fix / mod) and the name
        my $self = shift;
        my $type = shift or die("Please define a facility type");	
        my $name = shift or die("Please define a facility name");
        
        
        if ($name ne "map") {
        my $path1;
        $path1 = "$ENV{TOP_SRCDIR}/themes/species/human/units/$type\_$name/index.unit";

        # Open index.destop and values.list
        open( FILE, "<$path1") || die "Cannot open $type\_$name/index.unit $ENV{TOP_SRCDIR}/themes/species/human/units/$type\_$name/index.unit";
        
        my @CONTENT;

        @CONTENT = <FILE>;
        close(FILE);

        my @searchValues;
        
        @searchValues = ("Health", "OilCost", "MineralCost");

        print "<table border=\"0\" width=\"130\" class=\"values\">\n";

        my $line;
        my $found_value;
        my $value;
        my $health_lock = 0;
        
        foreach $line (@CONTENT) {
                foreach $found_value (@searchValues) {
                        # remove all spaces and \n
                        #$line =~ s/\s//g;
                        $found_value =~ s/\s//g;
                        $found_value =~ tr/\n//d;
                        #print "trying: $line <--> $line2\n";
                        if ($line =~ /^$found_value=\s*(.*)/ ) {
                                $value = $1;
                                # Workaround for technology upgrades. Only show the first Health in the file.
                                if ($found_value eq "Health") {
                                    $health_lock = $health_lock + 1;
                                }
                                if ($health_lock < 2) {
                                    print "<tr><td>$found_value</td><td>$value</td></tr>\n";
                                }
                        }
                }
        }
        print "</table>\n";
        }
}


1;

__END__



# TODO

- Allow translations in the values.list, we than don't read a value from a line, we read a value (which will be searched in the config file) and a translation
  Also the hard coded headers for the tables should go to values.list file


 
