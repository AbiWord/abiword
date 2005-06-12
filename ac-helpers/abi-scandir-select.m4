# start: abi/ac-helpers/abi-scandir-select.m4
# 
# Copyright (C) 2005 AbiSource, Inc
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
# serial: 1
#
# Usage: ABI_FUNC_SCANDIR_SELECT_CHECK

AC_DEFUN([ABI_FUNC_SCANDIR_SELECT_CHECK],[

dnl determines whether the "select" argument to the C scandir()
dnl function takes a const struct dirent * argument or just a struct dirent *.

dnl should call AC_PREREQ for several other macros here.

AC_LANG_PUSH([C++])
AC_MSG_CHECKING([whether scandir select requires const struct dirent])

AC_COMPILE_IFELSE([

#include <sys/types.h>
#include <dirent.h>

// desperately looking for NULL
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern "C" {
	int abi_select_test(const struct dirent *d) { return 0; }
}

int main(int argc, char ** argv) {
	struct dirent **namelist;
	int n = 0;

	n = scandir("/tmp", &namelist, abi_select_test, NULL);
	return 0;
}
],[
AC_MSG_RESULT([yes])
AC_DEFINE(ABI_SCANDIR_SELECT_QUALIFIER,[const],[
Define to const if the scandir select function requires that its argument is const-qualified, empty otherwise])
],[
AC_MSG_RESULT([no])
AC_DEFINE(ABI_SCANDIR_SELECT_QUALIFIER,[],[
Define to const if the scandir select function requires that its argument is const-qualified, empty otherwise])
])

AC_LANG_POP([C++])
AC_SUBST(ABI_SCANDIR_SELECT_QUALIFIER)

])
# 
# end: abi/ac-helpers/abi-scandir-select.m4
#
