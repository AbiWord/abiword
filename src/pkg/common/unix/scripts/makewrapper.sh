#!/bin/sh
#
# This script is invoked to create wrapper scripts for AbiSuite
# binaries.  One should pass it the _base_ name of the program
# to wrap (one would pass "AbiWord" to wrap "AbiWord_s" and 
# "AbiWord_d"), the install base of the tree and, optionally, the location
# of the machine-dependent binaries and scripts.

# Examples:
#     makewrapper.sh AbiWord /usr/local/AbiSuite /usr/local/libexec/AbiSuite
#     makewrapper.sh AbiCalc /usr/local/AbiSuite /usr/local/libexec/AbiSuite

PROGRAM_NAME=$1
INSTALL_BASE=$2
LIBEXECDIR=$3

# Did they supply any arguments?
if [ -z "$PROGRAM_NAME" ]
then
    echo ""
    echo "Error: first argument (base program name) not specified."
    echo ""
    exit 1
fi
if [ -z "$INSTALL_BASE" ]
then
    echo ""
    echo "Error: second argument (installation base directory) not specified."
    echo ""
    exit 1
fi
if [ -z "$LIBEXECDIR" ]
then
    LIBEXECDIR=${INSTALL_BASE}/bin
fi

# Make directory path up to program we're creating
mkdir -p $LIBEXECDIR

cat >$LIBEXECDIR/$PROGRAM_NAME<<EOF
#!/bin/sh
#
# AbiSuite program wrapper script, dynamically generated
# from abi/src/pkg/common/unix/scripts/makewrapper.sh.

currentFonts=\`xset q | grep Abi\`

# Change this if you move the AbiSuite tree.
ABISUITE_HOME=$INSTALL_BASE
export ABISUITE_HOME

# Change this if you move the AbiSuite binaries.
ABISUITE_LIBEXEC=$LIBEXECDIR

# Change this if you move your fonts.
ABISUITE_FONT_HOME=\$ABISUITE_HOME/fonts

#locale-specific dirs could be added to it.
ABISUITE_FONT_PATH=\$ABISUITE_FONT_HOME

#now try to guess locale
locale=\$LC_ALL	#it's incorrect to set this variable, but someone
		#might set it incorrectly.
if [ -z "\$locale" ]
then
    locale=\$LANG
fi

if [ ! -z "\$locale" ]
then
    #now guess encoding
    encoding=\`echo \$locale | sed -e 's/^.*\.\(.*\)\$/\1/'\`
    if [ ! -z "\$encoding" ]
    then
	addfontdir=\$ABISUITE_FONT_HOME/\$encoding
	if [ ! -z "\$addfontdir" ]
	then
	    if [ -d \$addfontdir ]
	    then
	    	#add directory with locale-specific fonts to font path
	    	ABISUITE_FONT_PATH=\$ABISUITE_FONT_PATH,\$addfontdir
	    fi
	fi
    fi
fi

# Set run-time font path
if [ -d \$ABISUITE_FONT_HOME ]
then
    xset fp+ \$ABISUITE_FONT_PATH 1>/dev/null 2>/dev/null
fi

# Figure out which binary to run
if [ -f \$ABISUITE_LIBEXEC/${PROGRAM_NAME}_d ]
then
    \$ABISUITE_LIBEXEC/${PROGRAM_NAME}_d "\$@"
elif [ -f \$ABISUITE_LIBEXEC/${PROGRAM_NAME}_s ]
then
    \$ABISUITE_LIBEXEC/${PROGRAM_NAME}_s "\$@"
else
    echo ""
    echo "Error: can't find ${PROGRAM_NAME} executables:"
    echo "    \$ABISUITE_LIBEXEC/${PROGRAM_NAME}_d"
    echo "    -or-"
    echo "    \$ABISUITE_LIBEXEC/${PROGRAM_NAME}_s"
    echo ""
    exit
fi
#Check to make sure we don't stomp on anything
if [ -z "\$currentFonts" ]
then
    # Set post run-time font path
    if [ -d \$ABISUITE_FONT_HOME ]
    then
	xset fp- \$ABISUITE_FONT_PATH 1>/dev/null 2>/dev/null
    fi
fi
EOF

chmod 755 ${LIBEXECDIR}/$PROGRAM_NAME


