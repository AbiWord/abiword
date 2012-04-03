#!/bin/bash

./update.pl --pot

for I in *.po;
do
    I=`echo "$I" | cut -d . -f1`
    ./update.pl --dist $I
done
