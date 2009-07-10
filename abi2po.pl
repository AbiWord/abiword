#!/usr/bin/perl
#
#   abi2po.pl
#
#   Converts the AbiWord strings files into something resembling a
#   gettext po file

$targetLang = $ARGV[0];
$abiroot = ".";

## en-US is in a different file in a different format
my $lang = 'en-US';
foreach my $file ("$abiroot/src/wp/ap/xp/ap_String_Id.h", "$abiroot/src/af/xap/xp/xap_String_Id.h") {
  open(STRINGS, "< $file" )
    or die "Cannot open $file";
  
  while (<STRINGS>) {
    next unless /\((.*)\s*,\s*\"(.*)\"/;
    my ($msgid,$string) = ($1,$2);
    $strings{$msgid}{$lang} = $string;
  }
  close(STRINGS);
}


## Read in each of the other language files 
## and process them appropriatly
$stringsfile = "$abiroot/user/wp/strings/$targetLang.strings";

open(STRINGS, "< $stringsfile")
    or die "Cannot open $stringsfile";

$missing{$lang} = 0;
$noamp{$lang} = 0;

while (<STRINGS>) {
    next unless /^(.*)=\"(.*)\"/;
    my ($msgid,$string) = ($1,$2);
    #$string =~ s/&amp;/&/;
    # We will in general pick up a few spurious lines; ignore them, they
    # won't be present in en-US
    if (exists $strings{$msgid}) {
	$strings{$msgid}{$targetLang} = $string;
    }
}

foreach my $msg (sort keys %strings) {
    next unless $msg;
  
    print "#: $msg\n";
    print "msgid \"$strings{$msg}{'en-US'}\"\n";
    if ($strings{$msg}{$targetLang}){
	print "msgstr \"$strings{$msg}{$targetLang}\"\n";
    } else {
	print "msgstr \"MISSING\"\n";
    }
    print "\n";
  
}

