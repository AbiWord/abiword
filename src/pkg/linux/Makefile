#! gmake

## AbiSource Applications
## Copyright (C) 1998 AbiSource, Inc.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
## 02111-1307, USA.

ABI_DEPTH=../..
include $(ABI_DEPTH)/config/abi_defs_wp.mk
include $(ABI_DEPTH)/config/abi_rules.mk

##################################################################
## makefile target to make binary distribution for this platform.

# Each distribution format we include (like tgz, rpm, deb) will
# take care of making its own font packages.
distribution:	tgz deb

# things covered by all unix platform code

# tgz
include $(ABI_DEPTH)/pkg/common/unix/tgz.mk

# things only Linux does

# deb
include deb/deb.mk
