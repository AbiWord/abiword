#!/bin/sh
echo "@cwd $prefix" > $OUTFILE
echo "@pkgdep `/bin/ls /var/db/pkg | grep gtk-`" >> $OUTFILE
echo "@pkgdep `/bin/ls /var/db/pkg | grep png-`" >> $OUTFILE
echo "@pkgdep `/bin/ls /var/db/pkg | grep ispell-`" >> $OUTFILE
cd $TARGET
find . -type f | sort | sed 's/^./AbiSuite/g' >> $OUTFILE
cd $BINDIR
find . -name "[Aa]bi[Ww]ord*" | sort | sed 's/^./bin/g' >> $OUTFILE
cd $TARGET
find . -type d | sort -r | sed 's/^./@dirrm AbiSuite/g' >> $OUTFILE
