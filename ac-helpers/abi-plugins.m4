# start: abi/ac-helpers/abi-plugins.m4
# 
# Copyright (C) 2003 Francis James Franklin
# Copyright (C) 2003 AbiSource, Inc
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
# Usage: ABI_PLUGINS

AC_DEFUN([ABI_PLUGINS],[

dnl plugin configuration - if any

PLUGIN_CFLAGS=""
PLUGIN_LIBS=""

PLUGIN_LIST=""
PLUGIN_DEFS=""

abi_builtin_plugins=no
_abi_plugin_list=""

AC_ARG_WITH(builtin_plugins,[  --with-builtin-plugins  (experimental) compile-in a selection of plugins],[
	if test "$withval" != "no"; then
		abi_builtin_plugins=yes

		if test "$withval" != "yes"; then
			_abi_plugin_list=`echo $withval | tr ',' ' '`
		fi
	fi
])

if test $abi_builtin_plugins = yes; then
	for p in $_abi_plugin_list; do
		echo "configuring for plugin '"$p"':"

		dnl ...

		_abi_plugin_name=`echo $p | tr '-' '_'`
		PLUGIN_LIST="$PLUGIN_LIST $_abi_plugin_name.o"
		PLUGIN_DEFS="$PLUGIN_DEFS -DABIPGN_BUILTIN_`echo $_abi_plugin_name | tr 'a-z' 'A-Z'`=1"
	done
fi
AM_CONDITIONAL(BUILD_IN_PLUGINS,[test $abi_builtin_plugins = yes])

AC_SUBST(PLUGIN_CFLAGS)
AC_SUBST(PLUGIN_LIBS)

AC_SUBST(PLUGIN_LIST)
AC_SUBST(PLUGIN_DEFS)

])
# 
# end: abi/ac-helpers/abi-plugins.m4
# 
