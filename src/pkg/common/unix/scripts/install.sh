#!/bin/sh
#
# This script is invoked by the Makefiles when the "install" target
# is specified.  It just puts things where most users will probably
# be happy, in $TARGET, with the binaries in $LIBEXECDIR, with
# symbolic links (in $BINDIR) to the binaries in $LIBEXECDIR.
# 
# $SRCDIR should be the place where the compile process (the "canonical"
# target especially) put all the stuff.  $SCRIPTDIR should be set by the 
# caller so we can know where we are.  $BINDIR is where the symbolic
# links are eventually installed, to point to the executable files within
# $LIBEXECDIR

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

# Make sure the libexec dir exists
if [ -z "$LIBEXECDIR" ]
then
    LIBEXECDIR=${TARGET}/bin
fi
mkdir -p $LIBEXECDIR

if [ ! -d $LIBEXECDIR ]
then
    echo ""
    echo "Error creating directory [$LIBEXECDIR]."
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

echo "Installing program binaries to [$LIBEXECDIR]..."
# Setup bins 
(cd $SRCDIR/bin; tar cf - *) | (cd $LIBEXECDIR; tar xf -)

########################################################################
# If we're on Solaris, run makepsres on the font path
########################################################################

OS_NAME=`uname -s`
OS_RELEASE_MAJOR=`uname -r | sed -e "s/\..*//"`

if [ "$OS_NAME" = "SunOS" -a "$OS_RELEASE_MAJOR" = "5" ]
then
    if [ -d $TARGET/fonts ]
    then
	echo "Building PostScript font resource database for installed fonts..."
	cd $TARGET/fonts
	/usr/openwin/bin/makepsres
    fi
fi

########################################################################
# Dynamically construct a wrapper for AbiSuite binaries
########################################################################

cd $SCRIPTDIR

echo "Making wrapper script at [$TARGET/bin/AbiWord]..."
./makewrapper.sh AbiWord $TARGET $LIBEXECDIR
# TODO : make use of these
# ./makewrapper.sh AbiCalc $TARGET $LIBEXECDIR
# ./makewrapper.sh AbiFile $TARGET $LIBEXECDIR


########################################################################
# Create symbolic links to the script we installed
########################################################################

echo "Creating symbolic links at [$BINDIR/AbiWord] and [$BINDIR/abiword]..."

mkdir -p $BINDIR

# NOTE : Solaris ln doesn't seem to honor the -f (force flag), so
# NOTE : we have to remove them first.
rm -f $BINDIR/AbiWord ; ln -s $LIBEXECDIR/AbiWord $BINDIR/AbiWord
rm -f $BINDIR/abiword ; ln -s $LIBEXECDIR/AbiWord $BINDIR/abiword

# TODO : make use of these, etc.
# rm -f $BINDIR/AbiCalc ; ln -s $LIBEXECDIR/AbiCalc $LINK_DIR/AbiCalc
# rm -f $BINDIR/abicalc ; ln -s $LIBEXECDIR/AbiCalc $LINK_DIR/abicalc

########################################################################
# Done
########################################################################
echo ""
echo "Installation complete and symbolic links installed."
echo ""
