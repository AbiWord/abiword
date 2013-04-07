#!/usr/bin/perl -w
#
# AbiWord
# Copyright (C) 2004 Hubert Figuiere
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301 USA.
#


use strict;


#require "context-id.pl";


my $file = shift;
my $previous_keyword = "";
#my @contexts = `cut -d':' -f 2 < $file | sort| uniq `;

my @keywordids;
my @keywords;


open (FILE, "<" . $file);

push @keywordids, "/* automatically generated, do no edit !*/";
push @keywordids, "typedef enum {";
push @keywordids, "\tRTF_UNKNOWN_KEYWORD = 0,";
push @keywordids, "\tRTF_KW_LF,";
push @keywordids, "\tRTF_KW_CR,";
push @keywordids, "\tRTF_KW_QUOTE,";
push @keywordids, "\tRTF_KW_HYPHEN,";
push @keywordids, "\tRTF_KW_STAR,";
push @keywordids, "\tRTF_KW_COLON,";
push @keywordids, "\tRTF_KW_BACKSLASH,";
push @keywordids, "\tRTF_KW_UNDERSCORE,";
push @keywordids, "\tRTF_KW_OPENCBRACE,";
push @keywordids, "\tRTF_KW_PIPE,";
push @keywordids, "\tRTF_KW_CLOSECBRACE,";
push @keywordids, "\tRTF_KW_TILDE,";


# the array is not const in C since we'll sort it out using qsort()
push @keywords, "_rtf_keyword rtfKeywords[] = {";

push @keywords, "\t{\"\\n\", false, false, NO_CONTEXT, RTF_KW_LF },";
push @keywords, "\t{\"\\r\", false, false, NO_CONTEXT, RTF_KW_CR },";
push @keywords, "\t{\"'\", false, false, NO_CONTEXT, RTF_KW_QUOTE },";
push @keywords, "\t{\"*\", false, false, NO_CONTEXT, RTF_KW_STAR },";
push @keywords, "\t{\"-\", false, false, NO_CONTEXT, RTF_KW_HYPHEN },";
push @keywords, "\t{\":\", false, false, NO_CONTEXT, RTF_KW_COLON },";

# uppercase goes here

push @keywords, "\t{\"\\\\\", false, false, NO_CONTEXT, RTF_KW_BACKSLASH },";
push @keywords, "\t{\"_\", false, false, NO_CONTEXT, RTF_KW_UNDERSCORE },";


while (<FILE>) {
    my $line = $_;
    my ($col1, $col2, $col3) = split (":", $line);
    $col1 =~ /\\ ?(clNoWrap|[a-zABDFLRTW]*)(N)? ?((7\.0|95|97|2000|2002))?/;

    my $keyword = $1;
    my $elem;   #array element for keywords
    if ($keyword ne $previous_keyword) {
	$previous_keyword = $keyword;
	my ($version, $hasversion, $hasparam);
	$hasversion = defined($3);
	if ($hasversion) {
	    $version = $3;
	}
	else {
	    $version = "";
	}
	
	$hasparam = defined($2);
	$elem = "\t{\"$keyword\", ";
	if ($hasparam) {
	    $elem .= "true, ";
	}
	else {
	    $elem .= "false, ";
	}
	$elem .= "false, NO_CONTEXT, RTF_KW_$keyword }, ";
	if ($hasversion) {
	    $elem .= "/* $version */ ";
	}
	push @keywords, $elem;
	push @keywordids, "\tRTF_KW_$keyword,";
    }
    else {
	print "Ignoring duplicate keyword $keyword.\n";
    }
}

push @keywordids, "\tRTF_KW__END__";

close FILE;

push @keywordids, "} RTF_KEYWORD_ID;";

push @keywords, "\t{\"{\", false, false, NO_CONTEXT, RTF_KW_OPENCBRACE },";
push @keywords, "\t{\"|\", false, false, NO_CONTEXT, RTF_KW_PIPE },";
push @keywords, "\t{\"}\", false, false, NO_CONTEXT, RTF_KW_CLOSECBRACE },";
push @keywords, "\t{\"~\", false, false, NO_CONTEXT, RTF_KW_TILDE }";
push @keywords, "};";



my $i;

open (OUTID, ">ie_imp_RTFKeywordIDs.h");
$i = 0;
for ($i = 0; $i < scalar @keywordids; $i++) {
    print OUTID "$keywordids[$i]\n";
}
close OUTID;

open (OUT, ">ie_imp_RTFKeywords.h");
$i = 0;
for ($i = 0; $i < scalar @keywords; $i++) {
    print OUT "$keywords[$i]\n";
}
close OUT;
