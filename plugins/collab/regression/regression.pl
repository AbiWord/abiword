#!/usr/bin/perl
use File::Spec::Functions qw(rel2abs);
use File::Basename;

my $curdir=dirname(rel2abs($0));

my $filter_cmd = "sed -e 's/^\<version.*\>/\<!-- version tag removed --\>/g' -e 's/^\<history.*\>/\<!-- history tag removed --\>/g' -e 's/^\\s*\<title\>.*\<\\/title\>/\<!-- title tag removed --\>/g' -e 's/^\<\\/history.*\>/\<!-- \\/history tag removed --\>/g' -e 's/listid=\".*\"/\<!-- listid removed --\\>/g' -e 's/table-sdh:[a-zA-Z0-9]*/\<!-- table-sdh removed --\>/g' -e 's/shplid[0-9]*/\<!-- shplid removed --\>/g' -e 's/version=\".*\"/\<!-- version removed --\>/g' -e 's/fileref=\".*\" /\<!-- fileref removed --\>/g' -e 's/raw.*.html_files/\<!-- html_files directory filtered --\>/g' -e 's/xid=\".*\"/xid=\"stripped\"/g' -e 's/xid-max=\".*\"/xid-max=\"stripped\"/g' -e 's/top-xid=\".*\"/top-xid=\"stripped\"/g'";

sub AbwCompare {
	my ($reffile, $newfile) = @_;
	die unless $reffile;
	die unless $newfile;

	system("$filter_cmd $reffile 1>$reffile.f 2>$reffile.f");
	system("$filter_cmd $newfile 1>$newfile.f 2>$newfile.f");

	`diff -u $reffile.f $newfile.f > $newfile.diff`;
	$diff = `cat $newfile.diff`;

	unlink "$reffile.f";
	unlink "$newfile.f";

	if ($diff ne "") {
		return;
	} else {
		unlink "$newfile.diff";
	}

	return "true";
}


# main routine

my @sessions = split(/\s+/, `ls sessions/*.session`);
foreach my $session (@sessions) {
	$fqsn = "$curdir/$session";
	print "$fqsn\n";

	system("abiword-2.7 --plugin \"AbiWord Collaboration\" regression $fqsn");

	my $fqsna = "$fqsn.abw";
	if (-e $fqsna) {
		if (&AbwCompare("$fqsn.reference.abw", $fqsna)) {
			print "REGRESSION SUCCESS: session $fqsn\n";
			unlink $fqsna;
		} else {
			print "REGRESSION FAILED: different outcome ($fqsna.diff)!\n";
		}
	} else {
		print "REGRESSION FAILED: missing ouput file ($fqsna)\n";
	}
}
