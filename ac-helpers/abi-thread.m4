# start: abi/ac-helpers/abi-thread.m4
# 
# Copyright (C) 2002 Francis James Franklin
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
# Usage: ABI_THREAD

AC_DEFUN([ABI_THREAD],[

AC_ARG_ENABLE(threads,[  --enable-threads    use (posix) threads],[
	if test "x$enableval" = "xno"; then
		abi_threads=no
	else
		abi_threads=yes
	fi
],[	abi_threads=check
])

if test $abi_threads != no; then
	AC_CHECK_HEADER(pthread.h,[
		AC_CHECK_FUNC(pthread_create,[
			abi_pthread_libs=""
			AC_CHECK_FUNC(pthread_yield,abi_pthread_yield=yes,abi_pthread_yield=no)
		],[	AC_CHECK_LIB(pthread,pthread_exit,[
				abi_pthread_libs="-lpthread"
				AC_CHECK_LIB(pthread,pthread_yield,[
					abi_pthread_yield=yes
				],[	abi_pthread_yield=no
				])
			],[	AC_MSG_ERROR([* * * posix threads broken? * * *])
			])
		])
	],[	if test $abi_threads = yes; then
			AC_MSG_ERROR([* * * only posix threads supported currently * * *])
		fi
		abi_threads=no
	])
fi

if test $abi_threads != no; then
	THREAD_CFLAGS="-DHAVE_THREADS=1 -DHAVE_POSIX_THREADS=1"
	THREAD_LIBS="$abi_pthread_libs"
	if test "x$abi_pthread_yield" = "xyes"; then
		THREAD_CFLAGS="$THREAD_CFLAGS -DHAVE_PTHREAD_YIELD=1"
	fi
	abi_threads=yes
else
	THREAD_CFLAGS=""
	THREAD_LIBS=""
fi
AM_CONDITIONAL(HAVE_THREADS, test $abi_threads = yes)
AC_SUBST(THREAD_CFLAGS)
AC_SUBST(THREAD_LIBS)

])
# 
# end: abi/ac-helpers/abi-thread.m4
# 
