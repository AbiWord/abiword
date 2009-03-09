dnl AbiWord
dnl Copyright (C) 2001 Sam Tobin-Hochstadt
dnl Copyright (C) 2005 J.M. Maurer <uwog@abisource.com>
dnl MacOS X hackery by Hubert Figuiere <hfiguiere@teaser.fr>
dnl
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License
dnl as published by the Free Software Foundation; either version 2
dnl of the License, or (at your option) any later version.
dnl 
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
dnl 02111-1307, USA.

dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)

m4_define([abiword_version_major], [2])
m4_define([abiword_version_minor], [6])
m4_define([abiword_version_micro], [8])
m4_define([abiword_series], [abiword_version_major.abiword_version_minor])
m4_define([abiword_version], [abiword_version_major.abiword_version_minor.abiword_version_micro])

AC_INIT([abiword],[abiword_version],[http://www.abisource.com/])
AC_CONFIG_SRCDIR([src/af/ev/xp/ev_Menu.h])
AC_CONFIG_AUX_DIR(ac-helpers)
dnl AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE

ABIWORD_SERIES="abiword_series"
AC_SUBST(ABIWORD_SERIES)
AC_DEFINE_UNQUOTED([ABIWORD_SERIES], [ "${ABIWORD_SERIES}" ])
AC_SUBST(ABIWORD_DATADIR,    "${datadir}/${PACKAGE_NAME}-${ABIWORD_SERIES}")
AC_SUBST(ABIWORD_GLADEDIR,   "${ABIWORD_DATADIR}/glade")
AC_SUBST(ABIWORD_HEADERSDIR, "${includedir}/${PACKAGE_NAME}-${ABIWORD_SERIES}")
AC_SUBST(ABIWORD_LIBDIR,     "${libdir}/${PACKAGE_NAME}-${ABIWORD_SERIES}")
AC_SUBST(ABIWORD_PLUGINSDIR, "${ABIWORD_LIBDIR}/plugins")

if test "$srcdir" = "."; then
	abi_rootdir=".."
else
	abi_rootdir="`dirname $srcdir`"
fi
abs_srcdir="`cd $srcdir && pwd`"

dnl Checks for programs.

abi_save_CFLAGS="$CFLAGS"
abi_save_CXXFLAGS="$CXXFLAGS"

AC_PROG_CXX
AC_PROG_CC
AC_ISC_POSIX
AC_PROG_CPP

CFLAGS="$abi_save_CFLAGS"
CXXFLAGS="$abi_save_CXXFLAGS"

AC_MSG_CHECKING(CFLAGS)
AC_MSG_RESULT($CFLAGS)

AC_MSG_CHECKING(CXXFLAGS)
AC_MSG_RESULT($CXXFLAGS)

ABI_FEATURE_DEFS=""

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
dnl AC_PROG_LIBTOOL
AM_ACLOCAL_INCLUDE(ac-helpers)

dnl Detect our platform (unix, win32, etc)
dnl Do this before the GNOME stuff
ABI_DETECT_PLATFORM
ABI_TYPES

dnl Checks for libraries.
dnl Define here ABI_FE, ABI_BE, PLATFORM_CFLAGS, PLATFORM_LIBS
with_unix=false
with_macosx=false
with_cocoa=false
with_qnx=false
with_win32=false

# this variable accumulates all the required pkg-config
# packages for the current configuration
ABIWORD_REQUIRED_PKGS=''

# when cross-compiling must have perl for cdump.pl
if test x$cross_compiling = xyes; then
AC_MSG_CHECKING([for perl])
if test "x$(which perl)" = "x"; then
   AC_MSG_ERROR([not found. Must have perl when cross-compiling.])
fi
AC_MSG_RESULT([found])
fi

# libabiword support
ABI_LIB

# check for embedded and print first, so we can handle gnomeprint dependency
# correcly
ABI_EMBEDDED

# by default we, of course, want Abi to be able to print, but on embedded
# systems it might not be required and because of the dependencies, we want an
# option to disable printing at compile time

# on embedded we will make --disable-print the default if no
# --enable/disable-printing is given

AC_MSG_CHECKING([for printing support])

AC_ARG_ENABLE(printing,[  --enable-printing    Include printing capabilities],[
	case "${enableval}" in
	   yes)	print=true ;;
	   no) print=false ;;
	     *)	AC_MSG_ERROR(bad value ${enableval} for --enable-printing) ;;
	esac
],[print=undefined])

if test "$print" = "undefined" && test "$embedded_print" = "false"; then
   print=false
elif test "$print" = "undefined"; then
   print=true;
fi

if test "$print" = "true" ; then
   AC_DEFINE(ENABLE_PRINT, 1, [Whether printing capabilities should be disabled])
fi

AC_MSG_RESULT([${print}])

AM_CONDITIONAL(ENABLE_PRINT, test "$print" = "true")

AM_CONDITIONAL(BUILD_TTFTOOL,[test $PLATFORM = unix && test x$print != xfalse])

AC_ARG_ENABLE(exports,[  --disable-exports    Do not export any symbols],[
	if test x$enableval = xno && test x$PLATFORM = xunix ; then
	   AC_DEFINE(ABI_UNIX_NO_EXPORTS, 1, [Whether no symbols should be exported])
	fi
],[])

if test "$PLATFORM" = "unix"; then
	ABI_FE="Unix"
	ABI_BE="$ABI_FE"
	with_unix=true
	dnl locate glib and gtk
	ABI_GTK2
if test $print = true; then
	ABI_GNOMEPRINT
fi
	ABI_FREETYPE
	AM_BINRELOC
elif test "$PLATFORM" = "cocoa" ; then
	ABI_GLIB2
	ABI_FE="Cocoa"
	ABI_BE="Unix"
	with_macosx=true
	with_cocoa=true

	dnl Don't change these (fjf)
	dnl 
	OBJCXX="c++"
	OBJCXXFLAGS=""
	OBJCXXDEPMODE="depmode=gcc3"
elif test "$PLATFORM" = "win" ; then
	with_win32=true
	ABI_FE="Win32"
	ABI_BE="$ABI_FE"
elif test "$PLATFORM" = "qnx" ; then
	ABI_FE="QNX"
	with_qnx=true
fi
AM_CONDITIONAL(WITH_UNIX,$with_unix)
AM_CONDITIONAL(WITH_MACOSX,$with_macosx)
AM_CONDITIONAL(WITH_COCOA,$with_cocoa)
AM_CONDITIONAL(WITH_QNX,$with_qnx)
AM_CONDITIONAL(WITH_WIN32,$with_win32)
AM_CONDITIONAL(WITH_BINRELOC, test "x$br_cv_binreloc" = "xyes")
AC_SUBST(OBJCXX)
AC_SUBST(OBJCXXFLAGS)
AC_SUBST(OBJCXXDEPMODE)

dnl These are the flags required and defined above
AC_SUBST(ABI_FE)
AC_SUBST(ABI_BE)

ABI_DETECT_ICONV

dnl ./configure command line arguments
dnl currently, we have:
dnl   --enable-debug
dnl   --enable-extra-optimization
dnl   --enable-profile
dnl   --enable-bidi
dnl   --enable-peer-config
dnl   --with-pspell
dnl   --with-libxml2
dnl The defined values are used mostly in includes.mk, but also in
dnl some other places

ABI_GUCHARMAP
ABI_XFT_QUICK
ABI_GSF
ABI_GOFFICE
ABI_GNOMEVFS
ABI_GNOMEUI

abi_build_shared=yes
abi_build_static=no

AC_ARG_ENABLE(shared,[  --disable-shared   don't build shared AbiWord (UNIX/GTK builds only)],[
	if test "x$enableval" = "xno"; then
		if test "$PLATFORM" = "unix" -a "$gnomeui" != "true"; then
			abi_build_shared=no
			abi_build_static=yes
		fi
	fi
])
AC_ARG_ENABLE(static,[  --enable-static          build static AbiWord (UNIX/GTK builds only)],[
	if test "x$enableval" != "xno"; then
		if test "$PLATFORM" = "unix" -a "$gnomeui" != "true"; then
			abi_build_static=yes
		fi
	fi
])

AM_CONDITIONAL(ABI_BUILD_SHARED,[test $abi_build_shared = yes])
AM_CONDITIONAL(ABI_BUILD_STATIC,[test $abi_build_static = yes])

dnl spell-checker detection precedes curl setting
dnl 
AC_LANG_CPLUSPLUS
ABI_SPELL
AC_LANG_C

ABI_FRIBIDI

ABI_PLUGINS

dnl The resource manager is a class (XAP_ResourceManager) designed to handle
dnl images/objects/hyperlinks included by the document. Disabled by default.
dnl 
abi_resource_manager=no
dnl AC_ARG_ENABLE(resource-manager,[  --enable-resource-manager  Use the resource manager (experimental)],[
dnl 	if test "x$enableval" = "xyes"; then
dnl 		abi_resource_manager=yes
dnl 		ABI_FEATURE_DEFS="$ABI_FEATURE_DEFS -DENABLE_RESOURCE_MANAGER=1"
dnl 	fi
dnl ])
AM_CONDITIONAL(WITH_RESOURCE_MANAGER,[test "x$abi_resource_manager" = "xyes"])

AC_ARG_ENABLE(profile,
    [  --enable-profile    Turn on profile ],
     [case "${enableval}" in
       yes) PROFILE_CFLAGS="-pg -fprofile-arcs -ftest-coverage" ;;
       no)  PROFILE_CFLAGS="" ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-profile) ;;
     esac],[PROFILE_CFLAGS=""])

AC_ARG_ENABLE(extra-optimization,
    [  --enable-extra-optimization    Turn on extra optimizations ],
     [case "${enableval}" in
       yes) OPTIMIZE_CFLAGS="-O3" ;;
       no)  OPTIMIZE_CFLAGS="-O2" ;; 
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-extra-optimization) ;;
     esac],[OPTIMIZE_CFLAGS=""])


AC_ARG_ENABLE(debug,[  --enable-debug    Turn on debugging],[
	case "${enableval}" in
	   yes)	debug=true ;
		DEBUG_CFLAGS="-DDEBUG -DUT_DEBUG -DFMT_TEST -DUT_TEST -DPT_TEST -g" ;
		OPTIMIZE_CFLAGS="" ;;
	    no)	debug=false ;
		DEBUG_CFLAGS="-DNDEBUG" ;;
	     *)	AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
	esac
],[	debug=false
	DEBUG_CFLAGS="-DNDEBUG"
])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

if test $debug = false; then
AC_ARG_ENABLE(symbols,[  --enable-symbols   Debugging symbols, but not debug-specific code],[
	case "${enableval}" in
	   yes)	symbols=true ;
		DEBUG_CFLAGS="-g" ;
		OPTIMIZE_CFLAGS="" ;;
	    no)	symbols=false ;;
	     *)	AC_MSG_ERROR(bad value ${enableval} for --enable-symbols) ;;
	esac
],[	symbols=false])
AM_CONDITIONAL(DEBUG, test x$symbols = xtrue)
fi

# by default, configure in the abi directory runs the same configure command
# in the peer directories (wv, expat, etc...).  this provides a way to
# disable that behavior.
AC_ARG_ENABLE(peer-config,
    [  --enable-peer-config  Enable automatic configuration of peer directories (on by default) ],
     [case "${enableval}" in
       yes) peerconfig=true ;;
       no)  peerconfig=false ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-peer-config) ;;
      esac],[peerconfig=true])

if test "$PLATFORM" = "unix"; then
	PLATFORM_CFLAGS="$GTK_CFLAGS $GMODULE_CFLAGS $GUCHARMAP_CFLAGS $GNOMEPRINT_CFLAGS $FREETYPE_CFLAGS $GOFFICE_CFLAGS $BONOBO_CFLAGS $GNOMEVFS_CFLAGS $GNOMEUI_CFLAGS -DSUPPORTS_UT_IDLE=1"
	PLATFORM_LIBS="$GTK_LIBS $GMODULE_LIBS $GUCHARMAP_LIBS $GNOMEPRINT_LIBS $FREETYPE_LIBS $BONOBO_LIBS $GNOMEVFS_LIBS $GNOMEUI_LIBS"
	
	if test "$embedded" != "none"; then
		PLATFORM_CFLAGS="$PLATFORM_CFLAGS $EMBEDDED_CFLAGS"
		PLATFORM_LIBS="$PLATFORM_LIBS $EMBEDDED_LIBS"
	fi		
fi
dnl AUTOCONF SUCKS
if test "$PLATFORM" = "cocoa"; then
	if "$debug" = "true"; then
		PLATFORM_LIBS="$GLIB_LIBS -liconv -framework Cocoa"
	else
		PLATFORM_LIBS="$GLIB_LIBS -liconv -framework Cocoa"
	fi
	PLATFORM_CFLAGS="$GLIB_CFLAGS -fpascal-strings -DNO_SYS_ERRLIST"
fi

if test "$PLATFORM" = "qnx"; then
	PLATFORM_LIBS="-fexceptions"
fi

# if we are not using spell-checker, need to propagate the WITHOUT_SPELL
# define down the tree
if test x$do_spell = xfalse ; then
   AC_DEFINE(WITHOUT_SPELL, 1, [Whether spellchecking should be disabled])
fi

dnl SUBST all the variables we just defined
AC_SUBST(ABI_FEATURE_DEFS)
AC_SUBST(OPTIMIZE_CFLAGS)
AC_SUBST(PROFILE_CFLAGS)
AC_SUBST(LIBJPEG_LIBS)
AC_SUBST(DEBUG_CFLAGS)
AC_SUBST(PLATFORM_CFLAGS)
AC_SUBST(PLATFORM_LIBS)
AC_SUBST(XSUBPPDIR)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(limits.h strings.h sys/time.h unistd.h malloc.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
ABI_C_INLINE
AC_TYPE_SIZE_T
AC_C_BIGENDIAN
AC_DEFINE(CHECKED_ENDIANNESS)
AM_CONDITIONAL(BIGENDIAN,[ test "x$ac_cv_c_bigendian" = "xyes" ])

dnl Currently, we need all of the following, but in the future we
dnl might not.  
dnl
dnl check for an xml parser

ABI_XML_PARSER

dnl check for wv
ABI_WV

dnl check for zlib.
ABI_ZLIB

dnl check for libpng.
ABI_LIBPNG

dnl check for popt.
ABI_POPT

dnl check for libjpeg
ABI_LIBJPEG

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_TYPE_SIGNAL

if test $peerconfig = "true"; then
    for i in . $PEERDIRS; do
        if test $i != . ; then
            j=`basename $i`
            ABI_PEER_CONFIG($j,$i)
        fi
    done
fi

# Check whether the scandir() select/filter function requires that its
# argument is const-qualified.
# uses the C++ compiler.
ABI_FUNC_SCANDIR_SELECT_CHECK

AC_SUBST(PEERDIRS)
AC_SUBST(PEERS)

dnl Check for valgrind for the UniTest
AC_CHECK_PROG(VALGRIND, valgrind, valgrind)
AC_CHECK_HEADERS(valgrind/memcheck.h)
if test "$VALGRIND" = ""; then
	AC_MSG_WARN([Valgrind is missing. UnitTesting will run without])
fi
if test "$VALGRIND" != ""; then
dnl due to stupid m4 I had to double the [ and ] in the regexp
	vg_ver=`valgrind --version | sed -e  's/.*-\([[0-9]]\)\.[[0-9]].*/\1/'`
	VALGRIND="$VALGRIND --tool=memcheck --leak-check=yes --num-callers=10"
	if test "$vg_ver" != "3" ; then
	   VALGRIND="$VALGRIND --logfile-fd=-1"
	fi
fi
AC_SUBST(VALGRIND)

AM_CONDITIONAL(CROSS_COMPILING, test x$cross_compiling = xyes)

# this must stay after all custrom m4 macros
AC_SUBST(ABIWORD_REQUIRED_PKGS)

AC_CONFIG_FILES([goffice-bits/GNUmakefile])

AC_CONFIG_FILES([
 abiword.pc
 libabiword.pc
 GNUmakefile
 src/GNUmakefile
 src/af/GNUmakefile
 src/af/tf/GNUmakefile
 src/af/ev/GNUmakefile
 src/af/ev/xp/GNUmakefile
 src/af/ev/cocoa/GNUmakefile
 src/af/ev/qnx/GNUmakefile
 src/af/ev/unix/GNUmakefile
 src/af/ev/unix/gnome/GNUmakefile
 src/af/ev/win/GNUmakefile
 src/af/gr/GNUmakefile
 src/af/gr/xp/GNUmakefile
 src/af/gr/cocoa/GNUmakefile
 src/af/gr/unix/GNUmakefile
 src/af/gr/qnx/GNUmakefile
 src/af/gr/win/GNUmakefile
 src/af/util/GNUmakefile
 src/af/util/xp/GNUmakefile
 src/af/util/xp/t/GNUmakefile
 src/af/util/unix/GNUmakefile
 src/af/util/qnx/GNUmakefile
 src/af/util/win/GNUmakefile
 src/af/tf/xp/GNUmakefile
 src/af/xap/GNUmakefile
 src/af/xap/xp/GNUmakefile
 src/af/xap/cocoa/GNUmakefile
 src/af/xap/qnx/GNUmakefile
 src/af/xap/win/GNUmakefile
 src/af/xap/unix/GNUmakefile
 src/af/xap/unix/t/GNUmakefile
 src/af/xap/unix/hildon/GNUmakefile
 src/config/GNUmakefile
 src/config/platforms/GNUmakefile
 src/config/require/GNUmakefile
 src/config/require/win/GNUmakefile
 src/other/GNUmakefile
 src/other/spell/GNUmakefile
 src/other/spell/xp/GNUmakefile
 src/pkg/GNUmakefile
 src/pkg/maemo/GNUmakefile
 src/pkg/maemo/abiword.desktop
 src/pkg/maemo/com.abisource.abiword.service
 src/pkg/maemo/debian/GNUmakefile
 src/text/GNUmakefile
 src/text/fmt/GNUmakefile
 src/text/fmt/xp/GNUmakefile
 src/text/fmt/unix/GNUmakefile
 src/text/ptbl/GNUmakefile
 src/text/ptbl/xp/GNUmakefile
 src/text/ptbl/xp/t/GNUmakefile
 src/wp/GNUmakefile
 src/wp/ap/GNUmakefile
 src/wp/ap/xp/GNUmakefile
 src/wp/ap/cocoa/GNUmakefile
 src/wp/ap/qnx/GNUmakefile
 src/wp/ap/unix/GNUmakefile
 src/wp/ap/unix/hildon/GNUmakefile
 src/wp/ap/win/GNUmakefile
 src/wp/ap/win/ToolbarIcons/GNUmakefile
 src/wp/impexp/GNUmakefile
 src/wp/impexp/cocoa/GNUmakefile
 src/wp/impexp/unix/GNUmakefile
 src/wp/impexp/qnx/GNUmakefile
 src/wp/impexp/win/GNUmakefile
 src/wp/impexp/xp/GNUmakefile
])
AC_CONFIG_FILES([
 src/wp/main/GNUmakefile
 src/wp/main/xp/GNUmakefile
 src/wp/main/cocoa/GNUmakefile
 src/wp/main/cocoa/bundle/GNUmakefile
 src/wp/main/cocoa/bundle/Info.plist
 src/wp/main/cocoa/AbiWord.app/GNUmakefile
 src/wp/main/cocoa/AbiWord.app/Contents/GNUmakefile
 src/wp/main/cocoa/AbiWord.app/Contents/MacOS/GNUmakefile
 src/wp/main/cocoa/AbiWord.app/Contents/Frameworks/GNUmakefile
 src/wp/main/cocoa/AbiWord.app/Contents/Resources/GNUmakefile
 src/wp/main/cocoa/AbiWord.app/Contents/Resources/English.lproj/GNUmakefile
 src/wp/main/qnx/GNUmakefile
 src/wp/main/unix/GNUmakefile
 src/wp/main/win/GNUmakefile
 src/wp/test/GNUmakefile
 src/wp/test/xp/GNUmakefile
 src/wp/test/unix/GNUmakefile
 src/wp/test/unix/testwrap.sh
 src/wp/test/win/GNUmakefile
 src/wp/test/win/testwrap.sh
 src/wp/test/cocoa/GNUmakefile
 src/tools/GNUmakefile
 src/tools/cdump/GNUmakefile
 src/tools/cdump/xp/GNUmakefile
 src/tools/po2str/GNUmakefile
 src/tools/po2str/xp/GNUmakefile
 user/GNUmakefile
 user/wp/GNUmakefile
 user/wp/strings/GNUmakefile
 user/wp/templates/GNUmakefile
])
AC_OUTPUT

echo " "

case "$PLATFORM" in
     "unix" )
       AC_MSG_RESULT([Configured to build on a Unix/Linux platform.]) ;;
     "win32" )
       AC_MSG_RESULT([Configured to build on a Windows platform.]) ;;
     "qnx" )
       AC_MSG_RESULT([Configured to build on a QNX RTOS platform.]) ;;
     "cocoa" )
       AC_MSG_RESULT([Configured to build on a Mac OS X platform using Cocoa.]) 
       if test ! -z $ABI_MACSDK_ROOT ; then 
         AC_MSG_RESULT([  Using SDK in $ABI_MACSDK_ROOT ])
       fi
       ;;
esac

case "$embedded" in
     "none" ) ;;
     *)
	AC_MSG_RESULT([Configured for embedded target (${embedded}).]) ;;
esac

case "$debug" in
     "true" )
        AC_MSG_RESULT([Configured with debugging symbols.]) ;;
esac

case "$print" in
	 "true" )
        AC_MSG_RESULT([Configured with printing capabilities.]) ;;
	 "false" )
        AC_MSG_RESULT([Configured without printing capabilites.]) ;;
esac

case "$do_spell" in
	 "false" )
        AC_MSG_RESULT([Configured without spellchecking capabilities.]) ;;
	 "*" )
        AC_MSG_RESULT([Configured with spellchecking capabilites.]) ;;
esac

case "$script" in
     "true" )
        AC_MSG_RESULT([Configured with scripting support.]) ;;
esac

case "$bidi" in
     "true" )
        AC_MSG_RESULT([Configured with BiDi support.]) ;;
esac

case "$xft" in
     "true" )
	AC_MSG_RESULT([Configured with XFT support.]) ;;
esac

case "$cross_compiling" in
     "yes" )
	AC_MSG_RESULT([Cross compiling -- not using binary custom tools.]) ;;
esac

echo " "

cat <<EOF
  XML Parser:             ${abi_xml_parser_message}
  zlib:                   ${abi_zlib_message}
  PNG Library:            ${abi_libpng_message}
  popt:	                  ${abi_libpopt_message}
  WV Library:             ${abi_wv_message}
  iconv:                  ${abi_iconv_message}
  spelling:               ${abi_spell_message}
  goffice:                ${abi_goffice_message}
  gnome-vfs:              ${abi_gnomevfs_message}
  gnome-ui:               ${abi_gnomeui_message}
  libabiword:             ${abi_lib_message}
  binreloc:               ${br_cv_binreloc}

  installing in:          ${prefix}

EOF


if test "x$_abi_wv_warning" = "xyes"; then
	ABI_WV_WARNING
fi

AC_MSG_RESULT([Type make to build AbiWord.])
