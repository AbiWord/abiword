#!/bin/sh

if [ $(( ($# < 1) || ($# > 2) )) = 1 ]
then
	echo ""
	echo "---------- AbiWord ttfadmin tool ------------------------"
	echo "This script generates support files required by AbiWord"
	echo "for each TrueType font in a given directory."
	echo "NB: this script must be located in the same directory as"
	echo "the program ttftool."
	echo ""
	echo "Usage: ttfadmin.sh directory [encoding]"
	echo "run 'ttftool -e print' for a list of supported encodings"
	echo "when the encoding is omitted fonts will be coded with"
	echo "Adobe StandardEncoding"
	echo "----------------------------------------------------------"
	echo ""
	exit 0
fi

if [ $(( $# == 2 )) = 1 ]
then
	ENCODING="-e $2"
fi

TTFTOOL=${0%ttfadmin.sh}ttftool

FILES=`ls -1 $1/*.ttf`
for dir in $FILES
do
	echo "Processing font $dir"
	AFM=${dir%.ttf}.afm
	UTOG=${dir%.ttf}.u2g
	T42=${dir%.ttf}.t42
	`$TTFTOOL -f $dir -a $AFM -p $T42 -u $UTOG $ENCODING`
done
