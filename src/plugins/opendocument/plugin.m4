dnl= ##########################################################################
dnl= # abiword-plugins: autogen.sh header
dnl= # this file is executed as a shell script
dnl= 
dnl= # 1. abi_plugin_macro is the name of the main macro defined here:
dnl= abi_plugin_macro="ABIPGN_OPENDOCUMENT"
dnl= 
dnl= # 2. abi_plugin_files is the list of files that configure must generate:
dnl= abi_plugin_files="GNUmakefile common/GNUmakefile common/xp/GNUmakefile exp/GNUmakefile exp/xp/GNUmakefile imp/GNUmakefile imp/xp/GNUmakefile"
dnl= 
dnl= # 3. abi_plugin_desc is a short description of the plugin
dnl= abi_plugin_desc="Plugin to allow abiword to read/write OASIS OpenDocument .odt files"
dnl=
dnl= # 4. abi_plugin_enable - whether plugin should be enabled by default 
dnl= abi_plugin_enable="yes"
dnl=
dnl= return
dnl= ##########################################################################

AC_DEFUN([ABIPGN_OPENDOCUMENT],[

_abi_cppflags_save="$CPPFLAGS"
_abi_ldflags_save="$LDFLAGS"

ABI_OPENDOCUMENT_CPPFLAGS=""
ABI_OPENDOCUMENT_LDFLAGS=""

# Checks for libraries.
# Checks for header files.

ABI_PLUGIN_REPORT([OpenDocument: okay])

AC_SUBST(ABI_OPENDOCUMENT_CPPFLAGS)
AC_SUBST(ABI_OPENDOCUMENT_LDFLAGS)

CPPFLAGS="$_abi_cppflags_save"
LDFLAGS="$_abi_ldflags_save"
])
