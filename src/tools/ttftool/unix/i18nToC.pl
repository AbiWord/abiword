#!/bin/perl

if($#ARGV == -1) {
	usage();
}

open IF, $ARGV[0] or die "cannot open input file";
open OF, ">$ARGV[0].cstyle" or die "cannot open output file";

$CMAP_START=0;
$LINE_COUNT=0;

while (<IF>) {
    if(/^CHARMAP/){
    	$CMAP_START = 1;
    	next;
    }

    if(!$CMAP_START){
       	next;
    }

	if(/^END CHARMAP/){
		last;
	}
    	
	if(/^<U(\w{4})>/) {
		  s/^<U(\w{4})>.*\n/0x$1\, /;
		
		  if ($LINE_COUNT == 0){
		  	print OF "\t\t";
		  }
		
          $LINE_COUNT++;
		  print OF $_;
		
		  if($LINE_COUNT == 8){
		  	$LINE_COUNT=0;
		  	print OF "\n";
		  }
	}
		
}

print "\nFinished: values written to file $ARGV[0].cstyle.\n\n";
print "-------------- WARNING !!! ------------------------------\n";
print "| Check that the file contains all 256 values (32 rows, |\n";
print "| 8 numbers each). Some i18n charmaps are incomplete.   |\n";
print "| In that case you will have to fix the file by hand.   |\n";
print "---------------------------------------------------------\n\n";

close IF;
close OF;

sub usage()
{
	print "\n--------------- i18nToC -------------------------------------\n\n";
	print "This script converts a i18n charmap into a format suitable\n";
	print "for use in AbiWord's /src/tools/ttftool/unix/encoding.h\n\n";
	print "Usage:\n";
	print "       i8nToC filename\n";
	print "              filename: a i18n charmap\n\n";
	print "              the output is stored in filename.cstyle\n";
	print "---------------------------------------------------------------\n\n";
	exit 0;
}