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

		PLUGIN_CFLAGS="-DABI_PLUGIN_BUILTIN=1"

		if test "$withval" != "yes"; then
			_abi_plugin_list=`echo $withval | tr ',' ' '`
		elif test -d ../abiword-plugins; then
			_abi_pwd=`pwd`
			cd ../abiword-plugins
			for i in `find . -name "*.a" -print`; do
				i=`dirname $i`;
				i=`dirname $i`;
				i=`dirname $i`;
				i=`basename $i`;
				if test "$i" != "."; then
					_abi_plugin_list="$_abi_plugin_list $i"
				fi
			done
			cd $_abi_pwd
		fi
	fi
])

if test $abi_builtin_plugins = yes; then

	dnl the real problem is, of course, PLUGIN_LIBS - the various libraries that
	dnl these depend on...

	for p in $_abi_plugin_list; do
		echo "configuring for plugin '"$p"':"

		case $p in
		abicommand)
			_abi_plugin_lib=AbiCommand
			ABI_COMMAND_LDFLAGS=""
			have_readline=unknown
			AC_CHECK_HEADER(readline/readline.h,[
				AC_CHECK_HEADER(readline/history.h,[
					AC_CHECK_LIB(readline,readline,[
						have_readline=yes
						LDFLAGS="-ltermcap $LDFLAGS"
						ABI_COMMAND_LDFLAGS="-ltermcap $ABI_COMMAND_LDFLAGS"
					],[	AC_CHECK_LIB(readline,rl_initialize,[
							have_readline=yes
							LDFLAGS="-lcurses $LDFLAGS"
							ABI_COMMAND_LDFLAGS="-lcurses $ABI_COMMAND_LDFLAGS"
						],have_readline=no,-lcurses)
					],-ltermcap)
				],have_readline=no)
			],have_readline=no)

			if test $have_readline != yes; then
				AC_MSG_ERROR([abicommand: error - readline libs or hdrs not found])
			else
				PLUGIN_LIBS="$PLUGIN_LIBS -lreadline -lhistory $ABI_COMMAND_LDFLAGS"
			fi
		;;
		abigimp)
		_abi_plugin_lib=AbiGimp
		;;
		aiksaurus)
		_abi_plugin_lib=AbiAikSaurus
		;;
		babelfish)
		_abi_plugin_lib=AbiBabelfish
		;;
		freetranslation)
		_abi_plugin_lib=AbiFreeTranslation
		;;
		gda)
		_abi_plugin_lib=AbiGDA
		;;
		gdict)
		_abi_plugin_lib=AbiGdict
		;;
		google)
		_abi_plugin_lib=AbiGoogle
		;;
		gypsython)
		_abi_plugin_lib=AbiGypsython
		;;
		ots)
			_abi_plugin_lib=AbiOTS
			PKG_CHECK_MODULES(_abi_ots,libots-1 >= 0.2.0)
			PLUGIN_LIBS="$PLUGIN_LIBS $_abi_ots_LIBS"
		;;
		referee)
		_abi_plugin_lib=AbiReferee
		;;
		urldict)
		_abi_plugin_lib=AbiURLDict
		;;
		wikipedia)
		_abi_plugin_lib=AbiWikipedia
		;;
		applix)
		_abi_plugin_lib=AbiApplix
		;;
		bz2abw)
		_abi_plugin_lib=AbiBZ2
		;;
		clarisworks)
		_abi_plugin_lib=AbiClarisWorks
		;;
		coquille)
		_abi_plugin_lib=AbiCoquille
		;;
		docbook)
		_abi_plugin_lib=AbiDocBook
		;;
		eml)
		_abi_plugin_lib=AbiEML
		;;
		AbiGdkPixbuf)
		_abi_plugin_lib=AbiGdkPixbuf
		;;
		bmp)
		_abi_plugin_lib=AbiBMP
		;;
		jpeg)
		_abi_plugin_lib=AbiJPEG
		;;
		librsvg)
		_abi_plugin_lib=AbiRSVG
		;;
		magick)
			_abi_plugin_lib=AbiMagick
			ABI_MAGICKPP_OPT(5.4.0,no)
			PLUGIN_LIBS="$PLUGIN_LIBS `$abi_magickpp_config --ldflags` `$abi_magickpp_config --libs`"
		;;
		wmf)
		_abi_plugin_lib=AbiWMF
		;;
		hancom)
		_abi_plugin_lib=AbiHancom
		;;
		hrtext)
		_abi_plugin_lib=AbiHRText
		;;
		iscii-text)
		_abi_plugin_lib=AbiISCII
		;;
		kword)
		_abi_plugin_lib=AbiKWord
		;;
		latex)
		_abi_plugin_lib=AbiLaTeX
		;;
		mif)
		_abi_plugin_lib=AbiMIF
		;;
		mswrite)
		_abi_plugin_lib=AbiMSWrite
		;;
		nroff)
		_abi_plugin_lib=AbiNroff
		;;
		OpenWriter)
			_abi_plugin_lib=AbiOpenWriter
			PKG_CHECK_MODULES(_abi_openwriter,libgsf-1 >= 1.4.0)
			PLUGIN_LIBS="$PLUGIN_LIBS $_abi_openwriter_LIBS"
		;;
		pdb)
		_abi_plugin_lib=AbiPalmDoc
		;;
		psion)
		_abi_plugin_lib=AbiPsion
		;;
		pw)
		_abi_plugin_lib=AbiPW
		;;
		sdw)
			_abi_plugin_lib=AbiSDW
			PKG_CHECK_MODULES(_abi_sdw,libgsf-1 >= 1.4.0)
			PLUGIN_LIBS="$PLUGIN_LIBS $_abi_sdw_LIBS"
		;;
		t602)
		_abi_plugin_lib=AbiT602
		;;
		wml)
		_abi_plugin_lib=AbiWML
		;;
		wordperfect)
			_abi_plugin_lib=AbiWordPerfect
			PKG_CHECK_MODULES(_abi_wordperfect,libwpd-1 >= 0.5.0)
			PLUGIN_LIBS="$PLUGIN_LIBS $_abi_wordperfect_LIBS"
		;;
		xhtml)
		_abi_plugin_lib=AbiXHTML
		;;
		xsl-fo)
		_abi_plugin_lib=AbiXSLFO
		;;
		shell)
		_abi_plugin_lib=AbiScriptHappy
		;;
		*)
		AC_MSG_ERROR([I do not know plugin $p])
		;;
		esac

		_abi_plugin_name=`echo $p | tr '-' '_'`
		PLUGIN_LIST="$PLUGIN_LIST lib$_abi_plugin_lib.a"
		PLUGIN_DEFS="$PLUGIN_DEFS -DABIPGN_BUILTIN_`echo $_abi_plugin_name | tr '@<:@a-z@:>@' '@<:@A-Z@:>@'`=1"
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
