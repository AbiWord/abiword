#!/bin/sh
# remove the static binary and strip the dynamic one
ls -Flags $prefix/AbiSuite/bin
strip $prefix/AbiSuite/bin/AbiWord_d
rm -f $prefix/AbiSuite/bin/AbiWord_s

# check and list the depend package
echo "@cwd $prefix" > $OUTFILE
SHPWD=`pwd`
cd /var/db/pkg
if [ `/bin/ls -d gtk-* | wc -l` -eq 1 ]; then
	echo @pkgdep `/bin/ls -d gtk-*` >> $OUTFILE
else
	cat $SHPWD/scripts/fbsdwarn
fi
if [ `/bin/ls -d png-* | wc -l` -eq 1 ]; then
	echo @pkgdep `/bin/ls -d png-*` >> $OUTFILE
else
	sed 's/gtk/png/g' $SHPWD/scripts/fbsdwarn
fi

# list the files in $TARGET(/usr/local/AbiSuite)
cd $TARGET
find . -type f | sort | sed 's/^./AbiSuite/g' >> $OUTFILE

# list the symbolic link files in $BINDIR(/usr/local/bin)
cd $BINDIR
find . -name "[Aa]bi[Ww]ord*" | sort | sed 's/^./bin/g' >> $OUTFILE

# list the directories ino $TARGET(/usr/local/AbiSuite)
# They will be remove if you remove AbiSuite package.
cd $TARGET
find . -type d | sort -r | sed 's/^./@dirrm AbiSuite/g' >> $OUTFILE
