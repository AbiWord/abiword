#!/usr/bin/perl
$id= ' $Id$ ';

###############################################################################
# Copyright (C) 2001  AbiWord development Team
# http://www.abisource.com
#
# Copyright (C) 2001  Gaetan RYCKEBOER - Club LinuX Nord-Pas de Calais
# http://clx.anet.fr
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
###############################################################################
##
## $Log$
## Revision 1.1  2001/03/07 16:15:35  dom
## CSS/HTML script from Gaetan RYCKEBOER
##
#
#
# TODO : I & O filename on command line. (I won't do it...)
#

#
# Differents line end markers
#
#$/="\n";
#$/="\r";
#$/="\f";

sub ShowHelp
{
    print (
"Usage: abw2html.pl [options] [filename]

AbiWord 2 HTML translator.
  Input  : (stdin)  : abiword made html file
  output ; (stdout) : html css based \"on the fly\" file.

 ex : cat abi.html | abw2html.pl > abi-css.html

Options : 
  -h, --help    : show this help
  -v, --version : show version

Report bugs to <asr\@mail.dotcom.fr>.
");
}

# ----------------------------------------------------------------------------
# Print the abw2html version on the standard ouptut.

sub ShowVersion
{
    print (
"
abw2html.pl Version 0.1
CVS :$id

abiword made html to css/html translator.
-----------------------------------------

 Copyright (C) 2001  
 Gaetan RYCKEBOER   -  Club LinuX Nord-Pas de Calais
 asr@mail.dotcom.fr -  http://clx.anet.fr

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

");
}


# command line handling
foreach $i ( @ARGV )
{
    #
    # Show help message.
    #
    if ( $i =~ /^-h/ || $i =~ /^--help/ )
    {
        &ShowHelp();
        exit(0);
    }
    #
    # Show version
    if ( $i =~ /^-v/ || $i =~ /^--version/ )
    {
        &ShowVersion();
        exit(0);
    }
    # etc.
    # put here code to open file
}

@styles=();
$stylenum=0;
$head=0;

foreach $inp (<STDIN>) {
    if ($head!=1){
	if ($inp =~ /(.*<body>)(.*)/) {
	    $head=1;
	    print $1;
	    $inp=$2;
	} else {
	    print "$inp";
	}
    }
    if ($head==1) {
	$line="";
	while ($inp =~ /^([^<]*)<([^>]*)>(.*)$/) {
	    $beg=$1; 
	    $end=$3;
	    $inp="$end";
	    $tag=$2;
	    if ($tag =~ /span style="(.*)"/) {
		$style=$snum{"$1"};
		if ($style=='') {
		    $styles{++$stylenum}=$1;
		    $style=$stylenum;
		    $snum{$1}=$style;
		}
		$line.="$beg<span class=\"abi-sty$style\">";
		
	    } elsif ($tag =~ /\/span/) {
		$line.="$beg</span>";
	    } else {
		$line.="$beg<$tag>";
	    }
	}
	$lines.="$line\n";
    }
}
    
print ("
<style type=\"text/css\">
<!-- 
 P { margin-top: 0pt; margin-bottom: 0pt } 
");

foreach (keys (%styles)) {
    print " .abi-sty$_ { $styles{$_} }\n";
}

print ("
 -->
</style>
");

print $lines;
