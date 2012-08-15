#!/bin/bash

./update.pl --pot

OPT=--stat
test "$1" = "fuzzy" && OPT=--dist 

for I in *.po;
do
    I=`echo "$I" | cut -d . -f1`
    ./update.pl $OPT $I
done

echo "Use 'abi-update-all.sh fuzzy' for fuzzy matching of msgmerge."
