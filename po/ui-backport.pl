#!/usr/bin/perl -w

#
#  The XP UI Translation Backporter
#
#  Copyright (C) 2000 Free Software Foundation.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2 of the
#  License, or (at your option) any later version.
#
#  This script is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this library; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#  Authors: Kenneth Christiansen <kenneth@gnu.org>
#

use strict;

# Declare global variables
#-------------------------
my $VERSION 	= "0.5";

my $lang 	= "$ARGV[0]";
my $kind 	= "0";
my @xap_strings;
my @ap_strings;
my $tag;
my $str;
my $cont 	= "0";

$lang =~ s/_/-/g;

print "Porting the PO translation back to AbiWord XP format\n";

if (! -s "$lang.po") { print "Error: file $lang.po does not exist!\n"; exit; }

open FILE, ">../user/wp/strings/$lang.strings";

print FILE
"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n".
"<!-- ==============================================================  -->\n".
"<!-- This file contains AbiWord Strings.  AbiWord is an Open Source  -->\n".
"<!-- word processor developed by AbiSource, Inc.  Information about  -->\n".
"<!-- this application can be found at http://www.abisource.com       -->\n".
"<!-- This file contains the string translations for one language.    -->\n".
"<!-- This file is covered by the GNU Public License (GPL).           -->\n".
"<!-- ==============================================================  -->\n\n".
"<AbiStrings app=\"AbiWord Personal\" ver=\"1.0\" language=\"$lang\">\n\n";

open (IN, "<$lang.po") || die "can't open $lang.po: $!";

while (<IN>) {
    last if (/Content-Transfer-Encoding/);
}

while (<IN>) {
    if ($cont == "1") { 
       if (m@^\"(.*)\"@)  { $str .= $1; next; } else 
       { 
          if ($kind == 1) { push @xap_strings, "$tag=\"$str\""; $cont=0; next; }
          if ($kind == 2) { push @ap_strings,  "$tag=\"$str\""; $cont=0; next; }
       }
    }

    if (m@^#\.\s(\w+)@)            { $tag  = $1; next; } 
    if (m@^#:\s(.*)/xap[^/.]+\.h@) { $kind =  1; next; } 
    if (m@^#:\s(.*)/ap[^/.]+\.h@)  { $kind =  2; next; }
    if (m@^msgstr\s\"(.*)\"@)      { $str  = $1; $cont=1; next; }
    
}

# Add the tags and the strings to the file
#-----------------------------------------

@xap_strings = sort (@xap_strings);
@ap_strings = sort (@ap_strings);

print FILE "<Strings    class=\"XAP\"\n";
for (my $n = 0; $n < @xap_strings; $n++) {
    $xap_strings[$n] =~ s/&/&amp;/mg;
    $xap_strings[$n] =~ s/\\n/&#x000a;/mg;
    $xap_strings[$n] =~ s/\\"/&quot;/mg;
    $xap_strings[$n] =~ s/</&lt;/mg;
    $xap_strings[$n] =~ s/>/&gt;/mg;
    print FILE "$xap_strings[$n]\n";
}   print FILE "/>\n\n";

print FILE "<Strings    class=\"AP\"\n";
for (my $n = 0; $n < @ap_strings; $n++) {
    $ap_strings[$n] =~ s/&/&amp;/mg;
    $ap_strings[$n] =~ s/\\n/&#x000a;/mg;
    $ap_strings[$n] =~ s/\\"/&quot;/mg;
    $ap_strings[$n] =~ s/</&lt;/mg;
    $ap_strings[$n] =~ s/>/&gt;/mg;
    print FILE "$ap_strings[$n]\n";
}   print FILE "/>\n\n";

print FILE "</AbiStrings>\n\n";

print "Wrote ../user/wp/strings/$lang.strings\n";
