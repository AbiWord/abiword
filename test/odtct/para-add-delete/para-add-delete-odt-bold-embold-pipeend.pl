#!/usr/bin/perl

#
# A little filter to trim out parts and highlight others 
# to help with verification of ODT+CT files.
#
# Usage:
#    ./para-add-delete-odt-bold-embold-pipeend.pl < /tmp/odt/o/content.xml
#
use IO::All;

my $io = io("| xmllint --format - |");

$norm = '[0m';
$bold = '[1m';

while( $line = $io->getline )
{
#    $line =~ s/body/BODY/g;
    $line =~ s!<delta:(removed-content[^>]*>)!<delta:$bold$1$norm!g;
    $line =~ s!</delta:(removed-content>)!</delta:$bold$1$norm!g;
    $line =~ s!(change-idref="[^"]*")!$bold$1$norm!g;
    $line =~ s!(insert-with-content)!$bold$1$norm!g;


    next if $line =~ /office:document-content/;
    print $line;
}
