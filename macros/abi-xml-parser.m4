# Copyright (C) 1998-2000 Joe Orton, Sam Tobin-Hochstadt
# This file is free software; you may copy and/or distribute it with
# or without modifications, as long as this notice is preserved.
# This software is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.

# The above license applies to THIS FILE ONLY, the abiword code
# itself may be copied and distributed under the terms of the GNU
# GPL, see COPYING for more details

# Check for XML parser.
# Supports:
#  *  libxml2
#  *  expat in -lexpat
#  *  expat in -lxmlparse and -lxmltok (as formerly packaged by Debian/Red Hat)
#  *  Bundled expat if a directory name argument is passed
#
# This has been hacked, and all the names have been changed.  
#
# Usage: 
#  ABI_XML_PARSER()
# or
#  ABI_XML_PARSER(expat-dir)

AC_DEFUN([ABI_XML_PARSER], [

if test "$ABI_NEED_XML_PARSER" = "yes"; then

abi_found_parser="no"

AC_ARG_WITH([libxml2],
 	[  --with-libxml2         force use of libxml2 ],
 	[abi_force_libxml2=$witheval],
 	[abi_force_libxml2=no])

AC_ARG_WITH([expat],
 	[  --with-expat         force use of expat ],
 	[abi_force_expat=$withval],
 	[abi_force_expat=no])

# check for expat

if test "$abi_found_parser" = "no"; then
	echo "checking for expat"
	AC_CHECK_LIB(expat, XML_Parse,
		abi_expat_libs="-lexpat" abi_found_parser="expat",
		AC_CHECK_LIB(xmlparse, XML_Parse,
			abi_expat_libs="-lxmltok -lxmlparse" 
			abi_found_parser="expat",
			abi_found_parser="no",
			-lxmltok )
		)
fi

# changed from the original neon test to detect libxml2, not libxml1.  

if test "$abi_found_parser" = "no" -a "$abi_force_expat" = "no" ; then
	#Have we got libxml2 or later?
	AC_CHECK_PROG(XML_CONFIG, xml2-config, xml2-config)
	if test "$XML_CONFIG" != ""; then
		#Check for recent library
		XML_LIBS="`$XML_CONFIG --libs` -DHAVE_LIBXML2"
		XML_CFLAGS="`$XML_CONFIG --cflags`"
		AC_CHECK_LIB(xml2, xmlCreatePushParserCtxt,
			abi_found_parser="libxml2" abi_xml_parser_message="libxml2"
			AC_DEFINE(HAVE_LIBXML2, 1, [Define if you have libxml2])
			,
			XML_CFLAGS=""
			XML_LIBS=""
			AC_WARN([cannot use libxml1, libxml2 is required ]))
		
	fi
fi

if test "$abi_found_parser" = "expat"; then
	# This is crap. Maybe just use AC_CHECK_HEADERS and use the
	# right file by ifdef'ing is best
	AC_CHECK_HEADER(expat.h,
	[abi_expat_incs="" abi_found_expatincs="yes" abi_expat_new="yes"=],
	AC_CHECK_HEADER(xmlparse.h,
	[abi_expat_incs="" abi_found_expatincs="yes" abi_expat_new=""],
	AC_CHECK_HEADER(xmlparse/xmlparse.h,
	[abi_expat_incs="-I/usr/include/xmlparse" abi_found_expatincs="yes" abi_expat_new=""],
	)))
	if test "$abi_found_expatincs" = "yes"; then
		AC_DEFINE(HAVE_EXPAT, 1, [Define if you have expat])
		if ! test "$abi_expat_new"; then
			AC_DEFINE(HAVE_OLD_EXPAT, 1, [Define if you have the jclark expat])
		fi
		if test "$abi_expat_incs"; then
			XML_CFLAGS="$abi_expat_incs"
		fi	
		XML_LIBS="$abi_expat_libs"
	else
	       AC_MSG_ERROR(["found expat library but could not find expat.h or xmlparse.h"])
	fi
	abi_xml_parser_message="expat in $abi_expat_libs"
fi

if test "$abi_found_parser" = "no" ; then
    if test "x$1" != "x" -a -a "$1"; then
	# Use the expat sources given as an argument
	XML_LIBS="$1/lib/.libs/libexpat.a"
	XML_CFLAGS="-I$1/lib/"
	AC_MSG_RESULT(using supplied expat XML parser)	
	AC_DEFINE(HAVE_EXPAT, 1, [Define if you have expat] )
	abi_xml_parser_message="supplied expat in $1"
	local_expat="true"
	AM_CONDITIONAL(LOCAL_EXPAT, test "$local_expat" = "true")
    else
	AC_MSG_ERROR([no XML parser was found])
    fi

fi

fi

AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)

])
