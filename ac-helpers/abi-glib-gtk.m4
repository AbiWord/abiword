# test for glib2
AC_DEFUN([ABI_GLIB2],[

	PKG_CHECK_MODULES(GLIB,glib-2.0 >= 2.0,[
	dnl do we need to set these?
		abi_glib2=yes
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS glib-2.0 >= 2.0"
	],[	abi_glib2=no
	])
	if test $abi_glib2 = no; then
		AC_MSG_ERROR([$GLIB_PKG_ERRORS])
	fi

	PKG_CHECK_MODULES(GMODULE,gmodule-2.0 >= 2.0,[
	dnl do we need to set these?
		abi_gmodule2=yes
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS gmodule-2.0 >= 2.0"
	],[	abi_gmodule2=no
	])
	if test $abi_gmodule2 = no; then
		AC_MSG_ERROR([$GMODULE_PKG_ERRORS])
	fi

	PKG_CHECK_MODULES(GTHREAD,gthread-2.0,[
	dnl do we need to set these?
		abi_gthread2=yes
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS gthread-2.0"
	],[	abi_gthread2=no
	])
	if test $abi_gthread2 = no; then
		AC_MSG_ERROR([$GTHREAD_PKG_ERRORS])
	else
		THREAD_CFLAGS="-DHAVE_THREADS=1 $GTHREAD_CFLAGS"
		THREAD_LIBS=$GTHREAD_LIBS
	fi

	AC_SUBST(THREAD_CFLAGS)
	AC_SUBST(THREAD_LIBS)

	AM_CONDITIONAL(HAVE_THREADS, test $abi_gthread2 = yes)
])

# test for gtk2, pango-xft and libglade-2.0
AC_DEFUN([ABI_GTK2],[
	ABI_GLIB2

	PKG_CHECK_MODULES(GTK,[
		gtk+-2.0 >= 2.6.0
		pangoxft >= 1.2.0
		libglade-2.0 >= 2.0.0
	],[	
		abi_gtk2=yes
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS gtk+-2.0 >= 2.6.0 pangoxft >= 1.2.0 libglade-2.0 >= 2.0.0"
	],[	abi_gtk2=no
	])
	if test $abi_gtk2 = no; then
		AC_MSG_ERROR([$GTK_PKG_ERRORS])
	fi

	PKG_CHECK_MODULES(PANGOFT2,[
		pangoft2 >= 1.2.0
	],[	
		pango_ft2=yes
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS pangoft2 >= 1.2.0"
	],[	pango_ft2=no
	])

	if test $pango_ft2 = yes; then
		GTK_CFLAGS="$GTK_CFLAGS $PANGOFT2_CFLAGS"
		GTK_LIBS="$GTK_LIBS $PANGOFT2_LIBS"

		AC_DEFINE(HAVE_PANGOFT2, 1, [Define if you have PangFT2])
	fi

	dnl since gtk+ doesn't add the X libraries into its dependency list we
	dnl need to add them here
	dnl 
	AC_PATH_X
	AC_PATH_XTRA

	GTK_LIBS="$GTK_LIBS $X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS"
	AC_SUBST(GTK_LIBS)
	AC_SUBST(GTK_CFLAGS)
])

# Check for optional glib

# Usage: 
# ABI_GLIB12_OPT(<micro-version>,<optional>) where <optional> = "no"|"yes"

AC_DEFUN([ABI_GLIB12_OPT], [	
	if [ test "x$2" = "xyes" ]; then
		abi_glib_opt=check
	else
		abi_glib_opt=required
	fi
	AC_ARG_WITH(glib,[  --with-glib[=DIR]  Use glib (v1.2) [in DIR] ],[
		if [ test "x$withval" = "xno" ]; then
			if [ test $abi_glib_opt = required ]; then
				AC_MSG_ERROR([* * * glib-1.2 is not optional! * * *])
			fi
			abi_glib_opt=no
		elif [ test "x$withval" = "xyes" ]; then
			abi_glib_opt=required
			GLIB_DIR=""
		else
			abi_glib_opt=required
			GLIB_DIR="$withval"
		fi
	],[	GLIB_DIR=""
	])
	if [ test $abi_glib_opt != no ]; then
		if [ test "x$GLIB_DIR" = "x" ]; then
			AC_PATH_PROG(GLIB_CONFIG,glib12-config, ,[$PATH])
		else
			AC_PATH_PROG(GLIB_CONFIG,glib12-config, ,[$GLIB_DIR/bin:$PATH])
		fi
		if [ test "x$GLIB_CONFIG" = "x" ]; then
			if [ test "x$GLIB_DIR" = "x" ]; then
				AC_PATH_PROG(GLIB_CONFIG,glib-config, ,[$PATH])
			else
				AC_PATH_PROG(GLIB_CONFIG,glib-config, ,[$GLIB_DIR/bin:$PATH])
			fi
		fi
		if [ test "x$GLIB_CONFIG" = "x" ]; then
			if [ test $abi_glib_opt = required ]; then
				AC_MSG_ERROR([* * * unable to find glib12-config or glib-config in path! * * *])
			fi
			abi_glib_opt=no
		fi
	fi
	if [ test $abi_glib_opt != no ]; then
	        if [ $GLIB_CONFIG --version > /dev/null 2>&1 ]; then
			abi_glib_opt_version=`$GLIB_CONFIG --version`
			abi_glib_opt_major=`echo $abi_glib_opt_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
			abi_glib_opt_minor=`echo $abi_glib_opt_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
			abi_glib_opt_micro=`echo $abi_glib_opt_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

			abi_glib_opt_version=""
			if [ test $abi_glib_opt_major -eq 1 ]; then
				if [ test $abi_glib_opt_minor -eq 2 ]; then
					if [ test $abi_glib_opt_micro -ge "$1" ]; then
						abi_glib_opt_version="1.2.$abi_glib_opt_micro"
					fi
				fi
			fi
			if [ test "x$abi_glib_opt_version" = "x" ]; then
				if [ test $abi_glib_opt = required ]; then
					AC_MSG_ERROR([* * * glib version is incompatible! require at least "1.2.$1" * * *])
				fi
				abi_glib_opt=no
			fi
		else
			AC_MSG_WARN([* * * problem obtaining glib version... * * *])
			if [ test $abi_glib_opt = required ]; then
				AC_MSG_ERROR([* * * unable to determine glib version! * * *])
			fi
			abi_glib_opt=no
		fi
	fi
])

# On unix, check for glib library.
# On mac, check for glib, but it's optional; module support unnecessary.
# Ensure that the version number of glib is >= 1.2.0

# Usage: 
# ABI_GLIB
#

AC_DEFUN([ABI_GLIB], [

dnl Perhaps this should be:
dnl
dnl    if test "$BE_PLATFORM" = "unix"; then
dnl
dnl which would eliminate the cocoa distinction... ??

if test "$PLATFORM" = "unix"; then
	ABI_GLIB12_OPT(0,no)

        GMODULE_CFLAGS=`$GLIB_CONFIG --cflags gmodule`
        GMODULE_LIBS=`$GLIB_CONFIG --libs gmodule`

        AC_SUBST(GMODULE_CFLAGS)
        AC_SUBST(GMODULE_LIBS)

elif test "$PLATFORM" = "mac"; then
	ABI_GLIB12_OPT(0,yes)

	if [ test "x$abi_glib_opt" = "xno" ]; then
	        GLIB_CFLAGS=""
        	GLIB_LIBS=""
	else
	        GLIB_CFLAGS=`$GLIB_CONFIG --cflags`
        	GLIB_LIBS=`$GLIB_CONFIG --libs`
	fi

        AC_SUBST(GLIB_CFLAGS)
        AC_SUBST(GLIB_LIBS)

elif test "$PLATFORM" = "cocoa"; then
	ABI_GLIB12_OPT(0,no)

        GMODULE_CFLAGS=`$GLIB_CONFIG --cflags gmodule`
        GMODULE_LIBS=`$GLIB_CONFIG --libs gmodule`

        AC_SUBST(GMODULE_CFLAGS)
        AC_SUBST(GMODULE_LIBS)
fi

])
