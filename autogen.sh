#!/bin/sh
# 
# Run this before configure
#
# This file blatantly ripped off from subversion.
#
# Note: this dependency on Perl is fine: only SVN developers use autogen.sh
#       and we can state that dev people need Perl on their machine
#

rm -f autogen.err

automake --version | perl -ne 'if (/\(GNU automake\) ([0-9].[0-9])/) {print;  if ($1 < 1.4) {exit 1;}}'

if [ $? -ne 0 ]; then
    echo "Error: you need automake 1.4 or later.  Please upgrade."
    exit 1
fi

if test ! -d `aclocal --print-ac-dir 2>> autogen.err`; then
  echo "Bad aclocal (automake) installation"
  exit 1
fi

for script in `cd ac-helpers/fallback; echo *.m4`; do
  if test -r `aclocal --print-ac-dir 2>> autogen.err`/$script; then
    # Perhaps it was installed recently
    rm -f ac-helpers/$script
  else
    # Use the fallback script
    cp ac-helpers/fallback/$script ac-helpers/$script
  fi
done

# Produce aclocal.m4, so autoconf gets the automake macros it needs
# 
echo "Creating aclocal.m4: aclocal -I ac-helpers $ACLOCAL_FLAGS"

aclocal -I ac-helpers $ACLOCAL_FLAGS 2>> autogen.err

echo "Checking for PKG_CHECK_MODULES..."

pkgcheckdef=`grep PKG_CHECK_MODULES aclocal.m4 | grep AC_DEFUN`
if test "x$pkgcheckdef" = "x"; then
  echo ""
  echo "error: PKG_CHECK_MODULES isn't defined"
  echo ""
  echo "   Either pkg.m4 wasn't in aclocal's search path or pkgconfig"
  echo "   (or pkgconfig-devel?) isn't installed."
  echo ""
  echo "   If pkg-config is installed in <prefix> then re-run autogen.sh:"
  echo ""
  echo "       ACLOCAL_FLAGS=\"-I <prefix>/share/aclocal\" ./autogen.sh"
  echo ""
  exit
fi

# Produce all the `GNUmakefile.in's and create neat missing things
# like `install-sh', etc.
# 
echo "automake --add-missing --copy --foreign"

automake --add-missing --copy --foreign 2>> autogen.err || {
    echo ""
    echo "* * * warning: possible errors while running automake - check autogen.err"
    echo ""
}

# If there's a config.cache file, we may need to delete it.  
# If we have an existing configure script, save a copy for comparison.
if [ -f config.cache ] && [ -f configure ]; then
  cp configure configure.$$.tmp
fi

# Produce ./configure
# 
echo "Creating configure..."

autoconf 2>> autogen.err || {
    echo ""
    echo "* * * warning: possible errors while running automake - check autogen.err"
    echo ""
}

echo ""
echo "You can run ./configure now."
echo ""

