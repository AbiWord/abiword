#!/usr/bin/perl
#
# This is a simple script to run AbiWord tests.
#
# It works by running one or more test scripts, which contain
# commands fed to AbiWord via the AbiCommand plugin. Lines
# starting with # are considered comments and are ignored.
#
# Embedded in the scripts can be various commands prefixed
# with *:
#  *CMP(cmp-id)
#    Will cause the current document contents to be saved
#    to a temporary file and compared against the file
#    identified by cmp-id. See hello.cmp for an example.
#    If the files match, a PASS line is printed out.
#    If the files do not match, a FAIL line is printed out.
#
#    If the script is invoked with --regenerate, the document
#    is saved to the comparison file. This is used for bootstrapping
#    the test files.
#
#
#  Copyright (c) 2002 Jesper Skov
#  
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA.

sub compare($$)
{
    my $dst = shift;
    my $src = shift;
    open(DST, $dst) or return -1;
    open(SRC, $src) or return -1;

    my $line = 0;
    while(<DST>) {
	$line++;

	$d = $_;
	$s = <SRC>;

	if (!defined $s || $d ne $s) {
	    return $line;
	}
    }

    return 0;
}

$temp = "/tmp/abi_test";
$launched = 0;
$generate = 0;
$debug = 0;

FILE:
while(defined($_ = shift @ARGV)) {
    if (/--generate/) {
	$generate = 1;
	next;
    }

    if (/--debug/) {
	$debug = 1;
	next;
    }

    s/.cmd$//;
    $test = $_;
    open (TEST, "$test" . ".cmd") or die("could not open file ${test}.cmd\n");

    if ($launched) {
        print "**************** Closing file\n";
	close ABI;
    }

    print "Running test $test\n";

    $line = 1;
    $launched = 0;
    while(<TEST>) {
	$line++;

	# skip comment lines
	if (/^\#/) {
	    # FIXME: allow for stuff to be set via comments, such as
	    # environment variables and extra abiword options

	    next; 
	}
	
	# Launch AbiWord if it hasn't already been
	if (!$launched) {
	    $launched = 1;
	    $extras = "";
	    $extras .= " >/dev/null 2>/dev/null" if (!$debug);
	       
	    open(ABI, "|abiword --plugin AbiCommand" . $extras)
		or die("could not open abiword pipe");
	}

	if (/^\*CMP\((.*)\)/) {
	    # compare file
	    @args = split(',', $1);
	    # FIXME: strip spaces
	    # 0: compare file, including type suffix
	    if ($args[0] =~ m/.*\.([^\.]+)/) {
		$suffix = $1;
		$match = $args[0];
		
		$src = "${test}.dat.${match}";
		$dst = "${temp}.${suffix}";

		if ($generate) {
		    print ABI "save $src\n";
		    print " Generate output file $src\n";
		} else {
		    print ABI "save $dst\n";
		    $res = compare($src, $dst);
		    if ($res == 0) {
			print " PASS:<CMP line $line>\n";
		    } else {
			print " FAIL:<CMP line $line [diff on line $res]>\n";
		    }
		}
	    } else {
		die "no CMP file suffix in test $test\n";
	    }
	} elsif (/^\*/) {
	    chop; #lose the newline
	    print "Unknown command ($_) in test $test line $line\n";
	    next FILE;
	} else {
	    print ABI $_;
	}
    }
}
