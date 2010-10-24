# some helpful debugging for ODT + Change Tracking
# author:   Ben Martin
# warning, this is a bit hacky, mainly to make eyeball verification 
# of the output of odt closepaths simpler
#
# page refs are to programming perl 2nd ed by ORA
#
package odtct;
use Exporter();
use Getopt::Long;
@ISA=qw(Exporter);

@EXPORT=qw($BoldOn $BoldOff $NormOn $NormOff quoteLine ColourLine $preamble $postamble);
@EXPORT_OK=qw();

$htmlmode=0;

$preamble="";
$postamble="[0m";

$NormalOn  = '[1m';
$NormalOff = '[0m';
$BoldOn  = '[1m';
$BoldOff = '[0m';

sub quoteLine($) {
    my $line = shift;
    if( $htmlmode )
    {
	$line =~ s!&!&amp;!g;
	$line =~ s!<!&lt;!g;
	$line =~ s!>!&gt;!g;
   }
    return $line;
}


sub ColourOn($) {
    my $id = shift;
    return $colourOnMap{$id};
}
sub ColourOff($) {
    my $id = shift;
    return $colourOffMap{$id};
}
sub ShiftCTID($) {
    my $id = shift;
    $id = $id + 10;
    return $id;
}
sub ColourLine($) {
    my $line = shift;
    $line =~ s!(change-idref=")([^"]*)"!&ColourOn($2) . $1 . $2 . '"' . &ColourOff($2) !e;
    $line =~ s!(ctid-)([0-9]+)!&ColourOn(ShiftCTID($2)) . $1 . $2 . &ColourOff(ShiftCTID($2)) !e;
    return $line;
}

sub setup() {

    # pp 446 getopts    
    GetOptions("output-html" => \$htmlmode);
    
    if( $htmlmode ) {
	print "html mode, setting vars\n";

	$preamble = "<pre>";
	$postamble = "</pre>";
	$NormalOn  = "";
	$NormalOff = "";
	$BoldOn    = "<b>";
	$BoldOff   = "</b>";

	%colourOnMap = (
	    "1" => "<font color='#440000'>",
	    "2" => "<font color='#004400'>",
	    "3" => "<font color='#000044'>",
	    "4" => "<font color='#444444'>",
	    "11" => "<font color='#448888'>",
	    "12" => "<font color='#4488AA'>",
	    "13" => "<font color='#44AA88'>",
	    "14" => "<font color='#44AAAA'>",
	    );

	foreach $k (keys %colourOnMap) {
	    $colourOffMap{$k} = "</font>";
	}
    }
    else
    {
	foreach $i ( 1..9 ) {
	    $v = 30 + $i;
	    $colourOnMap{$i}  = "[${v}m";
	}
	foreach $i ( 1..9 ) {
	    $v = 40 + $i;
	    $colourOnMap{$i+10}  = "[${v}m";
	}
	foreach $k (keys %colourOnMap) {
	    $colourOffMap{$k} = "[0m";
	}
    }
}


