#!/bin/sh
# 
# Run this before configure
#
# This file blatantly ripped off from subversion.
#
# Note: this dependency on Perl is fine: only SVN developers use autogen.sh
#       and we can state that dev people need Perl on their machine
#

rm -rf autom4te.cache
rm -f autogen.err

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

echo "+ Entering source dir $srcdir ..."
olddir=`pwd`
cd $srcdir

# find plugins Makefile templates
find plugins -name Makefile.am | sed  's|.am$||g' > plugin-makefiles.m4

# create plugin list
(cd plugins && find . -maxdepth 1 -type d | grep -v '^\.$' | grep -v '\./\.' | sed 's|\./||g' | xargs echo) > plugin-list.m4

# create plugin configuration
find plugins -name plugin.m4 | xargs cat > plugin-configure.m4

automake --version | perl -ne 'if (/\(GNU automake\) (([0-9]+).([0-9]+))/) {print; if ($2 < 1 || ($2 == 1 && $3 < 4)) {exit 1;}}'
if [ $? -ne 0 ]; then
    echo "* * * error: you need automake 1.4 or later.  Please upgrade."
    exit 1
fi

# find extra m4 files provided by plugins and symlink them
for f in ` find ./plugins -name '*.m4' | grep -v 'plugin\.m4'`; do
    ln -sf $f
done

# Produce aclocal.m4, so autoconf gets the automake macros it needs
ACLOCAL_FLAGS="$ACLOCAL_FLAGS -I ."
echo "+ Creating aclocal.m4: aclocal $ACLOCAL_FLAGS"
aclocal $ACLOCAL_FLAGS 2>> autogen.err || {
    echo ""
    echo "* warning: possible errors while running aclocal - check autogen.err"
    echo ""
}

echo "+ Running libtoolize --force --copy"
libtoolize --force --copy || {
    echo "* * * error: libtoolize failed"
    exit 1
}

# do not run autoheader on win32, reported to hang
uname | grep -i '^mingw32'
if [ $? -eq 0 ]; then
    echo "+ Win32 detected, skipping autoheader"
else
    echo "+ Running autoheader ..."
    autoheader configure.in 2>> autogen.err || {
        echo ""
        echo "* warning: possible errors while running automake - check autogen.err"
        echo ""
    }
fi

# Produce all the `Makefile.in's and create neat missing things
# like `install-sh', etc.
echo "+ Running automake ..."

automake --add-missing --copy 2>> autogen.err || {
    echo ""
    echo "* warning: possible errors while running automake - check autogen.err"
    echo ""
}

echo "+ Creating configure..."

autoconf 2>> autogen.err || {
    echo ""
    echo "* warning: possible errors while running automake - check autogen.err"
    echo ""
}

echo "+ Leaving source dir $srcdir ..."
cd $olddir

conf_flags="--enable-maintainer-mode"

if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile. || exit 1
else
  echo Skipping configure process.
fi
