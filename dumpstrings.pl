#!/usr/bin/perl
#
# This program analyzes the strings and dumps out a HTML
# page with information about those strings.
#
use CGI qw/-no_debug :standard/;

sub PrintTime {
    my($RetTime, $StartTimeArg, $sec, $minute, $hour, $mday, $mon, $year);
    
    ($sec,$minute,$hour,$mday,$mon,$year) = localtime();
    $mon++; # month is 0 based.
    
    printf("%04d-%02d-%02d %02d:%02d:00", 1900+$year,$mon,$mday,$hour,$minute);
}

my $lang = 'en-US';
$missing{$lang} = 0;

foreach my $file (qw(./src/wp/ap/xp/ap_String_Id.h ./src/af/xap/xp/xap_String_Id.h)) {
  open(STRINGS, "< $file" )
    or die "Cannot open $file";
  
  while (<STRINGS>) {
    next unless /^\s*dcl\s*\((\w+)\s*,\s*\"(.*)\"/;
    my ($dlg,$string) = ($1,$2);
    $dlgs{$dlg}{$lang} = $string;
  }
  
  close(STRINGS);
}

## Read in each of the other language files 
## and process them apropriatly
$stringsdir = "./po";
my @lang;
if(scalar @ARGV) {
  @lang = @ARGV;
} else {
  opendir(DIR, $stringsdir) || die "can't opendir $stringsdir: $!";
  @lang = grep { s/\.strings//  } readdir(DIR);
  closedir DIR;
}

foreach my $lang (@lang) {
  open(STRINGS, "< $stringsdir/$lang.strings") 
    or die "Cannot open $stringsdir/$lang.strings";

  $missing{$lang} = 0;

  while (<STRINGS>) {
    next unless /^(\w*)=\"(.*)\"/;
    my ($dlg,$string) = ($1,$2);
    $dlgs{$dlg}{$lang} = $string;
  }
}

@lang = sort @lang;
unshift(@lang, 'en-US');

## Determine global missing counts
print STDERR "Missing strings:\n";
$selected_langs=join ":",@ARGV;
foreach my $dlg (keys %dlgs) {
  foreach my $lang (@lang) {
    if ($dlgs{$dlg}{"en-US"} && not $dlgs{$dlg}{$lang}) {
      print STDERR "$lang: $dlg\n" if $selected_langs =~ $lang;
      $missing{$lang}++; 
    }
  }
}

## Overall Percent Complete
my $dlg_count = keys %dlgs;
foreach my $lang (@lang) {
  warn("$lang: $missing{$lang}\n");
  my $percent = sprintf("%3d%", 100 - ($missing{$lang} / $dlg_count) * 100);
  push ( @td, td( [ b( ($lang =~ "en-US" ? $lang : a({href=>"http://www.abisource.com/dev/strings/".$lang.".po"},$lang))) , $percent == 100 ? b($percent) : $percent ]),"\n");
}
print
  table({ border => 1, cellspacing => 0 }, Tr( [ th(['Language', 'Status']), @td ] )),"\n";

print "<br/>\n";
print "Last generated at<br/>";
&PrintTime;
print "\n";
