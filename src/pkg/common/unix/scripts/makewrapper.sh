#!/bin/sh
#
# This script is invoked to create wrapper scripts for AbiSuite
# binaries.  One should pass it the _base_ name of the program
# to wrap (one would pass "AbiWord" to wrap "AbiWord-2.0" and 
# "AbiWord-2.0"), the install base of the tree and, optionally, the location
# of the machine-dependent binaries and scripts, and, optionally, an
# installation-only prefix.

# Examples:
#     makewrapper.sh AbiWord /usr/local/AbiSuite /usr/local/libexec/AbiSuite /tmp/package_root
#     makewrapper.sh AbiCalc /usr/local/AbiSuite /usr/local/libexec/AbiSuite

SCRIPT_PATH=$1
PROGRAM_NAME=$2
INSTALL_BASE=$3
LIBEXECDIR=$4
DESTDIR=$5

# Did they supply any arguments?
if [ -z "$SCRIPT_PATH" ]
then
    echo ""
    echo "Error: first argument (script path/name) not specified."
    echo ""
    exit 1
fi
if [ -z "$PROGRAM_NAME" ]
then
    echo ""
    echo "Error: second argument (base program name) not specified."
    echo ""
    exit 1
fi
if [ -z "$INSTALL_BASE" ]
then
    echo ""
    echo "Error: third argument (installation base directory) not specified."
    echo ""
    exit 1
fi
if [ -z "$LIBEXECDIR" ]
then
    LIBEXECDIR=${INSTALL_BASE}/bin
fi

# Make directory path up to program we're creating
mkdir -p ${DESTDIR}${LIBEXECDIR}

cat >${DESTDIR}$SCRIPT_PATH <<EOF
#!/bin/sh
#
# AbiSuite program wrapper script, dynamically generated
# from abi/src/pkg/common/unix/scripts/makewrapper.sh.

EOF

# On SunOS (Solaris), set the library path to contain /usr/local/lib
# where gtk and other support libraries supposedly lies.
# See bug 1739 <http://bugzilla.abisource.com/show_bug.cgi?id=1739>
if [ `uname` = "SunOS" ]
then
	cat >>${DESTDIR}$SCRIPT_PATH <<EOF
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
export LD_LIBRARY_PATH

EOF
fi

cat >>${DESTDIR}$SCRIPT_PATH <<EOF
# Change this if you move the AbiSuite tree.
ABISUITE_HOME=$INSTALL_BASE
export ABISUITE_HOME

# Change this if you move the AbiSuite binaries.
ABISUITE_LIBEXEC=$LIBEXECDIR

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

EOF

chmod 755 ${DESTDIR}$SCRIPT_PATH

