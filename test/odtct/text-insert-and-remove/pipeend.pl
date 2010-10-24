#!/usr/bin/perl -I/usr/local/src/cvs/abiword/odf-2010-track-changes/test/odtct

#
# A little filter to trim out parts and highlight others 
# to help with verification of ODT+CT files.
#
# This script is for M2 which is 
#   insertion of paragraphs
#   removal   of paragraphs
#
# Usage:
#    ./para-add-delete-odt-bold-embold-pipeend.pl < /tmp/odt/o/content.xml
# For HTML:
#    ./para-add-delete-odt-bold-embold-pipeend.pl --output-html < /tmp/odt/o/content.xml
#
use IO::All;
use odtct;

my $io = io("| xmllint --format - |");

odtct::setup();
print $preamble;

while( $line = $io->getline )
{
    next if $line =~ /office:document-content/;
    $line = quoteLine( $line );

    $line =~ s!delta:(removed-content[^>]*)!delta:$BoldOn$1$BoldOff!g;
    $line =~ s!/delta:(removed-content)!/delta:$BoldOn$1$BoldOff!g;
    $line =~ s!(change-idref="[^"]*")!$BoldOn$1$BoldOff!g;
    $line =~ s!(insert-with-content)!$BoldOn$1$BoldOff!g;
    $line =~ s!(inserted-text-start delta:inserted-text-id)!$BoldOn$1$BoldOff!g;
    $line =~ s!(inserted-text-end delta:inserted-text-idref)!$BoldOn$1$BoldOff!g;

    $line = ColourLine( $line );
    print $line;
}

print $postamble;
