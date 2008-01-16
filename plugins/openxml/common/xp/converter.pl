#!/usr/bin/perl
#
# recover the gperf form the generated file. sort of reversed gperf.
#

use strict;

my $line;
my %list;

while (defined($line = <>))
{
	chomp($line);
	$line =~ m/\"(..)\".*\"(....)\"/;
	if (defined($1) && defined($2) && !exists($list{$1})) {
		$list{$1}= $2;
	}
}

print "%struct-type\n";
print "%language=C++\n";
print "%define slot-name lang\n";
print "%define class-name OXML_LangToScriptConverter\n";
print "struct OXML_LangScriptAsso { char *lang; char *script; };\n";
print "%%\n";

my $key;
foreach $key (sort keys %list) {
	print "$key, \"$list{$key}\"\n";
}

print "%%\n";

