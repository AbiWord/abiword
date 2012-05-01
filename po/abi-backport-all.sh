#!/bin/bash

#
# For sole use by the web server to generate translation statistics!
#

for I in *.po;
do
    I=`echo "$I" | cut -d . -f1`
    ./ui-backport.pl $I.po $I.strings
done

echo "It is completely pointless to generate the .strings files here!"
