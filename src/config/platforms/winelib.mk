#! gmake

## AbiSource Program Utilities
## Copyright (C) 2004 AbiSource, Inc.
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

##############################################################################
## Winelib platform defines
##############################################################################

include $(ABI_ROOT)/src/config/platforms/mingw32.mk

# Having them down here is hackish in that it syntactically has the potential to fsck things over if the defs get
# modified but in this specific case it should work because we dont.

CC             	= winegcc
CCC            	= wineg++
RC             	= wrc
EXE_SUFFIX		=
