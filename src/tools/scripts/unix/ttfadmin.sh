#!/bin/sh

if [ "$#" != "1" ]
then
	echo ""
	echo "This script generates support files required by AbiWord"
	echo "for each TrueType font in the given directory."
	echo "NB: this script must be located in the same directory as"
	echo "the programs ttf2uafm and ttf2t42."
	echo ""
	echo "Usage: ttfadmin.sh directory"
	return 0
fi

TTF2UAFM=${0%ttfadmin.sh}ttf2uafm
TTF2T42=${0%ttfadmin.sh}ttf2t42

FILES=`ls -1 $1/*.ttf`
for dir in $FILES
do
	AFM=${dir%.ttf}.afm
	UTOG=${dir%.ttf}.u2g
	T42=${dir%.ttf}.t42
	`$TTF2UAFM -f $dir -a $AFM -u $UTOG`
	`$TTF2T42 -t $dir -p $T42`
done
