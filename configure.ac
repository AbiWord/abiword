dnl AbiWord
dnl Copyright (C) 2001 Sam Tobin-Hochstadt
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
AC_INIT
AC_CONFIG_SRCDIR([src/af/ev/xp/ev_Menu.h])
AC_CONFIG_AUX_DIR(ac-helpers)
dnl AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE(AbiSuite,1.1.0)

abs_srcdir="`cd $srcdir && pwd`"

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_ISC_POSIX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
dnl AC_PROG_LIBTOOL
AM_ACLOCAL_INCLUDE(ac-helpers)

dnl Detect our platform (unix, win32, etc)
dnl Do this before the GNOME stuff
ABI_DETECT_PLATFORM

dnl Checks for libraries.
dnl Define here ABI_FE, ABI_BE, PLATFORM_CFLAGS, PLATFORM_LIBS
with_unix=false
with_macosx=false
with_carbon=false
with_cocoa=false
if test "$PLATFORM" = "unix"; then
	ABI_FE="Unix"
	ABI_BE="$ABI_FE"
	with_unix=true
	dnl locate glib and gtk
	ABI_GLIB_GTK
elif test "$PLATFORM" = "mac" ; then
	ABI_GLIB
	ABI_FE="Mac"
	ABI_BE="$ABI_FE"
	with_macosx=true
	with_carbon=true
elif test "$PLATFORM" = "cocoa" ; then
	ABI_GLIB
	ABI_FE="Cocoa"
	ABI_BE="Unix"
	with_macosx=true
	with_cocoa=true

	dnl Don't change these (fjf)
	dnl 
	OBJCXX="c++"
	OBJCXXFLAGS=""
	OBJCXXDEPMODE="depmode=gcc3"
fi
AM_CONDITIONAL(WITH_UNIX,$with_unix)
AM_CONDITIONAL(WITH_MACOSX,$with_macosx)
AM_CONDITIONAL(WITH_CARBON,$with_carbon)
AM_CONDITIONAL(WITH_COCOA,$with_cocoa)
AC_SUBST(OBJCXX)
AC_SUBST(OBJCXXFLAGS)
AC_SUBST(OBJCXXDEPMODE)

dnl These are the flags required and defined above
AC_SUBST(ABI_FE)
AC_SUBST(ABI_BE)

ABI_DETECT_ICONV

ABIWORD_APP_NAME="AbiWord"
AC_SUBST(ABIWORD_APP_NAME)
ABIWORD_APP_LIBDIR="AbiWord"
AC_SUBST(ABIWORD_APP_LIBDIR)

dnl ./configure command line arguments
dnl currently, we have:
dnl   --enable-gnome
dnl   --enable-debug
dnl   --enable-extra-optimization
dnl   --enable-profile
dnl   --enable-bidi
dnl   --enable-peer-config
dnl   --with-pspell
dnl   --with-libxml2
dnl The defined values are used mostly in includes.mk, but also in
dnl some other places


ABI_GNOME_QUICK

ABI_XFT_QUICK

abi_build_shared=yes
abi_build_static=no

AC_ARG_ENABLE(shared,[  --disable-shared   don't build shared AbiWord (UNIX/GTK builds only)],[
	if test "x$enableval" = "xno"; then
		if test "$PLATFORM" = "unix" -a "$gnome" != "true"; then
			abi_build_shared=no
			abi_build_static=yes
		fi
	fi
])
AC_ARG_ENABLE(stable,[  --enable-static          build static AbiWord (UNIX/GTK builds only)],[
	if test "x$enableval" != "xno"; then
		if test "$PLATFORM" = "unix" -a "$gnome" != "true"; then
			abi_build_static=yes
		fi
	fi
])

AM_CONDITIONAL(ABI_BUILD_SHARED,[test $abi_build_shared = yes])
AM_CONDITIONAL(ABI_BUILD_STATIC,[test $abi_build_static = yes])

ABI_CURL
AM_CONDITIONAL(WITH_CURL, test x$abi_curl = xyes)

ABI_SPELL

dnl BiDi (Bi-Directional) is required now
dnl 
BIDI_CFLAGS="-DBIDI_ENABLED"
bidi=true
AM_CONDITIONAL(BIDI_ENABLED, test "x$bidi" = "xtrue")

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

#
# Checks for FreeType
#
FREETYPE_LIBS=
FREETYPE_CFLAGS=
have_freetype=false
AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
if test "x$FREETYPE_CONFIG" != "xno" ; then
  FREETYPE_CFLAGS=`$FREETYPE_CONFIG --cflags`
  FREETYPE_LIBS=`$FREETYPE_CONFIG --libs`

  AC_CHECK_LIB(freetype, FT_New_Face, have_freetype=true
	       ,:,$FREETYPE_LIBS)

  if $have_freetype ; then
    abi_save_cppflags="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $FREETYPE_CFLAGS"

    AC_MSG_CHECKING([For sufficiently new FreeType (at least 2.0.1)])
    AC_TRY_COMPILE([
#include <freetype/freetype.h>
#include FT_ERRORS_H
]
                ,[(void)1;],:,have_freetype=false)
    if $have_freetype ; then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi

    CPPFLAGS="$abi_save_cppflags"
  fi
fi
AC_SUBST(FREETYPE_LIBS)
AC_SUBST(FREETYPE_CFLAGS)
AM_CONDITIONAL(HAVE_FREETYPE, $have_freetype) 

## see which freetype we have, 2.0.3 vs. 2.0.4
abi_save_cppflags="$CPPFLAGS"  
CPPFLAGS="$CPPFLAGS $FREETYPE_CFLAGS"
AC_CHECK_HEADER(freetype/internal/tterrors.h, AC_DEFINE(HAVE_FREETYPE_2_0_3))
CPPFLAGS="$abi_save_cppflags"

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

AC_ARG_ENABLE(scripting,
    [  --enable-scripting   Enable the perl scripting interface ],
     [case "${enableval}" in
       yes) script=true ;;
       no)  script=false ;; 
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-scripting) ;;
     esac],[script=false])

dnl this isn't actually used anywhere, but we provide it anyway
AM_CONDITIONAL(SCRIPT, test x$script = xtrue)

case "$script" in
     "true" ) 
	    SCRIPT_CFLAGS="-DABI_OPT_PERL `perl -MExtUtils::Embed -e ccopts` -Ubool"
	    SCRIPT_LIBS="`perl -MExtUtils::Embed -e ldopts`"
		AC_MSG_CHECKING(for perl xsubpp precompiler)
		XSUBPPDIR="`(cd $abs_srcdir/src/bindings/perl && perl -MExtUtils::MakeMaker -e 'print ExtUtils::MakeMaker->new({NAME => qw(abi)})->tool_xsubpp') | grep ^XSUBPPDIR | sed -e 's/XSUBPPDIR = //g;'`"
		if test -n "${XSUBPPDIR}"; then
			AC_MSG_RESULT(${XSUBPPDIR})
		else
			AC_MSG_ERROR(not found)
		fi
	    ;;
     * )     
    	    SCRIPT_CFLAGS=""
	    SCRIPT_LIBS=""
	    ;;
esac

if test "$PLATFORM" = "unix"; then
	if test "$gnome" = "true"; then
		PLATFORM_CFLAGS="$GNOME_CFLAGS $GMODULE_CFLAGS $GTK_CFLAGS -DSUPPORTS_UT_IDLE=1"
		PLATFORM_LIBS=
	else
		PLATFORM_CFLAGS="$GMODULE_CFLAGS $GTK_CFLAGS -DSUPPORTS_UT_IDLE=1"
		PLATFORM_LIBS=
	fi
fi
dnl AUTOCONF SUCKS
if test "$PLATFORM" = "mac"; then
	PLATFORM_LIBS="-liconv -framework Carbon -framework CoreServices -framework ApplicationServices -framework QuickTime"
	PLATFORM_CFLAGS=-I'$(top_builddir)/src/af/util/unix'\ -fpascal-strings\ -DUSE_CARBON_EVENTS\ -DCARBON_ON_MACH_O=1\ -DNO_SYS_ERRLIST\ -I/Developer/Headers/FlatCarbon
fi
if test "$PLATFORM" = "cocoa"; then
	if "$debug" = "true"; then
		PLATFORM_LIBS="$GLIB_LIBS -liconv -framework Cocoa"
	else
		PLATFORM_LIBS="$GLIB_LIBS -liconv -framework Cocoa"
	fi
	PLATFORM_CFLAGS="$GLIB_CFLAGS -fpascal-strings -DNO_SYS_ERRLIST"
fi


dnl SUBST all the variables we just defined
AC_SUBST(OPTIMIZE_CFLAGS)
AC_SUBST(PROFILE_CFLAGS)
AC_SUBST(LIBJPEG_LIBS)
AC_SUBST(DEBUG_CFLAGS)
AC_SUBST(SCRIPT_CFLAGS)
AC_SUBST(SCRIPT_LIBS)
AC_SUBST(BIDI_CFLAGS)
AC_SUBST(PLATFORM_CFLAGS)
AC_SUBST(PLATFORM_LIBS)
AC_SUBST(XSUBPPDIR)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(limits.h strings.h sys/time.h unistd.h malloc.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_C_BIGENDIAN
AM_CONDITIONAL(BIGENDIAN,[ test "x$ac_cv_c_bigendian" = "xyes" ])

dnl Currently, we need all of the following, but in the future we
dnl might not.  
dnl
dnl check for an xml parser

ABI_XML_PARSER

dnl check for wv

ABI_NEED_WV="yes"
ABI_WV(${abs_srcdir}/../wv)

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
AC_CHECK_FUNCS(re_comp regcomp strdup strstr)

AC_CHECK_FUNC(putenv,abi_putenv=maybe,abi_putenv=no)
AC_CHECK_FUNC(setenv,abi_setenv=maybe,abi_setenv=no)

if test $abi_putenv = no -a $abi_setenv = no; then
	AC_MSG_ERROR([* * * neither setenv nor putenv found! * * *])
fi
if test $abi_setenv = no; then
	AC_DEFINE(SETENV_MISSING,1,[use putenv, not setenv])
elif test $abi_putenv = maybe; then
	_abi_cppflags=$CPPFLAGS
	CPPFLAGS="$CPPFLAGS $WARNING_CFLAGS"

	AC_MSG_CHECKING([whether setenv declared in stdlib.h (for us)])
	AC_TRY_COMPILE([
#include <stdlib.h>
float setenv (int);
	],[
/* if this program fails then setenv isn't declared in stdlib.h 
 * or at least not for the flags we're using...
 */
	],[	AC_MSG_RESULT(no)
		dnl well, okay, setenv may not be missing but I'm going to
		dnl mark it as such:
		AC_DEFINE(SETENV_MISSING,1,[use putenv, not setenv])
	],[	AC_MSG_RESULT(yes)
	])

	CPPFLAGS=$_abi_cppflags
fi

if test $peerconfig = "true"; then
	dnl other configure scripts
	# AC_CONFIG_SUBDIRS($PEERDIRS) won't work :-(
	for i in . $PEERDIRS; do
	    if test $i != . ; then
		echo "Running configure in ${i}"
		config_flags='--disable-shared --enable-static'
		if test `basename "$i"` = wv; then
		    config_flags="$config_flags --with-exporter --with-glib=$abi_glib_opt"
		    config_flags="$config_flags $abi_wv_xml $abi_wv_iconv $abi_wv_png $abi_wv_zlib"
		    (cd $i; CPPFLAGS="$CPPFLAGS $abi_wv_cppflags" ./configure $config_flags)
		else
		    (cd $i; ./configure $config_flags)
		fi
	    fi
	done
fi

AC_SUBST(PEERDIRS)
AC_SUBST(PEERS)

AC_CONFIG_FILES([GNUmakefile
 src/GNUmakefile
 src/af/GNUmakefile
 src/af/ev/GNUmakefile
 src/af/ev/xp/GNUmakefile
 src/af/ev/cocoa/GNUmakefile
 src/af/ev/unix/GNUmakefile
 src/af/ev/unix/gnome/GNUmakefile
 src/af/ev/mac/GNUmakefile
 src/af/gr/GNUmakefile
 src/af/gr/xp/GNUmakefile
 src/af/gr/mac/GNUmakefile
 src/af/gr/cocoa/GNUmakefile
 src/af/gr/unix/GNUmakefile
 src/af/util/GNUmakefile
 src/af/util/xp/GNUmakefile
 src/af/util/mac/GNUmakefile
 src/af/util/unix/GNUmakefile
 src/af/xap/GNUmakefile
 src/af/xap/xp/GNUmakefile
 src/af/xap/cocoa/GNUmakefile
 src/af/xap/mac/GNUmakefile
 src/af/xap/unix/GNUmakefile
 src/af/xap/unix/gnome/GNUmakefile
 src/other/GNUmakefile
 src/other/spell/GNUmakefile
 src/other/spell/xp/GNUmakefile
 src/other/fribidi/GNUmakefile
 src/other/fribidi/xp/GNUmakefile
 src/other/ttftool/GNUmakefile
 src/other/ttftool/unix/GNUmakefile
 src/text/GNUmakefile
 src/text/fmt/GNUmakefile
 src/text/fmt/xp/GNUmakefile
 src/text/ptbl/GNUmakefile
 src/text/ptbl/xp/GNUmakefile
 src/wp/GNUmakefile
 src/wp/ap/GNUmakefile
 src/wp/ap/xp/GNUmakefile
 src/wp/ap/cocoa/GNUmakefile
 src/wp/ap/mac/GNUmakefile
 src/wp/ap/unix/GNUmakefile
 src/wp/ap/unix/gnome/GNUmakefile
 src/wp/impexp/GNUmakefile
 src/wp/impexp/cocoa/GNUmakefile
 src/wp/impexp/unix/GNUmakefile
 src/wp/impexp/unix/gnome/GNUmakefile
 src/wp/impexp/xp/GNUmakefile
 src/wp/main/GNUmakefile
 src/wp/main/xp/GNUmakefile
 src/wp/main/cocoa/GNUmakefile
 src/wp/main/cocoa/bundle/GNUmakefile
 src/wp/main/cocoa/bundle/Info.plist
 src/wp/main/mac/GNUmakefile
 src/wp/main/unix/GNUmakefile
 src/wp/main/unix/gnome/GNUmakefile
 src/tools/GNUmakefile
 src/tools/cdump/GNUmakefile
 src/tools/cdump/xp/GNUmakefile
 src/tools/pfa2afm/GNUmakefile
 src/tools/pfa2afm/unix/GNUmakefile
 src/tools/ttftool/GNUmakefile
 src/tools/ttftool/unix/GNUmakefile
 src/tools/scripts/GNUmakefile
 src/tools/scripts/unix/GNUmakefile
 src/bindings/GNUmakefile
 src/bindings/perl/GNUmakefile
])
AC_OUTPUT

echo " "

case "$PLATFORM" in
     "unix" )
       AC_MSG_RESULT([Configured to build on a Unix/Linux platform.]) ;;
     "win" )
       AC_MSG_RESULT([Configured to build on a Windows platform.]) ;;
     "beos" )
       AC_MSG_RESULT([Configured to build on a BeOS platform.]) ;;
     "qnx" )
       AC_MSG_RESULT([Configured to build on a QNX RTOS platform.]) ;;
     "cocoa" )
       AC_MSG_RESULT([Configured to build on a Mac OS X platform using Cocoa.]) ;;
     "mac" )
       AC_MSG_RESULT([Configured to build on a Mac OS X platform using Carbon.]) ;;
esac

case "$gnome" in
     "true" )
        AC_MSG_RESULT([Configured with GNOME support.]) ;;
esac

case "$libjpeg" in
     "true" )
        AC_MSG_RESULT([Configured with libjpeg support.]) ;;
esac

case "$debug" in
     "true" )
        AC_MSG_RESULT([Configured with debugging symbols.]) ;;
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

echo " "

cat <<EOF
  XML Parser:      ${abi_xml_parser_message}
  zlib:            ${abi_zlib_message}
  PNG Library:     ${abi_libpng_message}
  popt:		   ${abi_libpopt_message}
  WV Library:      ${abi_wv_message}
  iconv:           ${abi_iconv_message}
  spelling:        ${abi_spell_message}

EOF

AC_MSG_RESULT([Type make to build AbiWord.])
