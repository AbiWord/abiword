#!/bin/sh
# remove the static binary and strip the dynamic one
ls -Flags $prefix/AbiSuite/bin
strip $prefix/AbiSuite/bin/AbiWord_d
rm -f $prefix/AbiSuite/bin/AbiWord_s
# list the files now
echo "@cwd $prefix" > $OUTFILE
echo "@pkgdep `/bin/ls /var/db/pkg | grep gtk-`" >> $OUTFILE
echo "@pkgdep `/bin/ls /var/db/pkg | grep png-`" >> $OUTFILE
echo "@pkgdep `/bin/ls /var/db/pkg | grep ispell-`" >> $OUTFILE
cd $TARGET
find . -type f | sort | sed 's/^./AbiSuite/g' >> $OUTFILE
cd $BINDIR
find . -name "[Aa]bi[Ww]ord*" | sort | sed 's/^./bin/g' >> $OUTFILE
# find . -name "[Aa]bi[Ww]ord" | sort | sed 's/^./bin/g' >> $OUTFILE
cd $TARGET
find . -type d | sort -r | sed 's/^./@dirrm AbiSuite/g' >> $OUTFILE
