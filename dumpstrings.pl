#!/usr/bin/perl
#
# This program analyzes the strings directory and dumps out a HTML
# page with information about those strings.
#
use CGI qw/-no_debug :standard/;

sub PrintTime {
    my($RetTime, $StartTimeArg, $sec, $minute, $hour, $mday, $mon, $year);
    
    ($sec,$minute,$hour,$mday,$mon,$year) = localtime();
    $mon++; # month is 0 based.
    
    printf("%02d/%02d/%02d %02d:%02d:00", $mon,$mday,1900+$year,$hour,$minute );
}

## en-US is in a different file in a different format
my $lang = 'en-US';
foreach my $file (qw(./src/wp/ap/xp/ap_String_Id.h ./src/af/xap/xp/xap_String_Id.h)) {
  open(STRINGS, "< $file" )
    or die "Cannot open $file";
  
  while (<STRINGS>) {
    next unless /^\s*dcl\((.*)\s*,\s*\"(.*)\"/;
    my ($dlg,$string) = ($1,$2);
    $string =~ s/&amp/&/;
    $dlgs{$dlg}{$lang} = $string;
    $longest{$dlg} = $lang
  }
  close(STRINGS);
}

## Read in each of the other language files 
## and process them apropriatly
$stringsdir = "./user/wp/strings";
my @lang;
if(scalar @ARGV) {
  @lang = @ARGV;
} else {
  opendir(DIR, $stringsdir) || die "can't opendir $stringsdir: $!";
  @lang = grep { s/\.strings//  } readdir(DIR);
  closedir DIR;
}

#$stringsdir = "./user/wp/strings";
#opendir(DIR, $stringsdir) || die "can't opendir $stringsdir: $!";
#my @lang = grep { s/\.strings//  } readdir(DIR);
#closedir DIR;

 foreach my $lang (@lang) {
  open(STRINGS, "< $stringsdir/$lang.strings") 
    or die "Cannot open $stringsdir/$lang.strings";

  $missing{$lang} = 0;
  $noamp{$lang} = 0;

  while (<STRINGS>) {
    next unless /^(\w*)=\"(.*)\"/;
    my ($dlg,$string) = ($1,$2);
    $string =~ s/&amp;/&/;
    $dlgs{$dlg}{$lang} = $string;

    ## 
    $noamp{$lang}++ unless ($string =~ /\&amp/);
    next unless $string;
    
    ## Set the longest language for a DLG the actual
    ## string for the language can be looked up vi $dlgs
    ## later.
    if ($longest{$dlg}) {
      $longest{$dlg} = $lang
	if length($dlgs{$dlg}{$lang}) > length($dlgs{$dlg}{$longest{$dlg}});
    }
    else {
      $longest{$dlg} = $lang;
    }
  }
}

@lang = sort @lang;
## Add US into the list
unshift(@lang, 'en-US');

## Determine global missing counts
print STDERR "Missing strings:\n";
$selected_langs=join ":",@ARGV;
foreach my $dlg (keys %dlgs) {
  foreach my $lang (@lang) {
    unless($dlgs{$dlg}{$lang}) {
      print STDERR "$lang: $dlg\n" if $selected_langs =~ $lang;
      $missing{$lang}++; 
      #print $lang . " - missing " . $dlg . "\n" if $lang =~ "en-US";
    }
  }
}

## Overall Percent Complete
my $dlg_count = keys %dlgs;
foreach my $lang (@lang) {
  warn("$lang: $missing{$lang}\n");
  my $percent = sprintf("%3d%", 100 - ($missing{$lang} / $dlg_count) * 100, $missing{$lang}, $dlg_count);
  push ( @td, td( [ b( ($lang =~ "en-US" ? $lang : a({href=>"http://www.abisource.com/dev/strings/".$lang.".po"},$lang))) , $percent ]),"\n");
}
print
  table({ border => 1, cellspacing => 0 }, Tr( [ th(['Language', 'Status']), @td ] )),"\n";

print "<br/>\n";
print "Last generated at<br/>";
&PrintTime;
print "\n";

