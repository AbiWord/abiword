#!/bin/sh
#
# This script is invoked to create wrapper scripts for AbiSuite
# binaries.  One should pass it the _base_ name of the program
# to wrap (one would pass "AbiWord" to wrap "AbiWord_s" and 
# "AbiWord_d"), and the install base of the tree, to which "bin"
# will be appended for binary and script locations.

# Examples:
#     makewrapper.sh AbiWord /usr/local/AbiSuite
#     makewrapper.sh AbiCalc /usr/local/AbiSuite

PROGRAM_NAME=$1
INSTALL_BASE=$2

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

# Make directory path up to program we're creating
mkdir -p $INSTALL_BASE/bin

cat >$INSTALL_BASE/bin/$PROGRAM_NAME<<EOF
#!/bin/sh
#
# AbiSuite program wrapper script, dynamically generated
# from abi/src/pkg/common/unix/scripts/makewrapper.sh.

# Change this if you move the AbiSuite tree.
ABISUITE_HOME=$INSTALL_BASE
export ABISUITE_HOME

# Change this if you move your fonts.
ABISUITE_FONT_HOME=\$ABISUITE_HOME/fonts

# Set run-time font path
if [ -d \$ABISUITE_FONT_HOME ]
then
    xset fp+ \$ABISUITE_FONT_HOME 1>/dev/null 2>/dev/null
fi

# Figure out which binary to run
if [ -f \$ABISUITE_HOME/bin/${PROGRAM_NAME}_d ]
then
    \$ABISUITE_HOME/bin/${PROGRAM_NAME}_d "\$@"
elif [ -f \$ABISUITE_HOME/bin/${PROGRAM_NAME}_s ]
then
    \$ABISUITE_HOME/bin/${PROGRAM_NAME}_s "\$@"
else
    echo ""
    echo "Error: can't find ${PROGRAM_NAME} executables:"
    echo "    \$ABISUITE_HOME/bin/${PROGRAM_NAME}_d"
    echo "    -or-"
    echo "    \$ABISUITE_HOME/bin/${PROGRAM_NAME}_s"
    echo ""
    exit
fi

# Set post run-time font path
if [ -d \$ABISUITE_FONT_HOME ]
then
    xset fp- \$ABISUITE_FONT_HOME 1>/dev/null 2>/dev/null
fi
EOF

chmod 755 ${INSTALL_BASE}/bin/$PROGRAM_NAME


