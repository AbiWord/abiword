
# On unix, Check for glib and gtk libraries.  Ensure that the version
# number of glib is >= 1.2.0 and the version number of gtk is >= 1.2.2

# Usage: 
# ABI_GLIB_GTK
#

AC_DEFUN([ABI_GLIB_GTK], [

if test "$OS_NAME" = "FreeBSD"; then
	GLIB_CONFIG=glib12-config
	GTK_CONFIG=gtk12-config
else
	GLIB_CONFIG=glib-config
	GTK_CONFIG=gtk-config
fi

if test "$PLATFORM" = "unix"; then
        dnl ******************************
        dnl glib checking
        dnl ******************************
        AC_MSG_CHECKING(for glib >= 1.2.0)
        if $GLIB_CONFIG --version > /dev/null 2>&1; then 
            dnl We need the "%d" in order not to get e-notation on hpux.
            vers=`$GLIB_CONFIG --version | awk 'BEGIN { FS = "."; } { printf "%d", ($[1] * 1000 + $[2]) * 1000 + $[3];}'`
            if test "$vers" -ge 1002000; then
                AC_MSG_RESULT(found)
            else
	        AC_MSG_ERROR(You need at least glib 1.2.0 for this version of AbiWord)
            fi
        else
            AC_MSG_ERROR(Did not find glib installed)
        fi

        dnl AM_PATH_GLIB(1.2.0)
        GMODULE_CFLAGS=`$GLIB_CONFIG --cflags gmodule`
        GMODULE_LIBS=`$GLIB_CONFIG --libs gmodule`

        dnl ******************************
        dnl gtk+ checking
        dnl ******************************
        AC_MSG_CHECKING(for GTK >= 1.2.2)
        if $GTK_CONFIG --version > /dev/null 2>&1; then 
            dnl We need the "%d" in order not to get e-notation on hpux.
            vers=`$GTK_CONFIG --version | awk 'BEGIN { FS = "."; } { printf "%d", ($[1] * 1000 + $[2]) * 1000 + $[3];}'`
            if test "$vers" -ge 1002002; then
                AC_MSG_RESULT(found)
            else
                AC_MSG_ERROR(You need at least GTK+ 1.2.2 for this version of AbiWord)
            fi
        else
            AC_MSG_ERROR(Did not find GTK+ installed)
        fi

        GTK_CFLAGS=`$GTK_CONFIG --cflags`
        GTK_LIBS=`$GTK_CONFIG --libs`

        AC_SUBST(GTK_CFLAGS)
        AC_SUBST(GTK_LIBS)
        AC_SUBST(GMODULE_CFLAGS)
        AC_SUBST(GMODULE_LIBS)
fi

])
