#!/bin/sh
#
#  AbiSource Unix Installer Program
#  Copyright (C) 1999-2000 AbiSource, Inc.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

INSTALL_DATA_FILE=`pwd`/data.tar
DEFAULT_PREFIX=/usr/local
DEFAULT_HOME=AbiSuite
DEFAULT_LINK_DIR=/usr/local/bin

# Test for SysV echo; NECHO is defined for "no newline" echos
if [ "`echo 'echo\c'`" = "echo\c" ]
then
    NECHO="echo -n"
    POSTNECHO=""
else
    NECHO="echo"
    POSTNECHO="\c"
fi
ECHO="echo"

# Program execution
cat <<EOF

  AbiSuite Program Installer, Copyright (C) 1999 AbiSource, Inc.

   This program comes with ABSOLUTELY NO WARRANTY; this software is
   free software, and you are welcome to redistribute it under
   certain conditions.  Read the file called COPYING in the archive 
   in which this program arrived for more details.
EOF

# Make sure the data file is really here
if [ ! -f $INSTALL_DATA_FILE ]
then
    $ECHO ""
    $ECHO "Fatal error: can't find the file [$INSTALL_DATA_FILE]"
    $ECHO "which contains AbiSuite program and data files."
    $ECHO ""
    exit 1
fi

# Read prefix from user
cat <<EOF

  Please specify the directory into which you would like to install AbiSuite.
  The default directory is [$DEFAULT_PREFIX/$DEFAULT_HOME], 
  but you may provide an alternate path if you wish.  Hit "Enter" to use
  the default value.
EOF

GO=1
while test $GO -eq 1
do
    $ECHO ""
    $NECHO "Installation path for AbiSuite software [$DEFAULT_PREFIX/$DEFAULT_HOME]: $POSTNECHO"
    read INSTALL_BASE

    # did they use tildes for home dirs?
    if [ ! -z "`echo $INSTALL_BASE | grep '~'`" ]
    then
	LINK_DIR=`echo $INSTALL_BASE | sed "s:~:$HOME:"`
    fi

    # did they just hit enter?
    if [ -z "$INSTALL_BASE" ]
    then
	INSTALL_BASE="$DEFAULT_PREFIX/$DEFAULT_HOME"
    fi

    # GO off, passed tests
    GO=0
    if [ ! -d $INSTALL_BASE ]
    then
	$ECHO ""
	$NECHO "The directory [$INSTALL_BASE] does not exist; would you like to create it now? (y/n) [y]: $POSTNECHO"
	read CREATE_DIR
	if [ "$CREATE_DIR" = "n" -o "$CREATE_DIR" = "N" ]
	then
	    GO=1
	else
	    mkdir -p $INSTALL_BASE
	    if [ $? -ne 0 ]
	    then
		$ECHO ""
		$ECHO "I couldn't create [$INSTALL_BASE].  You are probably seeing this error"
 		$ECHO "because you do not have sufficient permission to perform this operation."
		$ECHO "You will most likely have to run this script as superuser to write to"
		$ECHO "system directories."
		# loop around again
		GO=1
	    fi
	fi
    else
	$ECHO ""
	$ECHO "  I found an existing directory called [$INSTALL_BASE].  You can choose"
	$ECHO "  to install into this directory, but existing files will be modified or"
	$ECHO "  replaced.  You can also choose not to install into this directory, and"
	$ECHO "  you will be prompted for another."
	$ECHO ""
	$NECHO "Do you want to install into [$INSTALL_BASE]? (y/n) [y]: $POSTNECHO"
	read INSTALL_OVER
	if [ "$INSTALL_OVER" = "n" -o "$INSTALL_OVER" = "N" ]
	then
	    GO=1
	fi
    fi
done

########################################################################
# Blow up our data file
########################################################################

$ECHO ""
$ECHO "Installing AbiSuite software in [$INSTALL_BASE]..."
cd $INSTALL_BASE
tar xf $INSTALL_DATA_FILE

if [ $? -ne 0 ] 
then 
    $ECHO "" 
    $ECHO "  Oops, tar seems to be having some trouble."
	$ECHO "  Refer to the errors above for more details." 
    $ECHO "  Installation aborted." 
    $ECHO "" 
    exit 1 
fi

########################################################################
# If we're on Solaris, do the PostScript resource thing.  This script
# and the main install script share this code... change one, change the
# other (please).
########################################################################

OS_NAME=`uname -s`
OS_RELEASE_MAJOR=`uname -r | sed -e "s/\..*//"`

if [ "$OS_NAME" = "SunOS" -a "$OS_RELEASE_MAJOR" = "5" ]
then
    if [ -d $INSTALL_BASE/fonts ]
    then
	$ECHO ""
	$ECHO "Building PostScript font resource database for installed fonts..."
	cd $INSTALL_BASE/fonts
	/usr/openwin/bin/makepsres
    fi
fi
cd $INSTALL_BASE

########################################################################
# Dynamically construct a wrapper for AbiSuite binaries
########################################################################

# do AbiWord

cat >$INSTALL_BASE/bin/AbiWord<<EOF
#!/bin/sh

# AbiWord wrapper script.

# Change this if you move the AbiSuite tree.
ABISUITE_HOME=$INSTALL_BASE
export ABISUITE_HOME

# Change this if you move your fonts
ABISUITE_FONT_HOME=\$ABISUITE_HOME/fonts

# Set run-time font path
if [ -d \$ABISUITE_FONT_HOME ]
then
    xset fp+ \$ABISUITE_FONT_HOME 1>/dev/null 2>/dev/null
fi

# Figure out which binary to run
if [ -f \$ABISUITE_HOME/bin/AbiWord_d ]
then
    \$ABISUITE_HOME/bin/AbiWord_d "\$@"
elif [ -f \$ABISUITE_HOME/bin/AbiWord_s ]
then
    \$ABISUITE_HOME/bin/AbiWord_s "\$@"
else
    $ECHO "Error: can't find AbiWord executables:"
    $ECHO "    \$ABISUITE_HOME/bin/AbiWord_d"
    $ECHO "    -or-"
    $ECHO "    \$ABISUITE_HOME/bin/AbiWord_s"
    $ECHO ""
    exit
fi

# Set post run-time font path
if [ -d \$ABISUITE_FONT_HOME ]
then
    xset fp- \$ABISUITE_FONT_HOME 1>/dev/null 2>/dev/null
fi
EOF

chmod 755 $INSTALL_BASE/bin/AbiWord

########################################################################
# Ask the user if he would like to set up symlinks to the script
# we just created.
########################################################################

cat <<EOF

  AbiSuite programs are now installed:
      $INSTALL_BASE/bin/AbiWord

  As a convenience, I can install symbolic links to the installed
  executables so you and other users do not have to modify
  your paths to execute the AbiSuite programs.  For example, if you
  proceed and specify "$DEFAULT_LINK_DIR", I will create symbolic links like
  "$DEFAULT_LINK_DIR/AbiWord" and point them to the executable I just
  previously installed.
EOF

# go for symlinks
GO=1
ASKED_LINKS=0
while test $GO -eq 1
do
    if [ $ASKED_LINKS -eq 0 ]
    then
	ASKED_LINKS=1
	$ECHO ""
	$NECHO "Do you want to provide a directory for these symbolic links? (y/n) [y]: $POSTNECHO"
	read MAKE_LINKS
	if [ "$MAKE_LINKS" = "n" -o "$MAKE_LINKS" = "N" ]
	then
	    $ECHO ""
	    $ECHO "Installation complete."
	    $ECHO ""
	    exit
	fi
    fi

    $ECHO ""
    $ECHO "In which directory shall I install the symbolic links?"
    $NECHO "[$DEFAULT_LINK_DIR]: $POSTNECHO"
    read LINK_DIR

    # did they use tildes for home dirs?
    if [ ! -z "`echo $LINK_DIR | grep '~'`" ]
    then
	LINK_DIR=`echo $LINK_DIR | sed "s:~:$HOME:"`
    fi

    # did they just hit enter?
    if [ -z "$LINK_DIR" ]
    then
	LINK_DIR="$DEFAULT_LINK_DIR"
    fi

    # GO off, passed tests
    GO=0
    if [ ! -d $LINK_DIR ]
    then
	$ECHO ""
	$NECHO "The directory [$LINK_DIR] does not exist; would you like to create it now? (y/n) [y]: $POSTNECHO"
	read CREATE_DIR
	if [ "$CREATE_DIR" = "n" -o "$CREATE_DIR" = "N" ]
	then
	    GO=1
	else
	    mkdir -p $LINK_DIR
	    if [ $? -ne 0 ]
	    then
		$ECHO ""
		$ECHO "  I couldn't create [$LINK_DIR].  You are probably seeing this error"
 		$ECHO "  because you do not have sufficient permission to perform this operation."
		$ECHO "  You will most likely have to run this script as superuser to write to"
		$ECHO "  system directories."
		$ECHO ""
		$ECHO "  If you wish, you can manually create these links at a later time."
		$ECHO "  You may cancel the installation of these links by issuing a"
		$ECHO "  terminal interrupt (usually Control-C), or you can provide another"
		$ECHO "  directory now."
		# loop again
		GO=1
	    fi
	fi
    fi
done

# Do the linkage to the installed, dynamically-generated SCRIPTS,
# not the actual executable binaries.  We install two links, one with 
# proper caps and one for the strong proponents of lowercase naming.
#
# Add more pairs here for any more binaries we install.

# NOTE : Solaris ln doesn't seem to honor the -f (force flag), so
# NOTE : we have to remove them first.
rm -f $LINK_DIR/AbiWord; ln -s $INSTALL_BASE/bin/AbiWord $LINK_DIR/AbiWord
rm -f $LINK_DIR/abiword; ln -s $INSTALL_BASE/bin/AbiWord $LINK_DIR/abiword

########################################################################
# Bye!
########################################################################
$ECHO ""
$ECHO "Installation complete and symbolic links installed."
$ECHO ""
