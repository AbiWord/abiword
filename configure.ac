
m4_define([abi_version_major], [3])
m4_define([abi_version_minor], [0])
m4_define([abi_version_micro], [99])
m4_define([abi_series], [abi_version_major.abi_version_minor])
m4_define([abi_version], [abi_version_major.abi_version_minor.abi_version_micro])

AC_INIT([abiword],[abi_version],[http://www.abisource.com/])
AC_CANONICAL_HOST
AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE([1.9 tar-ustar])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

dnl we use AM_MAINTAINER_MODE and enable it to allow to disable it.
dnl very usefull to build in scratchbox where the maintainer mode is used for the PC
dnl build but the embedded build just use pregenerated one (with a different version)
AM_MAINTAINER_MODE([enable])

#
# Dependency requirements 
#

#
# versions requested.
#
GTK_VERSION_REQ=3.0.8

dnl GTK2_VERSION_REQ=2.12.0
dnl
dnl AC_ARG_WITH([gtk2],
dnl 	[AS_HELP_STRING([--with-gtk2], [use Gtk library version 2.x])],
dnl [
dnl 	abi_cv_gtk2="$withval"
dnl ])


# This check MUST be done early.
#
# We define EMBEDDED_TARGET to a numerical value so that we can do like
# #if EMBEDDED_TARGET = EMBEDDED_TARGET_POKY
#
# The following have to be defined unconditionally, because we want 
# EMBEDDED_TARGET to be undefined if not building for embedded so that we 
# can use #ifdef and #ifndef on it (two undefined symbols will return 
# true if tested for equality in #if construct)
AC_DEFINE([EMBEDDED_TARGET_GENERIC], [1], [Generic embedded platform])
AC_DEFINE([EMBEDDED_TARGET_POKY],    [3], [Poky embedded platform])

AC_ARG_ENABLE([embedded],
	[AS_HELP_STRING([--enable-embedded], [=generic|poky, (Gtk+ only), build for embedded platform])],
[
	abi_cv_embedded_platform="$enableval"
	case "$enableval" in
	poky)
		abi_cv_embedded_target=EMBEDDED_TARGET_POKY ;;
	generic)
		abi_cv_embedded_target=EMBEDDED_TARGET_GENERIC ;;
	yes)
		abi_cv_embedded_target=EMBEDDED_TARGET_GENERIC 
		abi_cv_embedded_platform="generic" ;;
	no)
		;;
	*)
		AC_MSG_ERROR(bad value $enableval for --enable-embedded) ;;
	esac
],[
	abi_cv_embedded_platform="no"
	abi_cv_embedded_target="no"
])

AC_ARG_ENABLE([qt],
	[AS_HELP_STRING([--enable-qt], [Build using Qt4])],
[
	abi_cv_qt="yes"
],
[
	abi_cv_qt="no"
])

#
# setting the deps requirement is done further down
#

# cross platform deps
fribidi_req='fribidi >= 0.10.4'
glib_req='glib-2.0 >= 2.6.0 gthread-2.0 >= 2.6.0 gobject-2.0 >= 2.6.0'
gsf_req='libgsf-1 >= 1.14.18'
gsf_gio_req='libgsf-1 >= 1.14.9'
wv_req='wv-1.0 >= 1.2.0'
goffice_req='libgoffice-0.10 >= 0.10.0'
xslt_req='libxslt'
xp_pkgs="
  ${fribidi_req}
  ${glib_req}
  ${gsf_req}
  ${goffice_req}
  ${wv_req}
  ${xslt_req}
"

# optional deps
enchant_req='enchant >= 1.2.0'
gio_req='gio-2.0'

dnl libosso-gsf-1'
# placeholder for accumulated optional deps
opt_pkgs=''

# gtk deps
cairo_req='cairo-pdf cairo-ps pangocairo'
#
if test "x$abi_cv_gtk2" = "xyes"; then
gtk_req="gtk+-2.0 >= $GTK2_VERSION_REQ gtk+-unix-print-2.0 librsvg-2.0 >= 2.16.0"
else
gtk_req="gtk+-3.0 >= $GTK_VERSION_REQ gtk+-unix-print-3.0 librsvg-2.0 >= 2.16.0"
fi
dnl cairo-fc is needed but only available on cairo > 1.10
dnl http://bugzilla.abisource.com/show_bug.cgi?id=13265
PKG_CHECK_EXISTS(cairo >= 1.10, [gtk_req="$gtk_req cairo-fc"],[])
gtk_pkgs="
  ${cairo_req}
  ${gtk_req}
"

# qt4 deps
qt_pkgs="QtGui   ${cairo_req}"

# cocoa deps
cocoa_pkgs="$enchant_req $cairo_req"


# win32 deps
win_pkgs="$enchant_req"

#
# System tests
#

AC_PROG_CC
AC_PROG_CXX
#AC_PROG_OBJC
AC_PROG_INSTALL
# For libtool 1.5.x compatability (AC_PROG_LIBTOOL is deprecated version of LT_INIT)
m4_ifdef([LT_INIT], [LT_INIT([disable-static win32-dll])], [AC_LIBTOOL_WIN32_DLL])
m4_ifdef([LT_INIT], [], [AC_PROG_LIBTOOL])
AC_PROG_LN_S

AC_CHECK_PROG(HAVE_PERL, perl, yes, no)
if test "x$HAVE_PERL" = "xno"; then
	AC_MSG_ERROR([*** perl program not found])
fi


TOOLKIT_IS_GTK2="no"
AC_MSG_CHECKING([for platform and toolkit])
case ${host_os} in
  *darwin*)
    PLATFORM="unix"
    TOOLKIT="cocoa"
    ;;
  *mingw*)
    PLATFORM="win"
    TOOLKIT="win"
    ;;
  *)
    PLATFORM="unix"
    if test "x$abi_cv_qt" = "xyes"; then
      TOOLKIT="qt"
    else
      if test "x$abi_cv_gtk2" = "xyes"; then
        TOOLKIT_IS_GTK2="yes"
      fi
      TOOLKIT="gtk"
    fi
    ;;
esac
AC_MSG_RESULT([$PLATFORM / $TOOLKIT])
AC_SUBST([PLATFORM])
AC_DEFINE_UNQUOTED(PLATFORM, "$PLATFORM", The platform that is compiled for)
AC_SUBST([TOOLKIT])
AC_DEFINE_UNQUOTED(TOOLKIT, "$TOOLKIT", The toolkit that is used)

# Platform-specific tests

function_err='Your operating system or setup seems to be missing one or more required functions'
header_err='Your operating system or setup seems to be missing one or more required header files'

cocoa_funcs='floor mkdir'
unix_funcs='alarm gettimeofday'
win_funcs=
xp_funcs='localeconv strcspn strncasecmp strtoul' # "sqrt" test fails, hmm
funcs=

if test "$PLATFORM" = "cocoa"; then
	funcs="$cocoa_funcs $xp_funcs"
elif test "$PLATFORM" = "unix"; then
	AC_CHECK_HEADERS([sys/time.h], [], [$header_err])
	AC_FUNC_STRTOD
	AC_TYPE_UINT32_T
	funcs="$unix_funcs $xp_funcs"
elif test "$PLATFORM" = "win"; then
	funcs="$win_funcs $xp_funcs"
fi

AC_CHECK_FUNCS([ $funcs ], 
[], [
	AC_MSG_ERROR([$function_err])
])

#
# Features
#

AC_ARG_ENABLE([default-plugins], 
	[AS_HELP_STRING([--disable-default-plugins], [do not build any plugins by default])], 
[
	if test "$enableval" = "no"; then
		abi_cv_disable_default_plugins="yes"
	fi
])

m4_define([plugin_list], m4_include([plugin-list.m4]))

AC_ARG_ENABLE([plugins], 
	[AS_HELP_STRING([--enable-plugins], [="foo bar baz", list of plugins to build. Use --disable-plugins to disable plugin loading support, resulting in a smaller binary size. The list of plugins : ]m4_n([plugin_list]))], 
[
	if test "$enableval" = "no"; then
		abi_cv_disable_exports="yes"
		abi_cv_disable_default_plugins="yes"
		abi_cv_plugins=""
	elif test "$enableval" = "yes"; then
		# auto-detect which plugins can be built
		abi_cv_plugins="auto"
	else
		# custom plugins list
		abi_cv_plugins="$enableval"
	fi
])

AC_ARG_ENABLE([builtin-plugins], 
	[AS_HELP_STRING([--enable-builtin-plugins], [="foo bar baz", list of plugins to link statically])], 
[
	if test "$enableval" = "no"; then
		abi_cv_builtin_plugins=""
	elif test "$enableval" = "yes"; then
		# just build default plugins
		abi_cv_builtin_plugins=""
	else
		# custom plugins list
		abi_cv_builtin_plugins="$enableval"
	fi
])

AC_ARG_ENABLE([menubutton], 
	[AS_HELP_STRING([--enable-menubutton], [(Gtk+ only) menu-button instead of menu-bar])], 
[
	abi_cv_menubutton="$enableval"
],[
	abi_cv_menubutton="no"
])

AC_ARG_ENABLE([print], 
	[AS_HELP_STRING([--disable-print], [(Gtk+ only) do not include printing support])], 
[
	abi_cv_print="$enableval"
],[
	abi_cv_print="yes"
])

if test "$abi_cv_print" = "yes" &&
   test "$TOOLKIT" = "gtk"; then
	opt_pkgs="$opt_pkgs"
fi

AC_ARG_ENABLE([spell], 
	[AS_HELP_STRING([--disable-spell], [(Gtk+ only) do not include spell checking support])], 
[
	abi_cv_spell="$enableval"
],[
	# autodetect
	if test "$PLATFORM" = "unix"; then
		PKG_CHECK_EXISTS([ $enchant_req ],
		[
			abi_cv_spell="yes"
		], [
			abi_cv_spell="no"
		])
	else
		abi_cv_spell="yes"
	fi
])

if test "$abi_cv_spell" = "yes"; then
	opt_pkgs="$opt_pkgs $enchant_req"
fi

AC_ARG_ENABLE([statusbar], 
	[AS_HELP_STRING([--disable-statusbar], [(Gtk+ only) do not include status bar])], 
[
	abi_cv_statusbar="$enableval"
],[
	# on by default
	abi_cv_statusbar="yes"
])

AC_ARG_ENABLE([emacs-keybinding], 
	[AS_HELP_STRING([--enable-emacs-keybinding], [Enable the use of emacs-compatible keyboard commands])], 
[
	abi_cv_emacs_keybinding="$enableval"
], [
	abi_cv_emacs_keybinding="yes"
])

AC_ARG_ENABLE([vi-keybinding], 
	[AS_HELP_STRING([--enable-vi-keybinding], [Enable the use of vi-compatible keyboard commands])], 
[
	abi_cv_vi_keybinding="$enableval"
], [
	abi_cv_vi_keybinding="yes"
])

AC_ARG_ENABLE([clipart], 
	[AS_HELP_STRING([--enable-clipart], [Install clipart])], 
[
	abi_cv_clipart="$enableval"
], [
	abi_cv_clipart="no"
])

AC_ARG_ENABLE([templates], 
	[AS_HELP_STRING([--enable-templates], [Install additional templates])], 
[
	abi_cv_templates="$enableval"
], [
	abi_cv_templates="no"
])

AC_ARG_ENABLE([debug], 
	[AS_HELP_STRING([--enable-debug], [Enable debugging functionality, verbose terminal output])], 
[
	if test "$enableval" = "yes"; then
		abi_cv_debug="yes"
	fi
], [
	abi_cv_debug="no"
])

#
# Optional packages
#
if test "x$abi_cv_gtk2" = "xyes"; then
AC_ARG_WITH([gnomevfs], 	 
         [AS_HELP_STRING([--with-gnomevfs], [(Gtk+-2 only) use gnome-vfs library])], 	 
	[ 	 
	        abi_cv_gnomevfs="$withval" 	 
	],[ 	 
	        # use gnomevfs if detected 	 
	        if test "$TOOLKIT_IS_GTK2" = "yes"; then 	 
	                PKG_CHECK_EXISTS([ $gnomevfs_req ], 	 
	                [ 	 
	                        abi_cv_gnomevfs="yes" 	 
	                ], [ 	 
	                        abi_cv_gnomevfs="no" 	 
	                ]) 	 
	        else 	 
	                abi_cv_gnomevfs="no" 	 
	        fi 	 
	]) 	 
 	 
	if test "$abi_cv_gnomevfs" = "yes"; then 	 
	        opt_pkgs="$opt_pkgs $gnomevfs_req" 	 
	fi 	 
fi

AC_ARG_WITH([gio], 
	[AS_HELP_STRING([--with-gio], [use GIO library])], 
[
	abi_cv_gio="$withval"
],[
	# use gio if detected
	PKG_CHECK_EXISTS([ $gio_req ],
	[
		abi_cv_gio="yes"
	], [
		abi_cv_gio="no"
	])
])

if test "$abi_cv_gio" = "yes"; then
	opt_pkgs="$opt_pkgs $gio_req"
fi

AC_ARG_WITH([darwinports],
	[AS_HELP_STRING([--with-darwinports], [(Mac OSX only) add `/opt/local' prefix to CPP/LDFLAGS])], 
[
        if test "x$withval" != "xno"; then
            CPPFLAGS="$CPPFLAGS -I/opt/local/include"
            LDFLAGS="$LDFLAGS -headerpad_max_install_names -L/opt/local/lib"
        fi
])

AC_ARG_WITH([fink],
	[AS_HELP_STRING([--with-fink], [(Mac OSX only) add `/sw' prefix to CPP/LDFLAGS])], 
[
        if test "x$withval" != "xno"; then
                CPPFLAGS="$CPPFLAGS -I/sw/include"
                LDFLAGS="$LDFLAGS -L/sw/lib"
        fi
])

AC_ARG_WITH([abisdk],
	[AS_HELP_STRING([--with-abisdk], [(Mac OSX only) use the AbiSDK])], 
[
	if test "x$withval" = "x"; then
		AC_MSG_ERROR([The AbiSDK path must be specified.])
	fi
	if test "x$withval" != "xno"; then
		PKG_CONFIG=/opt/abi/bin/pkg-config
		PKG_CONFIG_PATH=/usr/lib/pkgconfig ; export PKG_CONFIG_PATH
		CPPFLAGS="$CPPFLAGS -mmacosx-version-min=10.4 -isysroot $withval -I/opt/abi/include"
		LDFLAGS="$LDFLAGS -mmacosx-version-min=10.4 -isysroot $withval -L/opt/abi/lib"
	fi
])


redland_req='redland >= 1.0.10 rasqal >= 0.9.17'
AC_ARG_WITH([redland], 
	[AS_HELP_STRING([--with-redland], [use redland and raptor libraries])], 
[
	abi_cv_redland="$withval"
],[
	# use redland if detected
	PKG_CHECK_EXISTS([ $redland_req ],
	[
		abi_cv_redland="yes"
	], [
		abi_cv_redland="no"
	])
])

if test "$abi_cv_redland" = "yes"; then
	opt_pkgs="$opt_pkgs $redland_req"
	AC_DEFINE([WITH_REDLAND], [1], [Using redland])
fi

AM_CONDITIONAL([WITH_REDLAND], test "$abi_cv_redland" = "yes")
AM_CONDITIONAL([HAVE_REDLAND], test "$abi_cv_redland" = "yes")


evolution_data_server_req='libebook-1.2 >= 3.6 libecal-1.2'
AC_ARG_WITH([evolution_data_server], 
	[AS_HELP_STRING([--with-evolution-data-server], [Use Evolution Data Server to get at contact and calendar information])], 
[
	abi_cv_evolution_data_server="$withval"
],[
	# use evolution_data_server if detected
	PKG_CHECK_EXISTS([ $evolution_data_server_req ],
	[
		abi_cv_evolution_data_server="yes"
	], [
		abi_cv_evolution_data_server="no"
	])
])

if test "$abi_cv_evolution_data_server" = "yes"; then
	opt_pkgs="$opt_pkgs $evolution_data_server_req"
	AC_DEFINE([WITH_EVOLUTION_DATA_SERVER], [1], [Using evolution_data_server])
fi

AM_CONDITIONAL([WITH_EVOLUTION_DATA_SERVER], test "$abi_cv_evolution_data_server" = "yes")
AM_CONDITIONAL([HAVE_EVOLUTION_DATA_SERVER], test "$abi_cv_evolution_data_server" = "yes")


libical_req=' libical >= 0.46 '
AC_ARG_WITH([libical], 
	[AS_HELP_STRING([--with-libical], [use libical and raptor libraries])], 
[
	abi_cv_libical="$withval"
],[
	# use libical if detected
	PKG_CHECK_EXISTS([ $libical_req ],
	[
		abi_cv_libical="yes"
	], [
		abi_cv_libical="no"
	])
])

if test "$abi_cv_libical" = "yes"; then
	opt_pkgs="$opt_pkgs $libical_req"
	AC_DEFINE([WITH_LIBICAL], [1], [Using libical])
fi

AM_CONDITIONAL([WITH_LIBICAL], test "$abi_cv_libical" = "yes")
AM_CONDITIONAL([HAVE_LIBICAL], test "$abi_cv_libical" = "yes")


champlain_req=' champlain-gtk-0.12 '
AC_ARG_WITH([champlain], 
	[AS_HELP_STRING([--with-champlain], [use champlain to display maps])],
	,[with_champlain="check"])

if test "$TOOLKIT" = "qt"; then
    with_champlain=no
fi

if test "x$with_champlain" != "xno"; then
    PKG_CHECK_MODULES(CHAMPLAIN, [ $champlain_req ],,
    [if test "x$with_champlain" = "xyes"; then
    	AC_MSG_FAILURE([--with-champlain was given, but test for champlain failed])
    fi])
    abi_cv_champlain=yes

	if test "$pkg_failed" = "no"; then
	#
	# try to make sure that champlain-gtk through pkg-config and
 	# the GTK+ version that the user wants abiword to directly
 	# link with match on major number.
	# 
	# The test is simple, make sure that a little client using the
	# GTK+ that champlain natively uses is using the GTK+ version
	# that we want to link with.
 	#
	AC_LANG_PUSH(C++)
	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CHAMPLAIN_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $CHAMPLAIN_LIBS "

	if test "x$abi_cv_gtk2" = "xyes"; then
 	  echo -n "checking whether CHAMPLAIN uses GTK+2... "
 	  
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <champlain-gtk/champlain-gtk.h>],[
 #if GTK_MAJOR_VERSION > 2
 #error bad
 #endif
                               ])], gtk_matches=yes, gtk_matches=no)
	else
 	  echo -n "checking whether CHAMPLAIN uses GTK+3... "
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <champlain-gtk/champlain-gtk.h>],[
 #if GTK_MAJOR_VERSION < 3
 #error bad
 #endif
                               ])], gtk_matches=yes, gtk_matches=no)
	fi
        echo "$gtk_matches"        

	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
        AC_LANG_POP

 	# Don't do mapping if their libchamplain is not using the right GTK+
	if test "$gtk_matches" = "no"; then
		if test "x$with_champlain" = "xyes"; then
			AC_MSG_FAILURE([--with-champlain was given, but champlain does not use the correct gtk version])
		fi
	   abi_cv_champlain=no
           CHAMPLAIN_CFLAGS=
           CHAMPLAIN_LIBS=
	fi
	else
		abi_cv_champlain=no
	fi
else
	abi_cv_champlain=no
fi

if test "$abi_cv_champlain" = "yes"; then
dnl	opt_pkgs="$opt_pkgs $champlain_req"
	AC_DEFINE([WITH_CHAMPLAIN], [1], [Using champlain])
fi

AC_SUBST([CHAMPLAIN_CFLAGS])
AC_SUBST([CHAMPLAIN_LIBS])
AM_CONDITIONAL([WITH_CHAMPLAIN], test "$abi_cv_champlain" = "yes")
AM_CONDITIONAL([HAVE_CHAMPLAIN], test "$abi_cv_champlain" = "yes")


AC_ARG_WITH([icondir],
	[AS_HELP_STRING([--with-icondir=DIR], [install icon in DIR instead of PREFIX/share/icons/hicolor/48x48/apps])],
[
	abi_cv_icondir="$withval"
])
if test "$abi_cv_icondir" = "" -o \
        "$abi_cv_icondir" = "yes" -o \
        "$abi_cv_icondir" = "no"; then
	abi_cv_prefix="$prefix"
	test "$abi_cv_prefix" = "NONE" && abi_cv_prefix="$ac_default_prefix"  
	abi_cv_icondir="${abi_cv_prefix}/share/icons/hicolor/48x48/apps"
fi
AC_SUBST(ABIWORD_ICONDIR, "$abi_cv_icondir") 	     


#
# Compiler settings (from gnumeric's configure.in)
#
# TODO enable -Wshadow
AC_LANG_PUSH(C++)
if test "$GCC" = "yes"; then
	for option in -Wall -Wextra -Wsign-compare -Wpointer-arith \
		      -Wchar-subscripts -Wwrite-strings -Wmissing-noreturn \
                      -Wunreachable-code \
		      -Wunused -Wpointer-arith -Wshadow; do
		SAVE_CPPFLAGS="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $option"
		AC_MSG_CHECKING([whether gcc understands $option])
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],
			has_option=yes,
			has_option=no)
		if test $has_option = no; then
		  CPPFLAGS="$SAVE_CPPFLAGS"
		fi
		AC_MSG_RESULT($has_option)
		unset has_option
		unset SAVE_CPPFLAGS
	done
	unset option

	for option in -Wno-overloaded-virtual; do
		SAVE_CXXFLAGS="$CXXFLAGS"
		CXXFLAGS="$CXXFLAGS $option"
		AC_MSG_CHECKING([whether gcc understands $option])
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],
			has_option=yes,
			has_option=no)
		if test $has_option = no; then
		  CXXFLAGS="$SAVE_CXXFLAGS"
		fi
		AC_MSG_RESULT($has_option)
		unset has_option
		unset SAVE_CXXFLAGS
	done
	unset option
fi
AC_LANG_POP

#
# Dependency tests
#

# We need libpng
for l in libpng libpng14 libpng12; do
  AC_MSG_CHECKING(for $l)
  if $PKG_CONFIG --exists $l ; then
    AC_MSG_RESULT(yes)
    PKG_CHECK_MODULES(PNG, $l, [abi_cv_png=yes],[abi_cv_png=no])
    break
  else
    AC_MSG_RESULT(no)
  fi
done
if test x$abi_cv_png != xyes; then
  AC_CHECK_HEADER([png.h], [], 
  [
    AC_MSG_ERROR([`png.h' not found, install libpng or specify CPPFLAGS to include custom locations])
  ])
  PNG_CFLAGS=
  PNG_LIBS=-lpng
fi

# We need ljpeg
AC_CHECK_HEADER(jpeglib.h,[
	AC_CHECK_LIB(jpeg,jpeg_read_header,
	[], 
	[
	  AC_MSG_ERROR([libjpeg not found])
	])
],[
	  AC_MSG_ERROR([jpeg.h not found])
])

# TODO need to check for libz too, at least on win32
AC_CHECK_HEADER([zlib.h], [], 
[
	AC_MSG_ERROR([`zlib.h' not found, install zlib or specify CPPFLAGS to include custom locations])
])

AX_BOOST_BASE([1.40.0],,[AC_MSG_ERROR([`boost' not found.])])

deps_pkgs="$xp_pkgs $opt_pkgs"
SYSTEM_LIBS=
SYSTEM_CFLAGS=
if test "$TOOLKIT" = "win"; then
	deps_pkgs="$deps_pkgs $win_pkgs"
	# nonstandard dlls go below the blank line
	SYSTEM_LIBS="-ladvapi32 -lcomctl32 -lcomdlg32 -lgdi32 -lkernel32 -lole32 -loleaut32 -lshell32 -luser32 -luuid -lversion -lwinspool \
		      $PNG_LIBS -lz -ljpeg"
	SYSTEM_CFLAGS="-mthreads"
elif test "$TOOLKIT" = "cocoa"; then
	deps_pkgs="$deps_pkgs $cocoa_pkgs"
	SYSTEM_LIBS="-framework Cocoa"
	AC_CHECK_PROG([CONVERT], [convert], convert, AC_MSG_ERROR(Cannot find ImageMagick convert))
elif test "$TOOLKIT" = "qt"; then
        deps_pkgs="$deps_pkgs $qt_pkgs"
else
	SYSTEM_CFLAGS=$PNG_CFLAGS
	SYSTEM_LIBS="$PNG_LIBS -ljpeg"
	deps_pkgs="$deps_pkgs $gtk_pkgs x11"
fi

PKG_CHECK_MODULES(DEPS,[$deps_pkgs])

dnl make that for all.
if test "$TOOLKIT" = "gtk" -o "$TOOLKIT" = "cocoa" -o "$TOOLKIT" = "qt"; then
	dnl move away
	AC_DEFINE([WITH_CAIRO], [1], [Using Cairo])
	WITH_CAIRO=1
fi
AM_CONDITIONAL([WITH_CAIRO], test "$WITH_CAIRO" = "1")

MOC=moc
if test "$TOOLKIT" = qt; then
   MOC=`pkg-config Qt --variable=moc`
fi
AC_SUBST([MOC])

AC_SUBST([DEPS_CFLAGS])
if test "$abi_cv_gio" = "yes"; then
	PKG_CHECK_MODULES(GSFGI, [$gsf_gio_req], [abi_cv_gsf_gio=yes], 
                 [abi_cv_gsf_gio=no])
	if test "$abi_cv_gsf_gio" = "yes" ; then
		AC_DEFINE([HAVE_GSF_GIO], [1], [GSF has GIO support])
	fi
else
	abi_cv_gsf_gio=no
fi

DEPS_LIBS="$SYSTEM_LIBS $DEPS_LIBS"
AC_SUBST([DEPS_LIBS])

BASE_CPPFLAGS_='
  ${DEPS_CFLAGS} 
  ${BOOST_CPPFLAGS}
  -I${top_srcdir} 
  ${GOFFICE_BUILTIN_CPPFLAGS}'
BASE_CPPFLAGS="$SYSTEM_CFLAGS"
for f in $(echo $BASE_CPPFLAGS_); do BASE_CPPFLAGS="$BASE_CPPFLAGS $f"; done
AC_SUBST([BASE_CPPFLAGS])

AF_CPPFLAGS_='
  ${BASE_CPPFLAGS} 
  -I${top_srcdir}/src/af/ev/${TOOLKIT} 
  -I${top_srcdir}/src/af/ev/xp 
  -I${top_srcdir}/src/af/gr/${TOOLKIT} 
  -I${top_srcdir}/src/af/gr/xp 
  -I${top_srcdir}/src/af/util/${PLATFORM} 
  -I${top_srcdir}/src/af/util/xp 
  -I${top_srcdir}/src/af/xap/${TOOLKIT} 
  -I${top_srcdir}/src/af/xap/xp'
AF_CPPFLAGS=
for f in $(echo $AF_CPPFLAGS_); do AF_CPPFLAGS="$AF_CPPFLAGS $f"; done
AC_SUBST([AF_CPPFLAGS])

AF_TEST_CPPFLAGS_='
  ${AF_CPPFLAGS} 
  -I${top_srcdir}/src/af/tf/xp'
AF_TEST_CPPFLAGS=
for f in $(echo $AF_TEST_CPPFLAGS_); do AF_TEST_CPPFLAGS="$AF_TEST_CPPFLAGS $f"; done
AC_SUBST([AF_TEST_CPPFLAGS])

TEXT_CPPFLAGS_='
  ${AF_CPPFLAGS} 
  -I${top_srcdir}/src/text/fmt/${TOOLKIT}
  -I${top_srcdir}/src/text/fmt/xp 
  -I${top_srcdir}/src/text/ptbl/xp'
TEXT_CPPFLAGS=
for f in $(echo $TEXT_CPPFLAGS_); do TEXT_CPPFLAGS="$TEXT_CPPFLAGS $f"; done
AC_SUBST([TEXT_CPPFLAGS])

TEXT_TEST_CPPFLAGS_='
  ${TEXT_CPPFLAGS} 
  -I${top_srcdir}/src/af/tf/xp'
TEXT_TEST_CPPFLAGS=
for f in $(echo $TEXT_TEST_CPPFLAGS_); do TEXT_TEST_CPPFLAGS="$TEXT_TEST_CPPFLAGS $f"; done
AC_SUBST([TEXT_TEST_CPPFLAGS])

IMPEXP_CPPFLAGS_='
  ${TEXT_CPPFLAGS} 
  -I${top_srcdir}/src/wp/impexp/${TOOLKIT} 
  -I${top_srcdir}/src/wp/impexp/xp'
IMPEXP_CPPFLAGS=
for f in $(echo $IMPEXP_CPPFLAGS_); do IMPEXP_CPPFLAGS="$IMPEXP_CPPFLAGS $f"; done
AC_SUBST([IMPEXP_CPPFLAGS])

IMPEXP_TEST_CPPFLAGS_='
  ${IMPEXP_CPPFLAGS} 
  -I${top_srcdir}/src/af/tf/xp'
IMPEXP_TEST_CPPFLAGS=
for f in $(echo $IMPEXP_TEST_CPPFLAGS_); do IMPEXP_TEST_CPPFLAGS="$IMPEXP_TEST_CPPFLAGS $f"; done
AC_SUBST([IMPEXP_TEST_CPPFLAGS])

WP_CPPFLAGS_='
  ${IMPEXP_CPPFLAGS} 
  -I${top_srcdir}/src/wp/ap/${TOOLKIT} 
  -I${top_srcdir}/src/wp/ap/xp
  -I${top_srcdir}/src/plugins'
WP_CPPFLAGS=
for f in $(echo $WP_CPPFLAGS_); do WP_CPPFLAGS="$WP_CPPFLAGS $f"; done
AC_SUBST([WP_CPPFLAGS])

#
# Settings
#

ABIWORD_SERIES="abi_series"
AC_DEFINE_UNQUOTED([ABIWORD_SERIES], [ "$ABIWORD_SERIES" ], [major.minor])
AC_SUBST(ABIWORD_SERIES)
AC_SUBST(ABIWORD_HEADERSDIR, "${includedir}/${PACKAGE_NAME}-${ABIWORD_SERIES}")
if test "$TOOLKIT" = "cocoa"; then
AC_SUBST(ABIWORD_CONTENTSDIR, "/AbiWord.app/Contents")
AC_SUBST(ABIWORD_DATADIR,    "${ABIWORD_CONTENTSDIR}/Resources")
AC_SUBST(ABIWORD_UIDIR,      "${ABIWORD_CONTENTSDIR}/Resources")
AC_SUBST(ABIWORD_MACOSDIR,   "${ABIWORD_CONTENTSDIR}/MacOS")
AC_SUBST(ABIWORD_PLUGINSDIR, "${ABIWORD_CONTENTSDIR}/PlugIns")
AC_SUBST(ABIWORD_LIBDIR,     "${ABIWORD_CONTENTSDIR}/Frameworks")
else
AC_SUBST(ABIWORD_DATADIR,    "${datadir}/${PACKAGE_NAME}-${ABIWORD_SERIES}")
AC_SUBST(ABIWORD_UIDIR,   "${ABIWORD_DATADIR}/ui")
AC_SUBST(ABIWORD_LIBDIR,     "${libdir}/${PACKAGE_NAME}-${ABIWORD_SERIES}")
AC_SUBST(ABIWORD_PLUGINSDIR, "${ABIWORD_LIBDIR}/plugins")
fi

# maybe just switch on TOOLKIT instead of the fancy TARGET defines
AM_CONDITIONAL([TOOLKIT_COCOA], test "$TOOLKIT" = "cocoa")
AM_CONDITIONAL([TOOLKIT_GTK], test "$TOOLKIT" = "gtk")
AM_CONDITIONAL([TOOLKIT_GTK2], test "$TOOLKIT_IS_GTK2" = "yes")
AM_CONDITIONAL([TOOLKIT_QT], test "$TOOLKIT" = "qt")
AM_CONDITIONAL([TOOLKIT_WIN], test "$TOOLKIT" = "win")

if test "$enable_shared" = ""; then
	enable_shared="no"
fi

if test "$enable_static" = ""; then
	enable_static="no"
fi

if test "$TOOLKIT" = "win"; then
	AC_DEFINE([TOOLKIT_WIN], [1], [Build win32 user interface])
	case ${host} in
	  *-w64-mingw*)
		AC_DEFINE([UNICODE], [1], [Build a full unicode AbiWord])
		;;
	  *)
dnl on Mingw32 it is defined.
		AC_DEFINE([_WIN32_IE], [0x0501], [minimal comctl.dll v4.70 for toolbars])
		AC_DEFINE([_WIN32_WINNT], [0x0500], [support collaboration plugin in Windows 2000 or higher])
	  	;;
	esac
elif test "$TOOLKIT" = "cocoa"; then
	AC_DEFINE([TOOLKIT_COCOA], [1], [Build cocoa user interface])
	# override static/shared, on cocoa. shared only
	enable_shared="yes"
	enable_static="no"
elif test "$TOOLKIT" = "gtk"; then
     if test "$TOOLKIT_IS_GTK2" = "yes"; then
	AC_DEFINE([TOOLKIT_GTK2], [1], [Build gtk+ user interface])
	AC_DEFINE([TOOLKIT_GTK_ALL], [1], [Build gtk+ user interface])
     else
	AC_DEFINE([TOOLKIT_GTK], [1], [Build gtk+ user interface])
	AC_DEFINE([TOOLKIT_GTK_ALL], [1], [Build gtk+ user interface])
dnl We have no reason to mess with that.
dnl	# build static binary by default
dnl	if test "$enable_shared" = "no" && \
dnl	   test "$enable_static" = "no"; then 
dnl		enable_shared="no"
dnl		enable_static="yes"
dnl	fi
     fi
elif test "$TOOLKIT" = "qt"; then
     AC_DEFINE([TOOLKIT_QT], [1], [Build using Qt4])
fi
AM_CONDITIONAL([ENABLE_DYNAMIC], test "$enable_shared" = "yes")
AM_CONDITIONAL([ENABLE_STATIC], test "$enable_static" = "yes")

PKG_CHECK_MODULES(PLUGIN,[$glib_req])
PLUGIN_CFLAGS="$PLUGIN_CFLAGS "'${WP_CPPFLAGS} -DABI_DLL'
if test "$TOOLKIT" = "gtk"  || test "$TOOLKIT" = "qt" && 
   test "$enable_shared" = "yes"; then
	# link plugins to work around gcc visibility issue with 
	# derived classes in dlopened modules
	PLUGIN_LIBS='${top_builddir}/src/libabiword-'"$ABIWORD_SERIES"'.la'" $PLUGIN_LIBS"
elif test "$TOOLKIT" = "win"; then
	PLUGIN_CFLAGS="$PLUGIN_CFLAGS "'-D_WIN32_IE=0x0501 -D_WIN32_WINNT=0x0500'
	PLUGIN_LIBS='${top_builddir}/src/libabiword-'"$ABIWORD_SERIES"'.la'" $PLUGIN_LIBS"
elif test "$TOOLKIT" = "cocoa" ; then 
	PLUGIN_LIBS='${top_builddir}/src/libabiword-'"$ABIWORD_SERIES"'.la'" $PLUGIN_LIBS"
fi
AC_SUBST([PLUGIN_CFLAGS])
AC_SUBST([PLUGIN_LIBS])

if test "$abi_cv_disable_exports" = "yes"; then
	AC_DEFINE([DISABLE_EXPORTS], [1], [Define to prevent symbols from being exported dynamically])
fi

if test "$abi_cv_menubutton" = "yes"; then
	AC_DEFINE([ENABLE_MENUBUTTON], [1], [Define if building menu button support])
fi
AM_CONDITIONAL([ENABLE_MENUBUTTON], test "$abi_cv_menubutton" = "yes")

if test "$abi_cv_print" = "yes"; then
	AC_DEFINE([ENABLE_PRINT], [1], [Define if building printing support])
fi
AM_CONDITIONAL([ENABLE_PRINT], test "$abi_cv_print" = "yes")

if test "$abi_cv_spell" = "yes"; then
	AC_DEFINE([ENABLE_SPELL], [1], [Define if building spell checking support])
	# TODO get rid of this after all platforms are using enchant exclusively
	AC_DEFINE([WITH_ENCHANT], [1], [use Dom's enchanting spell checker abstraction library])
fi
AM_CONDITIONAL([ENABLE_SPELL], test "$abi_cv_spell" = "yes")

if test "$abi_cv_statusbar" = "yes"; then
	AC_DEFINE([ENABLE_STATUSBAR], [1], [Define if building status bar])
fi
AM_CONDITIONAL([ENABLE_STATUSBAR], test "$abi_cv_statusbar" = "yes")

if test "$abi_cv_embedded_target" != "no"; then
	AC_DEFINE_UNQUOTED(EMBEDDED_TARGET, $abi_cv_embedded_target, [Whether we are building for embedded device])
fi

if test "$abi_cv_emacs_keybinding" = "yes"; then
	AC_DEFINE([ENABLE_EMACS_KEYBINDING], [1], [Define to enable include emacs-compatible keyboard commands])
fi
AM_CONDITIONAL(ENABLE_EMACS_KEYBINDING, test "$abi_cv_emacs_keybinding" = "yes")

if test "$abi_cv_vi_keybinding" = "yes"; then
	AC_DEFINE([ENABLE_VI_KEYBINDING], [1], [Define to enable include vi-compatible keyboard commands])
fi
AM_CONDITIONAL(ENABLE_VI_KEYBINDING, test "$abi_cv_vi_keybinding" = "yes")

AM_CONDITIONAL(ENABLE_CLIPART, test "$abi_cv_clipart" = "yes")
AM_CONDITIONAL(ENABLE_TEMPLATES, test "$abi_cv_templates" = "yes")

if test "$abi_cv_debug" = "yes"; then
	AC_DEFINE([DEBUG], [1], [Define to enable debugging functionality])
else
	AC_DEFINE([NDEBUG], [1], [Define to disable debugging functionality])
fi
AM_CONDITIONAL(DEBUG, test "$abi_cv_debug" = "yes")

#
# Optional dependencies handling
#
AM_PATH_LIBGCRYPT( 1.4.5,
		   [
		   abi_cv_gcrypt=no
 		   AC_DEFINE([HAVE_GCRYPT], [1], [Use gcrypt for the cryptos])
		   ],
		   [
		   abi_cv_gcrypt=yes
		   ] )
AM_CONDITIONAL(HAVE_GCRYPT, test "$abi_cv_gcrypt" = "yes")

if test "$abi_cv_gnomevfs" = "yes"; then 	 
    AC_DEFINE([WITH_GNOMEVFS], [1], [Define if using gnome-vfs]) 	 
fi 	 
	
if test "$abi_cv_gio" = "yes"; then
	AC_DEFINE([WITH_GIO], [1], [Define if using GIO])
fi

if test "$abi_cv_goffice" = "yes"; then
	AC_DEFINE([WITH_GOFFICE], [1], [Define if using goffice])
fi
AM_CONDITIONAL([TOOLKIT_GTK_ALL], test "$TOOLKIT" = "gtk")
AM_CONDITIONAL([WITH_GOFFICE_BUILTIN], test "$TOOLKIT" = "gtk" && test "$abi_cv_goffice" != "yes")
AC_SUBST([GOFFICE_BUILTIN_LIBS])

ALL_PLUGINS_="\
  m4_n([plugin_list])"
# strip trailing newline from the included file, it messes up sed on windows
ALL_PLUGINS=
for p in $(echo $ALL_PLUGINS_); do ALL_PLUGINS="$ALL_PLUGINS $p"; done
AC_SUBST([ALL_PLUGINS])

PLUGINS=
if test "$abi_cv_plugins" = "auto"; then

	# which plugins to auto-enable
	# this creates variables like "enable_wordperfect=auto"
	for plugin in `echo $ALL_PLUGINS`; do
		eval $(echo "enable_$plugin")="auto"
	done
else
	default_plugins="opendocument openxml"

	if test "$abi_cv_disable_default_plugins" != "yes"; then
		PLUGINS="$default_plugins"
	fi
	PLUGINS="$PLUGINS $abi_cv_plugins"

	# which plugins to enable
	# this creates variables like "enable_wordperfect=yes"
	for plugin in `echo $PLUGINS`; do
		eval $(echo "enable_$plugin")="yes"
	done
fi

# enable the plugins we want to link statically
# this creates variables like "enable_wordperfect=yes"
BUILTIN_PLUGINS="$abi_cv_builtin_plugins"
for plugin in `echo $BUILTIN_PLUGINS`; do
	eval $(echo "enable_$plugin")="yes"
done

# which plugins to build in
# this creates variables like "enable_wordperfect_builtin=yes"
for plugin in `echo $BUILTIN_PLUGINS`; do
	variable="enable_"$plugin"_builtin"
	eval $(echo $variable)="yes"
done

# this creates the list of .la files to link statically
# used in src/plugins/Makefile.am
BUILTIN_PLUGINS_LA=
for plugin in `echo $BUILTIN_PLUGINS`; do
	BUILTIN_PLUGINS_LA="$BUILTIN_PLUGINS_LA ../../plugins/$plugin/$plugin.la"
done

m4_include([plugin-builtin.m4])

m4_include([plugin-configure.m4])

AC_SUBST([PLUGINS])
AC_SUBST([BUILTIN_PLUGINS])
AC_SUBST([BUILTIN_PLUGINS_LA])

# put all deps on a single line for the .pc file
ABIWORD_REQUIRED_PKGS=
for dep in `echo $deps_pkgs`; do
	ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS $dep"
done
AC_SUBST(ABIWORD_REQUIRED_PKGS)

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

dnl Introspection support
if test "$TOOLKIT" = "gtk"; then
GIR_REQ=1.0.0
AC_ARG_ENABLE(introspection,
          AS_HELP_STRING([--enable-introspection[=@<:@no/auto/yes@:>@]],
                         [Enable introspection for this build]),, 
                         [enable_introspection=no])

AC_MSG_CHECKING([for gobject-introspection])

dnl presence/version checking
AS_CASE([$enable_introspection],
[no], [
    found_introspection="no (disabled, use --enable-introspection to enable)"
],
[yes],[
    PKG_CHECK_EXISTS([gobject-introspection-1.0],,
                     AC_MSG_ERROR([gobject-introspection-1.0 is not installed]))
    PKG_CHECK_EXISTS([gobject-introspection-1.0 >= $GIR_REQ],
                     found_introspection=yes,
                     AC_MSG_ERROR([You need to have gobject-introspection >= $1 installed to build AC_PACKAGE_NAME]))
],
[auto],[
    PKG_CHECK_EXISTS([gobject-introspection-1.0 >= $GIR_REQ], found_introspection=yes, found_introspection=no)
dnl Canonicalize enable_introspection
enable_introspection=$found_introspection
],
[
    AC_MSG_ERROR([invalid argument passed to --enable-introspection, should be one of @<:@no/auto/yes@:>@])
])

AC_MSG_RESULT([$found_introspection])

INTROSPECTION_SCANNER=
INTROSPECTION_COMPILER=
INTROSPECTION_GENERATE=
INTROSPECTION_GIRDIR=
INTROSPECTION_TYPELIBDIR=
if test "x$found_introspection" = "xyes"; then
   AM_PATH_PYTHON
   INTROSPECTION_SCANNER=`$PKG_CONFIG --variable=g_ir_scanner gobject-introspection-1.0`
   INTROSPECTION_COMPILER=`$PKG_CONFIG --variable=g_ir_compiler gobject-introspection-1.0`
   INTROSPECTION_GENERATE=`$PKG_CONFIG --variable=g_ir_generate gobject-introspection-1.0`
   INTROSPECTION_GIRDIR=`$PKG_CONFIG --variable=girdir gobject-introspection-1.0`
   INTROSPECTION_TYPELIBDIR="$($PKG_CONFIG --variable=typelibdir gobject-introspection-1.0)"
   INTROSPECTION_CFLAGS=`$PKG_CONFIG --cflags gobject-introspection-1.0`
   INTROSPECTION_LIBS=`$PKG_CONFIG --libs gobject-introspection-1.0`
   INTROSPECTION_MAKEFILE=`$PKG_CONFIG --variable=datadir gobject-introspection-1.0`/gobject-introspection-1.0/Makefile.introspection
fi
AC_SUBST(INTROSPECTION_SCANNER)
AC_SUBST(INTROSPECTION_COMPILER)
AC_SUBST(INTROSPECTION_GENERATE)
AC_SUBST(INTROSPECTION_GIRDIR)
AC_SUBST(INTROSPECTION_TYPELIBDIR)
AC_SUBST(INTROSPECTION_CFLAGS)
AC_SUBST(INTROSPECTION_LIBS)
AC_SUBST(INTROSPECTION_MAKEFILE)

ABI_SUB_SERIES="abi_version_major"_"abi_version_minor"
AC_SUBST(ABI_SUB_SERIES)

dnl we need to change the install directories for distcheck
AC_ARG_WITH([gir-dir],
	AS_HELP_STRING(
		[--with-gir-dir],
		[
		 path to gir repository
		 (automatically detected via pkg-config)
		]
	),
	[GIRDIR=$withval],
	[GIRDIR=$INTROSPECTION_GIRDIR]
)
AC_SUBST(GIRDIR)
AC_ARG_WITH([typelib-dir],
	AS_HELP_STRING(
		[--with-typelib-dir],
		[
		 path to typelibs repository
		 (automatically detected via pkg-config)
		]
	),
	[TYPELIBDIR=$withval],
	[TYPELIBDIR=$INTROSPECTION_TYPELIBDIR]
)
AC_SUBST(TYPELIBDIR)
fi

AM_CONDITIONAL(HAVE_INTROSPECTION, test "x$found_introspection" = "xyes")

AC_OUTPUT(
  libabiword.pc
  Makefile
  po/Makefile
  src/Makefile
  src/af/Makefile
  src/af/ev/Makefile
  src/af/ev/cocoa/Makefile
  src/af/ev/gtk/Makefile
  src/af/ev/qt/Makefile
  src/af/ev/win/bmp/Makefile
  src/af/ev/win/Makefile
  src/af/ev/xp/Makefile
  src/af/gr/Makefile
  src/af/gr/cocoa/Makefile
  src/af/gr/gtk/Makefile
  src/af/gr/qt/Makefile
  src/af/gr/win/Makefile
  src/af/gr/xp/Makefile
  src/af/tf/Makefile
  src/af/tf/xp/Makefile
  src/af/util/Makefile
  src/af/util/unix/Makefile
  src/af/util/win/Makefile
  src/af/util/xp/Makefile
  src/af/util/xp/t/Makefile
  src/af/xap/Makefile
  src/af/xap/cocoa/Makefile
  src/af/xap/gtk/Makefile
  src/af/xap/gtk/t/Makefile
  src/af/xap/qt/Makefile
  src/af/xap/win/Makefile
  src/af/xap/xp/Makefile
  src/gi-overrides/Makefile
  src/plugins/Makefile
  src/text/Makefile
  src/text/fmt/Makefile
  src/text/fmt/gtk/Makefile
  src/text/fmt/xp/Makefile
  src/text/fmt/xp/t/Makefile
  src/text/ptbl/Makefile
  src/text/ptbl/xp/Makefile
  src/text/ptbl/xp/t/Makefile
  src/wp/Makefile
  src/wp/ap/Makefile
  src/wp/ap/cocoa/Makefile
  src/wp/ap/gtk/Makefile
  src/wp/ap/qt/Makefile
  src/wp/ap/win/Makefile
  src/wp/ap/xp/Makefile
  src/wp/ap/xp/ToolbarIcons/Makefile
  src/wp/impexp/Makefile
  src/wp/impexp/cocoa/Makefile
  src/wp/impexp/gtk/Makefile
  src/wp/impexp/qt/Makefile
  src/wp/impexp/win/Makefile
  src/wp/impexp/xp/Makefile
  src/wp/test/Makefile
  src/wp/test/cocoa/Makefile
  src/wp/test/unix/Makefile
  src/wp/test/unix/testwrap.sh
  src/wp/test/win/Makefile
  src/wp/test/win/testwrap.sh
  src/wp/test/xp/Makefile
  src/wp/main/Makefile
  src/wp/main/cocoa/Makefile
  src/wp/main/cocoa/bundle/Info.plist
  src/wp/main/win/Makefile
  user/Makefile
  user/wp/Makefile
  user/wp/clipart/Makefile
  user/wp/dictionary/Makefile
  user/wp/templates/Makefile
  tools/Makefile
  tools/cdump/Makefile
  tools/cdump/xp/Makefile
)

AC_OUTPUT(
  m4_include([plugin-makefiles.m4])
)

echo "
Configuration:
  host                  ${host}
  dynamic binary        ${enable_shared}
  static binary         ${enable_static}
  platform              ${PLATFORM} (embedded: $abi_cv_embedded_platform)
  toolkit               ${TOOLKIT}
  debug                 ${abi_cv_debug}

Optional features:
  menubutton            ${abi_cv_menubutton}
  printing              ${abi_cv_print}
  spell checking        ${abi_cv_spell}
  status bar            ${abi_cv_statusbar}
  emacs keybinding      ${abi_cv_emacs_keybinding}
  vi keybinding         ${abi_cv_vi_keybinding}
  clipart               ${abi_cv_clipart}
  templates             ${abi_cv_templates}

Optional dependencies:
  gio			${abi_cv_gio}
  gsf-gio               ${abi_cv_gsf_gio}
  redland               ${abi_cv_redland}
  evolution_data_server ${abi_cv_evolution_data_server}
  libical               ${abi_cv_libical}
  champlain             ${abi_cv_champlain}

Builtin plugins         ${BUILTIN_PLUGINS}
Plugins                 ${PLUGINS}
"

