## AbiSource Applications
## Copyright (C) 2002 Francis James Franklin
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

bundledir = $(prefix)/AbiWord.app
contentsdir = $(bundledir)/Contents
macosdir = $(contentsdir)/MacOS
frameworksdir = $(contentsdir)/Frameworks
resourcesdir = $(contentsdir)/Resources
englishdir = $(resourcesdir)/English.lproj

xapsrcdir = $(top_srcdir)/src/af/xap/cocoa
apsrcdir = $(top_srcdir)/src/wp/ap/cocoa
mainsrcdir = $(top_srcdir)/src/wp/main/cocoa
mainbuilddir = $(top_builddir)/src/wp/main/cocoa

englishbuilddir = $(mainbuilddir)/AbiWord.app/Contents/Resources/English.lproj
