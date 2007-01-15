# start: abi/ac-helpers/abi-peer-config.m4
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
# Usage: ABI_PEER_CONFIG(<peer>,<peerdir>)

AC_DEFUN([ABI_PEER_CONFIG],[
if test "$1" != "" -a "$2" != ""; then

_abi_peer="$1"
_abi_pdir="$2"
_abi_bdir="../$_abi_peer"

if ! test -e $_abi_bdir; then
	mkdir $_abi_bdir
fi
if test -d $_abi_bdir; then

echo ""
echo "configuring $_abi_peer: srcdir=$_abi_pdir, builddir=$_abi_bdir"
echo ""

config_flags='--disable-shared --enable-static'

if test "$_abi_peer" = "expat"; then
    _expat_cppflags="-I`cd $_abi_bdir; pwd`/lib"
    (cd $_abi_bdir && CPPFLAGS="$CPPFLAGS $_expat_cppflags" $_abi_pdir/configure $config_flags)

else
    if test "$_abi_peer" = "libiconv"; then
	config_flags="$config_flags --enable-extra-encodings"
    fi
    (cd $_abi_bdir && $_abi_pdir/configure $config_flags)
fi

echo ""

else

echo ""
echo "error: unable to configure $_abi_peer in $_abi_bdir - no such directory"
echo ""
exit

fi
fi
])
# 
# end: abi/ac-helpers/abi-peer-config.m4
# 
