#!/bin/bash

./update.pl --pot

for I in `ls *.po` ;
do
    I=`echo "$I" | cut -d . -f1`
    ./update.pl $I
done
