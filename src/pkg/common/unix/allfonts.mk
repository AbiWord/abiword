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

##################################################################
# THIS MAKEFILE IS A STANDARD SET OF INSTALL TARGETS AND WILL BE 
# INCLUDED BY ACTUAL PLATFORM PKG MAKEFILES.  REFERENCES TO SHARED
# (COMMON) RESOURCES SHOULD BE DONE FROM A LOCATION "INSIDE" THE
# INCLUDING MAKEFILE.  BE SURE TO INCLUDE THIS MAKEFILE AFTER 
# YOU HAVE ALREADY INCLUDED THE ABI_DEFS.MK AND ABI_RULES.MK.
##################################################################	

##################################################################
## allfonts -- just a few defines that most (all?) font packages
##			will use (like tgzfonts.mk and maybe 
##			platform ones like rpmfonts.mk)

# extract the build version from the fonts package source
ABI_FONTS_BUILD_VERSION	= $(shell cat $(OUTDIR)/AbiSuite/fonts/BUILD)

# this supercedes the PKGBASENAME stuff defined in abi_defs.mk.
# fonts are only needed on Unix and need different names
ABI_FONTS_PKGBASENAME	= AbiSuite-Fonts-$(ABI_FONTS_BUILD_VERSION)
