#!/usr/bin/perl
#
#   po2abi.pl
#
#   Converts a po file as produced by abi2po into an AbiWord
#   localised strings file

$abiroot = ".";

while (<STDIN>){
    if (/^#: (.*)/){
	$msgid = $1;
	#print "id=$msgid:";
    } elsif (/^msgstr \"(.*)\"/) {
	$string = $1;
	$strings{$msgid} = $string;
	#print "str=$string\n";
    }
}
close(STDIN);

$targetLang = $ARGV[0];
#print "lang=$targetLang\n";

print
"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n\n".
"<!-- ==============================================================  -->\n".
"<!-- This file contains AbiWord Strings.  AbiWord is an Open Source  -->\n".
"<!-- word processor developed by AbiSource, Inc.  Information about  -->\n".
"<!-- this application can be found at http://www.abisource.com       -->\n".
"<!-- This file contains the string translations for one language.    -->\n".
"<!-- This file is covered by the GNU Public License (GPL).           -->\n".
"<!-- ==============================================================  -->\n\n";

if (open(CREDITS, "< $abiroot/$targetLang.credits")){
    while (<CREDITS>){
	print "$_";
    }
    close(CREDITS);
} else {
    print "<!-- $targetLang translation provided by ?? -->\n";
}
print "\n\n";

print "<AbiStrings app=\"AbiWord\" ver=\"1.0\" language=\"$targetLang\">\n\n";


## en-US is in a different file in a different format
my $lang = 'en-US';

print "<Strings	class=\"XAP\"\n";

open(STRINGS, "< $abiroot/src/af/xap/xp/xap_String_Id.h" )
    or die "Cannot open xap_String_Id.h:$!";

while (<STRINGS>) {
    next unless /\((.*)\s*,\s*\"(.*)\"/;
    my ($msgid,$string) = ($1,$2);
    print "$msgid=\"$strings{$msgid}\"\n";
}
close(STRINGS);
print "/>\n\n";

print "<Strings	class=\"AP\"\n";

open(STRINGS2, "< $abiroot/src/wp/ap/xp/ap_String_Id.h") or die "Cannot open ap_String_Id.h:$!";
while (<STRINGS2>) {
    next unless /\((.*)\s*,\s*\"(.*)\"/;
    my ($msgid,$string) = ($1,$2);
    print "$msgid=\"$strings{$msgid}\"\n";
}
close(STRINGS2);
print "/>\n\n";

print "</AbiStrings>\n\n";
