#!/bin/sh
# 
# Run this before configure
#
# This file blatantly ripped off from subversion.
#
# Note: this dependency on Perl is fine: only SVN developers use autogen.sh
#       and we can state that dev people need Perl on their machine
#

set -e

automake --version | perl -ne 'if (/\(GNU automake\) ([0-9].[0-9])/) {print;  if ($1 < 1.4) {exit 1;}}'

if [ $? -ne 0 ]; then
    echo "Error: you need automake 1.4 or later.  Please upgrade."
    exit 1
fi

if test ! -d `aclocal --print-ac-dir`; then
  echo "Bad aclocal (automake) installation"
  exit 1
fi

for script in `cd ac-helpers/fallback; echo *.m4`; do
  if test -r `aclocal --print-ac-dir`/$script; then
    # Perhaps it was installed recently
    rm -f ac-helpers/$script
  else
    # Use the fallback script
    cp ac-helpers/fallback/$script ac-helpers/$script
  fi
done

# Produce aclocal.m4, so autoconf gets the automake macros it needs
echo "Creating aclocal.m4..."
aclocal -I ac-helpers $ACLOCAL_FLAGS

# autoheader

# Produce all the `GNUmakefile.in's and create neat missing things
# like `install-sh', etc.
automake --add-missing --copy --foreign

# If there's a config.cache file, we may need to delete it.  
# If we have an existing configure script, save a copy for comparison.
if [ -f config.cache ] && [ -f configure ]; then
  cp configure configure.$$.tmp
fi

# Produce ./configure
echo "Creating configure..."
autoconf

echo ""
echo "You can run ./configure now."
echo ""

