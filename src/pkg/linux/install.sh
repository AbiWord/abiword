#!/bin/sh
#
#  AbiSource Unix Installer Program
#  Copyright (C) 1999 AbiSource, Inc.
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

# Program execution
cat <<EOF

  AbiSuite Program Installer, Copyright (C) 1999 AbiSource, Inc.

  This program comes with ABSOLUTELY NO WARRANTY; this software is
  free software, and you are welcome to redistribute it under
  certain conditions.  Read the file called COPYING in the archive 
  in which this program arrived for more details.

EOF

# Make sure the fonts are really here
if [ ! -f ${INSTALL_DATA_FILE} ]
then
    echo ""
    echo "Fatal error: can't find the file [${INSTALL_DATA_FILE}]"
    echo "which contains AbiSuite program and data files."
    echo ""
    exit 1
fi

# Read prefix from user
cat <<EOF

  Please specify the directory in which you would like to install AbiSuite.
  The default directory is [${DEFAULT_PREFIX}/${DEFAULT_HOME}], 
  but you may provide an alternate path if you wish.  Hit "Enter" to use
  the default value.
EOF

GO=1
while test ${GO} -eq 1
do
    echo ""
    echo -n "Installation path for AbiSuite software [${DEFAULT_PREFIX}/${DEFAULT_HOME}]: "
    read INSTALL_BASE

    # did they just hit enter?
    if [ -z "${INSTALL_BASE}" ]
    then
	INSTALL_BASE="${DEFAULT_PREFIX}/${DEFAULT_HOME}"
    fi

    # GO off, passed tests
    GO=0
    if [ ! -d ${INSTALL_BASE} ]
    then
	echo ""
	echo -n "The directory [${INSTALL_BASE}] does not exist; would you like to create it now? (y/n) [y]: "
	read CREATE_DIR
	if [ "${CREATE_DIR}" = "n" -o "${CREATE_DIR}" = "N" ]
	then
	    GO=1
	else
	    mkdir -p ${INSTALL_BASE}
	    if [ $? -ne 0 ]
	    then
		echo ""
		echo "I couldn't create [${INSTALL_BASE}].  You are probably seeing this error"
 		echo "because you do not have sufficient permission to perform this operation."
		echo "You will most likely have to run this script as superuser to write to"
		echo "system directories."
		exit 2
	    fi
	fi
    else
	echo ""
	echo "  I found an existing directory called [${INSTALL_BASE}].  You can choose"
	echo "  to install into this directory, but existing files will be modified or"
	echo "  replaced.  You can also choose not to install into this directory, and"
	echo "  you will be prompted for another."
	echo ""
	echo -n "Do you want to install into [${INSTALL_BASE}]? (y/n) [y]: "
	read INSTALL_OVER
	if [ "${INSTALL_OVER}" = "n" -o "${INSTALL_OVER}" = "N" ]
	then
	    GO=1
	fi
    fi
done

########################################################################
# Blow up our data file
########################################################################

echo ""
echo "Installing AbiSuite software in [${INSTALL_BASE}]..."
cd ${INSTALL_BASE}
tar xf ${INSTALL_DATA_FILE}

########################################################################
# If we're on Solaris, run makepsres (for DPS X servers)
########################################################################

if (

########################################################################
# Dynamically construct a wrapper for AbiSuite binaries
########################################################################

# do AbiWord

cat > ${INSTALL_BASE}/bin/AbiWord <<EOF
#!/bin/sh
# change this if you move the AbiSuite tree
ABISUITE_HOME=${INSTALL_BASE}
# change this if you move your fonts
ABISUITE_FONTHOME=\$\(ABISUITE_HOME\)/fonts
# set font path
if [ -d \$\(ABISUITE_FONT_HOME\) ]
then
    xset fp+ \$\(ABISUITE_FONT_HOME\)
endif
# which binary to run?
if [ -f \$\(ABISUITE_HOME\)/bin/AbiWord_d ]
then
    \$\(ABISUITE_HOME\)/bin/AbiWord_d
elif [ -f \$\(ABISUITE_HOME\)/bin/AbiWord_s ]
then
    \$\(ABISUITE_HOME\)/bin/AbiWord_s
else
    echo "Can't find AbiWord executables:"
    echo "    \$\(ABISUITE_HOME\)/bin/AbiWord_d"
    echo "    \$\(ABISUITE_HOME\)/bin/AbiWord_s"
    echo ""
    echo "Where did they go?"
endif
# remove font path
if [ -d \$\(ABISUITE_FONT_HOME\) ]
then
    xset fp- \$\(ABISUITE_FONT_HOME\)
endif
EOF

echo ""
echo "Done!  A total of [${TOTALNUM}] files were successfully installed."
echo ""
