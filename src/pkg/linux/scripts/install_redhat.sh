#!/bin/sh
#
# This script is invoked by the Makefiles when the "install_redhat" target
# is specified.  It puts things where Red Hat systems normally store binaries.
# be happy, in $TARGET, with symbolic links (in $BINDIR) to the binaries
# in $TARGET/bin
# 
# $SRCDIR should be the place where the compile process (the "canonical"
# target especially) put all the stuff.  $SCRIPTDIR should be set by the 
# caller so we can know where we are.  $BINDIR is where the symbolic
# links are eventually installed, to point to the "bin/*" files within
# $TARGET/bin

# if no target, bail
if [ -z "$TARGET" ]
then
    echo ""
    echo "Error: Installation target (\$TARGET) not specified."
    echo ""
    exit 1
fi
if [ -z "$BINDIR" ]
then
    echo ""
    echo "Error: Binary directory (\$BINDIR) not specified."
    echo ""
    exit 1
fi
if [ -z "$SRCDIR" ]
then
    echo ""
    echo "Error: Compile output directory (\$SRCDIR) not specified."
    echo ""
    exit 1
fi
if [ -z "$SCRIPTDIR" ]
then
    echo ""
    echo "Error: Script location directory (\$SCRIPTDIR) not specified."
    echo ""
    exit 1
fi

# Make sure the target dir exists
mkdir -p $TARGET

if [ ! -d $TARGET ]
then
    echo ""
    echo "Error creating directory [$TARGET]."
    echo ""
    exit 1
fi

# start with breathing room
echo ""

########################################################################
# Install program data and binaies
########################################################################

echo "Installing program data files to [$TARGET]..."
# Copy the files from $SRCDIR to $TARGET
(cd $SRCDIR/AbiSuite; tar cf - *) | (cd $TARGET; tar xf -)

mkdir -p $TARGET/bin

echo "Installing program binaries to [$TARGET/bin]..."
# Setup bins (only dynamic in RPM)
(cp $SRCDIR/bin/AbiWord_d $TARGET/bin)


########################################################################
# Install menu for GNOME and other window managers
########################################################################
echo "Installing menu hook at [/usr/lib/menu/abisuite]..."
# add other program hooks here for AbiCalc, etc.

mkdir -p /usr/lib/menu

echo '?package(abisuite):needs=X11 section=Apps/Editors title="AbiWord" command="/usr/bin/X11/AbiWord"' > \
    /usr/lib/menu/abisuite

echo "Installing GNOME desktiop icon at [/usr/share/pixmaps/abiword_48.png]..."
mkdir -p /sur/share/pixmaps
cp $SRCDIR/AbiSuite/icons/abiword_48.png /usr/share/pixmaps/abiword_48.png

echo "Installing GNOME desktop hook at [/usr/share/gnome/apps/Applications/abiword.desktop]..."
mkdir -p /usr/share/gnome/apps/Applications
cp rpm/data/abiword.desktop /usr/share/gnome/apps/Applications/abiword.desktop

########################################################################
# Install documentation in standard Red Hat places
########################################################################
echo "Installing user documentation at [/usr/doc/abisuite]..."

mkdir -p /usr/doc/abisuite

cat > /usr/doc/abisuite/README << EOF
Information about AbiSuite applications and development can be
found at http://www.abisource.com/.  This information will always
be current and in-sync with the latest AbiSuite development efforts.
EOF
cp $SRCDIR/AbiSuite/readme.txt /usr/doc/abisuite/copyright
cp $SRCDIR/AbiSuite/COPYING /usr/doc/abisuite/COPYING

########################################################################
# Dynamically construct a wrapper for AbiSuite binaries
########################################################################

cd $SCRIPTDIR

echo "Making wrapper script at [$TARGET/bin/AbiWord]..."
./makewrapper.sh AbiWord $TARGET
# TODO : make use of these
# ./makewrapper.sh AbiCalc $TARGET
# ./makewrapper.sh AbiFile $TARGET


########################################################################
# Create symbolic links to the script we installed
########################################################################

echo "Creating symbolic links at [$BINDIR/AbiWord] and [$BINDIR/abiword]..."

mkdir -p $BINDIR

# NOTE : Solaris ln doesn't seem to honor the -f (force flag), so
# NOTE : we have to remove them first.
rm -f $BINDIR/AbiWord ; ln -s $TARGET/bin/AbiWord $BINDIR/AbiWord
rm -f $BINDIR/abiword ; ln -s $TARGET/bin/AbiWord $BINDIR/abiword

# TODO : make use of these, etc.
# rm -f $BINDIR/AbiCalc ; ln -s $TARGET/bin/AbiCalc $LINK_DIR/AbiCalc
# rm -f $BINDIR/abicalc ; ln -s $TARGET/bin/AbiCalc $LINK_DIR/abicalc

########################################################################
# Done
########################################################################
echo ""
echo "Installation complete and symbolic links installed."
echo ""
