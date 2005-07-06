#! /scratchbox/tools/bin/bash

for arquivo in `find . -name .cvsignore -print`
do
	cur_dir=`dirname $arquivo`
	cur_file=`basename $arquivo`
	echo -n "cleaning up $cur_dir... "
	(cd $cur_dir ; rm -rf `cat $cur_file` ; cd -)
	echo "ok."
done
