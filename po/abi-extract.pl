#!/usr/bin/perl -w

#  Copyright (C) 2000 Free Software Foundation.
#
#  This script is free software; you can redistribute it and/or
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

#use strict;
$| = 1;
my $ARG1 = $ARGV[0];

# Use the supplied arguments
#---------------------------
if ($ARGV[0]=~/^-(.)*/){
   if ("$ARGV[0]" eq "--no-pot-update" 
    || "$ARGV[0]" eq "-N"){ $ARG1=$ARGV[1];   &Extract; &UpdateQuick;  exit; } 
}  else {                                     &Extract; &UpdateNormal; exit; }

sub Extract{

if (! -s "../user/wp/strings/$ARG1.strings") {
  print "Error: The is no stringfile for $ARG1\n"
       ."Script cannot proceed!\n";
  exit;
}

if  (! -s "$ARG1.po") { system("touch $ARG1.po"); }

foreach 
my $file ("../src/wp/ap/xp/ap_String_Id.h", 
          "../src/af/xap/xp/xap_String_Id.h") {

  open IN, "<$file" || die "Cannot open $file";
  
  while (<IN>) {
     next unless /\((.*)\s*,\s*\"(.*)\"/;
     my ($msgid, $string) = ($1, $2);
     $strings{$msgid}{"en-US"} = $string;
  }
  close IN;
}

$file = "../user/wp/strings/$ARG1.strings";

open IN, "<$file" || die "Cannot open $file";

while (<IN>) {
    next unless /^(.*)=\"(.*)\"/;
    my ($msgid, $string) = ($1, $2);

    # XML to ASCII
    $string =~ s/&amp;/&/g;
    $string =~ s/(&#10;|&#x000a;)/\\n\"\n\"/g;
    $string =~ s/(&#9;|&#x0009;)/\\t/g;
    $string =~ s/(&lt;)/</g;
    $string =~ s/(&gt;)/>/g;    

    if (exists $strings{$msgid}) {
	$strings{$msgid}{$ARG1} = $string;
    }
}

$file = "$ARG1-tmp.po";
open OUT, ">$file" || die "Cannot open $file";

foreach my $msg (sort keys %strings) {
    next unless $msg;
  
    print OUT "msgid \"$strings{$msg}{\"en-US\"}\"\n";
    if ($strings{$msg}{$ARG1}){
	print OUT "msgstr \"$strings{$msg}{$ARG1}\"\n";
    } else {
	print OUT "msgstr \"\"\n";
    }
    print OUT "\n";
}

close OUT;

print "Adding [$ARG1] lines from strings file";
system("touch tmpmerge.po");
system("msgcomm --more-than=0 --omit-header --add-location --output=tmp.po tmpmerge.po $file");
system("msgmerge tmp.po $ARG1.po -o $ARG1.po");
system("rm tmp.po");
system("rm tmpmerge.po");
system("rm $file");
}

sub UpdateNormal{
   system("./update.pl $ARG1");
   print "\n";
}

sub UpdateQuick{
   print "Merging $ARG1.po with abiword.pot";
   system("./update.pl --dist $ARG1");
   
   print "\n";
} 
