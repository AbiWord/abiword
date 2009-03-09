#! gmake

## AbiSource Program Utilities
## Copyright (C) 1998,1999 AbiSource, Inc.
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

##################################################################
##################################################################
## Symbols to uniquely identify the build.
##
## ABI_BUILD_VERSION_MAJOR/MINOR/MICRO[/BUILD]	should be set to the three
##          numbers making up the build version (e.g. 1 0 0)
##          for a numbered build.  BUILD is optional, should be 0
##          for all release builds, indicates CVS or other nightly build.
##
## ABI_BUILD_ID		can be used as a identifying label (such as
##			a date stamp in a nightly build system).
##
## 
ABI_BUILD_VERSION_MAJOR= 2
ABI_BUILD_VERSION_MINOR= 6
ABI_BUILD_VERSION_MICRO= 8
