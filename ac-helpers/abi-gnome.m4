# start: abi/ac-helpers/abi-gnome.m4
# 
# Copyright (C) 2002 AbiSource, Inc
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
# Usage: ABI_GNOME_QUICK

AC_DEFUN([ABI_GNOME_QUICK], [

dnl Quick&Easy GNOME Detection

GNOME_CFLAGS=""
GNOME_LIBS=""

AC_ARG_ENABLE(gnome,[  --enable-gnome    Turn on gnome ],[
	case "${enableval}" in
	 yes)	if test "$PLATFORM" = unix; then
			gnome=true
		else
			AC_MSG_ERROR([sorry: --enable-gnome supported only on UNIX platforms])
			gnome=false
		fi
		;;
	  no)	gnome=false ;;
	   *)	AC_MSG_ERROR(bad value ${enableval} for --enable-gnome) ;;
	esac
],[	gnome=false
])

if test "$gnome" = true ; then
	AC_MSG_CHECKING(for gnome-libs >= 2.0.0)
	if pkg-config --modversion libgnome-2.0 2> /dev/null; then 
	    dnl We need the "%d" in order not to get e-notation on hpux.
	    vers=`pkg-config --modversion libgnome-2.0 | awk 'BEGIN { FS = "."; } { printf "%d", ([$]1 * 1000 + [$]2) * 1000 + [$]3;}'`
	    if test "$vers" -ge 2000000; then
	        AC_MSG_RESULT(found)
	    else
	       AC_MSG_RESULT(You need at least gnome-libs 2.0.0: disabling gnome)
               gnome=false
	    fi
	else
	    AC_MSG_RESULT(not found: disabling gnome)
            gnome=false
        fi
fi

if test "$gnome" = true ; then
	gnomelibs="libbonobo-2.0 libgnomeui-2.0 gal-2.0 libgnomeprint-2.0 gnome-vfs-2.0 gdk-pixbuf-2.0"
	gnomeliberrors=`pkg-config --cflags $gnomelibs 2>&1 | grep "Unknown library"`
	if test "x$gnomeliberrors" != "x"; then
		AC_MSG_ERROR([One or more gnome libraries not found; require: $gnomelibs])
	fi
fi

if test "$gnome" = true ; then
	dnl What is the minimum gal library we can use?
	AC_MSG_CHECKING(for gal2 >= 0.0.5)
	if pkg-config --modversion gal-2.0; then 
	    dnl We need the "%d" in order not to get e-notation on hpux.
	    vers=`pkg-config --modversion gal-2.0 | awk 'BEGIN { FS = ".0."; } { printf "%d", [$]1 * 1000 + [$]2;}'`
	    if test "$vers" -ge 5; then
	        AC_MSG_RESULT(found)
	    else
	       AC_MSG_RESULT(You need at least gal 0.5: disabling gnome)
               gnome=false
	    fi
	else
	    AC_MSG_RESULT(not found: disabling gnome)
            gnome=false
        fi
fi

if test "$gnome" = true ; then
	GNOME_CFLAGS="`pkg-config --cflags $gnomelibs` -DHAVE_GNOME=1"
	GNOME_LIBS="`pkg-config --libs $gnomelibs`"

	AC_PATH_PROG(NAUTILUS_CONFIG,nautilus-config, ,[$PATH])
	if test "x$NAUTILUS_CONFIG" != "x"; then
		GNOME_CFLAGS="`$NAUTILUS_CONFIG --cflags` $GNOME_CFLAGS"
		GNOME_LIBS="`$NAUTILUS_CONFIG --libs` $GNOME_LIBS"
	fi
fi

AC_SUBST(GNOME_CFLAGS)
AC_SUBST(GNOME_LIBS)
	
AM_CONDITIONAL(WITH_GNOME, test "x$gnome" = "xtrue")

])
# 
# end: abi/ac-helpers/abi-gnome.m4
# 
