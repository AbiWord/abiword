# start: abi/ac-helpers/abi-embedded.m4
# 
# Copyright (C) 2007 Tomas Frydrych
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
# Usage: ABI_EMBEDDED

AC_DEFUN([ABI_EMBEDDED], [

# We define EMBEDDED_TARGET to a numerical value so that we can do like
# #if EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#
# The following have to be defined unconditionally, because we want 
# EMBEDDED_TARGET to be undefined if not building for embedded so that we 
# can use #ifdef and #ifndef on it (two undefined symbols will return 
# true if tested for equality in #if construct)
AC_DEFINE(EMBEDDED_TARGET_GENERIC, 1)
AC_DEFINE(EMBEDDED_TARGET_HILDON,  2)
AC_DEFINE(EMBEDDED_TARGET_POKY,    3)

embedded=none
embedded_print=undefined
embedded_statusbar=false
embedded_menubutton=true

EMBEDDED_CFLAGS=""
EMBEDDED_LIBS=""
HILDON_CFLAGS=""
HILDON_LIBS=""

AC_ARG_ENABLE(embedded,[  --enable-embedded=generic|hildon|poky    Include embedded features],[
   if test "${enableval}" != "no" ; then
      if test "$PLATFORM" != unix; then
	AC_MSG_ERROR([--enable-embedded available only on UNIX platforms])
      fi
   fi
   case "${enableval}" in
	hildon) 
	  PKG_CHECK_MODULES(HILDON, hildon-1 hildon-fm-2 dbus-1 libosso, [], [AC_MSG_ERROR([Hildon dependencies not satisfied.])])
	  EMBEDDED_CFLAGS="$EMBEDDED_CFLAGS $HILDON_CFLAGS"
	  EMBEDDED_LIBS="$EMBEDDED_LIBS $HILDON_LIBS"
	  embedded=EMBEDDED_TARGET_HILDON
	  embedded_print=false;;
	poky)
	  embedded_print=false
	  embedded=EMBEDDED_TARGET_POKY ;;
	generic)
	  embedded_print=false
	  embedded=EMBEDDED_TARGET_GENERIC ;;
	yes)	
	  embedded_print=false
	  embedded=EMBEDDED_TARGET_GENERIC ;;
	no)
	  embedded=none ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-embedded) ;;
	esac
],[	
	embedded=none
])

if test "$embedded" != "none"; then
   AC_DEFINE_UNQUOTED(EMBEDDED_TARGET, $embedded, [Whether we are building for embedded device])

   if test "$embedded_menubutton" = "true"; then
      AC_DEFINE(ENABLE_MENUBUTTON, 1, [Whether to use menubutton instead of menubar])
   fi
else
   # make sure we do not prevent statusbar from building
   embedded_statusbar=true
fi

if test "$embedded_statusbar" = "true"; then
   AC_DEFINE(ENABLE_STATUSBAR, 1, [Whether to include statusbar])
fi

AM_CONDITIONAL(EMBEDDED_HILDON, test "x$embedded" = "xEMBEDDED_TARGET_HILDON")
AM_CONDITIONAL(ENABLE_STATUSBAR, test "x$embedded_statusbar" = "xtrue")

AC_SUBST(EMBEDDED_CFLAGS)
AC_SUBST(EMBEDDED_LIBS)
])
# 
# end: abi/ac-helpers/abi-embedded.m4
# 
