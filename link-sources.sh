#!/bin/sh

### abiword tree
### The AbiWord tree is part of the repo already in the branch
# for l in `find . -name sources.list | grep -v '^./plugins'`; do 
# 	d=`dirname $l`
# 	cd=`echo $d | sed -e 's|/[[:alnum:]-]*|/\.\.|g'`
# 	if [ -f "$d/sources.base" ]; then
# 		srcd=`cat $d/sources.base`
# 	else
# 		srcd="$d"
# 	fi
# 	for f in `cat $l`; do
# 		(echo $d/$f && cd $d && ln -sf $cd/../abiword/$srcd/$f)
# 	done
# done

# plugins tree
for l in `find ./plugins -name sources.list`; do 
	d=`dirname $l`
	cd=`echo $d | sed -e 's|/[[:alnum:]-]*|/\.\.|g'`
	srcd=`cat $d/sources.base`
	for f in `cat $l`; do
		(echo $d/$f && cd $d && ln -sf $cd/../abiword-plugins/$srcd/$f)
	done
done

