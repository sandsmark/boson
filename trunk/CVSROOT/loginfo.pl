#!/usr/bin/perl

# free of any copyright. Written by taj@kde.org with small changes
# by coolo@kde.org and dirk@kde.org
# partly rewritten to add links to webcvs by daniel.naber@t-online.de

# edited by abmann@users.sourceforge.net to fit into the boson project (taken from KDE)

use Socket;

$outText = "";
$diffText = "";
$author = "";
$module = "";
$recipient = "boson-cvs\@lists.sourceforge.net";
$ccrecipients = "";
$from = "";
$tagname = "HEAD";

#$cvsweb='http://webcvs.kde.org/cgi-bin/cvsweb.cgi';
#$cvsweb_head='X-WebCVS:';

# don't blow up the message with too many webcvs links:
$max_diffs = 5;
$count_diffs = 0;
$stop_diffs = 0;

$juststarted = 1;
$beforelogmsg = 1;
$infiles_modified = 0;
$infiles_added = 0;
$infiles_removed = 0;

# for linking webcvs:
%rev1={};
%rev2={};

while(<>) {
#        next if /(^Update of .home.kde.*|^In directory cvs.*)/; 

	if( $beforelogmsg == 0 && /^CCMAIL:/ ) {
		$ccrecipients = "\nCc: ".substr( $_, 7, -1);
		next
	}

	if( /CVS.?SILENT/ ) {
		$recipient = "abmann\@users.sourceforge.net";
                $from = "CVS silently by ";
	}

#	# for linking webcvs:
#	if( $juststarted ) {
#	      next if /^\s*$/;
#	      @files = split /\s/,$_;
#	      $dir = shift @files;
#	      foreach $f (@files) {
#		($file,$r1,$r2) = $f =~ /(.+),(.+),(.+)/;
#		$files .= ' ' . $file;
#		$rev1{$file} = $r1;
#		$rev2{$file} = $r2;
#	      }
#	      $juststarted = 0;
#	}
	
	if (/^Log Message:/) {
              $beforelogmsg = 0;
              $infiles_modified = 0;
              $infiles_added = 0;
              $infiles_removed = 0;
	} elsif (/^Modified Files:/) {
              $infiles_modified = 1;
              $outText = $outText."\n".$_;
	      next;
	} elsif (/^Added Files:/) {
              $infiles_added = 1;
              $outText = $outText.$_;
	      next;
	} elsif (/^Removed Files:/) {
              $infiles_removed = 1;
              $outText = $outText.$_;
	      next;
        }

	if ($beforelogmsg == 1 && /\s*Tag: (\S*)/) {
	      $tagname = $1;
#        } elsif ($infiles_modified == 1 || $infiles_added == 1 || $infiles_removed == 1) {
#	      # link the files/diffs in webcvs:
#	      @fls=split;
#	      $starttag = 0;
#              $outText .= " " x 8;
#	      foreach $f (@fls) {
#		 if ($f =~ /^Tag:$/) {
#        	   $starttag = 1;
#		 } elsif ($starttag == 1) {
#                   $outText = $outText."Tag: $f\n";
#        	   $starttag = 0;
#		 } else {
#                   $outText = $outText." ".$f;
#	      	   if( $count_diffs < $max_diffs ) {
#		      if( $infiles_added || $infiles_removed || $infiles_modified ) {
#                	$diffText .= $cvsweb_head." $cvsweb/$dir/$f\n";
#			$count_diffs++;
#		      }
#        	      $r1 = $rev1{$f};
#        	      $r2 = $rev2{$f};
#        	      if ($r2 gt "" && $r1 gt "" && $r1 ne "NONE" && $r2 ne "NONE") {
#        		$diffText .= $cvsweb_head." $cvsweb/$dir/$f.diff?r1=".$r1."&r2=".$r2."\n";
#			$count_diffs++;
#        	      }
#		   } elsif( ! $stop_diffs )  {
#        	      $diffText .= $cvsweb_head." stop listing files to prevent bloat\n";
#		      $stop_diffs = 1;
#		   }
#		 }
#	      }
#              $outText = $outText."\n";
	} else {
           $outText = $outText.$_;
           chop;
	}
        
        if( $module eq "" ) {
                /^\s*([^\s]+)\s+/;
                $module = $1;
        }

        if ( /^\s*Author:\s*(\w+)/ ) {
                $author = $1 if (!$author);
        }
}

$subject = "";

if ($tagname eq 'HEAD') {
   $subject=$module;
} else {
   $subject="$tagname: $module";
}

# boson does not have access to such a file
#open(INFO, "/usr/bin/cvs -d /home/kde co -p kde-common/accounts 2>/dev/null |");
#my @info = grep /^$author\s*(.*)\s+\S+\s*$/, <INFO>;
#close(INFO);

my $blame = "$author <boson-cvs\@lists.sourceforge.net>";
#$blame = "$1 <$2>" if (defined($info[0]) && $info[0] =~ /^\S*\s+(\S.+\S)\s\s*(\S+)\s*$/);

open (MAIL, "|/usr/lib/sendmail -t") or die "Could not open sendmail: $!";
print MAIL<<EOF;                                 
From: $from$blame
To: $recipient$ccrecipients
Subject: $subject
$diffText

$outText
.
EOF

close(MAIL);
