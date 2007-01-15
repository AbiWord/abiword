# start: abi/ac-helpers/abi-xml-parser.m4
# 
# Copyright (C) 2002 Francis James Franklin
# Copyright (C) 2002 AbiSource, Inc
# Copyright (C) 1998-2000 Joe Orton
# Copyright (C) 1998-2000 Sam Tobin-Hochstadt
# 
# This file is free software; you may copy and/or distribute it with
# or without modifications, as long as this notice is preserved.
# This software is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.
#
# The above license applies to THIS FILE ONLY, the abiword code
# itself may be copied and distributed under the terms of the GNU
# GPL, see COPYING for more details
#
# This file detects which of the AbiWord platforms we are currently
# building on.  The detection logic in question is mostly by Jeff
# Hostetler, and is taken from the original AbiWord build system.  
#
# Usage: ABI_XML_PARSER

# Check for XML parser.
# Supports:
#  *  libxml2
#  *  expat in -lexpat
#  *  peer expat

AC_DEFUN([ABI_XML_PARSER],[
if test "x$abi_xml" = "x"; then

XML_CFLAGS=""
XML_LIBS=""

EXPAT_PEERDIR=""

abi_xml=unknown

ABI_XML_DIR=
AC_ARG_WITH([libxml2],[  --with-libxml2         force use of libxml2 ],[
	if test "x$withval" = "xno"; then
		abi_xml=expat
	elif test "x$withval" = "xyes"; then
		abi_xml=xml2
	else
		abi_xml=xml2
		ABI_XML_DIR="$withval"
	fi
])

AC_ARG_WITH([expat],[  --with-expat         force use of expat ],[
	if test "x$withval" = "xno"; then
		if test $abi_xml != xml2; then
			AC_MSG_ERROR([* * * require either expat or libxml2 * * *])
		fi
		abi_xml=xml2
	elif test "x$withval" = "xpeer"; then
		if test $abi_xml = xml2; then
			AC_MSG_ERROR([* * * don't specify both expat and libxml2 * * *])
		fi
		abi_xml=peer
	elif test "x$withval" = "xyes"; then
		if test $abi_xml = xml2; then
			AC_MSG_ERROR([* * * don't specify both expat and libxml2 * * *])
		fi
		abi_xml=expat
	else
		if test $abi_xml = xml2; then
			AC_MSG_ERROR([* * * don't specify both expat and libxml2 * * *])
		fi
		abi_xml=expat
		ABI_XML_DIR="$withval"
	fi
])

dnl default to libxml2
dnl 
if test $abi_xml = unknown; then
	abi_xml=xml2
fi

dnl check for libxml2
dnl 
if test $abi_xml = xml2; then
	if test "x$ABI_XML_DIR" = "x"; then
		AC_PATH_PROG(ABI_XML_CONFIG,xml2-config, ,[$PATH])
	else
		AC_PATH_PROG(ABI_XML_CONFIG,xml2-config, ,[$ABI_XML_DIR/bin:$PATH])
	fi
	if test "x$ABI_XML_CONFIG" = "x"; then
		AC_MSG_ERROR([* * * unable to find xml2-config! require libxml2-devel * * *])
	fi
	XML_CFLAGS="`$ABI_XML_CONFIG --cflags`"
	XML_LIBS="`$ABI_XML_CONFIG --libs`"
	abi_xml_parser_message="libxml2 in $XML_LIBS"
fi

dnl check for expat (installed library)
dnl
if test $abi_xml = expat; then
	if test "x$ABI_XML_DIR" != "x"; then
		_abi_cppflags="$CPPFLAGS"
		_abi_ldflags="$LDFLAGS"
		CPPFLAGS="$CPPFLAGS -I$ABI_XML_DIR/include"
		LDFLAGS="$LDFLAGS -L$ABI_XML_DIR/lib"
	fi
	AC_CHECK_HEADER(expat.h,[
		AC_CHECK_LIB(expat,XML_Parse,[
			XML_LIBS="-lexpat"
		],[	AC_MSG_ERROR([* * * require expat-devel; or --with-expat=peer * * *])
		])
	],[	AC_MSG_ERROR([* * * require expat-devel; or --with-expat=peer * * *])
	])
	if test "x$ABI_XML_DIR" != "x"; then
		CPPFLAGS="$_abi_cppflags"
		LDFLAGS="$_abi_ldflags"
		XML_CFLAGS="-I$ABI_XML_DIR/include"
		XML_LIBS="-L$ABI_XML_DIR/lib -lexpat"
	fi
	abi_xml_parser_message="expat in $XML_LIBS"
fi

dnl check for expat (peer)
dnl
if test $abi_xml = peer; then
	abi_xml=expat
	AC_MSG_CHECKING(for peer expat)
	if test -d "$abi_rootdir/expat"; then
		EXPAT_PEERDIR="$abi_rootdir/expat"
		AC_MSG_RESULT($EXPAT_PEERDIR)
	else
		AC_MSG_ERROR(peer expat not found)
	fi
	XML_CFLAGS="-I\$(top_builddir)/../expat/lib"
	XML_LIBS="\$(top_builddir)/../expat/lib/.libs/libexpat.a"
	abi_xml_parser_message="peer expat"
        PEERDIRS="${PEERDIRS} ${EXPAT_PEERDIR}"
	PEERS="${PEERS} expat"
fi

if test $abi_xml = expat; then
	AC_DEFINE(HAVE_EXPAT,1,[Define if you have expat])
else
	AC_DEFINE(HAVE_LIBXML2,1,[Define if you have libxml2])
fi
AM_CONDITIONAL(HAVE_EXPAT, test $abi_xml = expat)

AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)

AC_SUBST(EXPAT_PEERDIR)

fi
])
# 
# end: abi/ac-helpers/abi-xml-parser.m4
