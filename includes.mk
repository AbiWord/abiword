## AbiSource Applications
## Copyright (C) 2001 Sam Tobin-Hochstadt
## Copyright (C) 2001 Hubert Figuiere <hfiguiere@teaser.fr>
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

# The point of this file is to encapsulate all the nasty knowledge
# about options and platforms in one place, so that a makefile down
# the tree can just include this and then use some variables.  This
# makes the job of dealing with regular make files much simpler.  

# automake complains at us if we just if out the gnome-specific parts
if WITH_GNOME
AF_INCLUDES=-I'$(top_srcdir)/src/af/util/xp' 
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/gr/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/xap/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/util/@BE_PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/@PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/gr/@PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/xap/@PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/xap/@PLATFORM@/gnome'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/@PLATFORM@/gnome'
else
AF_INCLUDES=-I'$(top_srcdir)/src/af/util/xp' 
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/gr/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/xap/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/util/@BE_PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/@PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/gr/@PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/xap/@PLATFORM@'
endif

if WITH_GNOME
WP_INCLUDES=-I'$(top_srcdir)/src/wp/ap/xp'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/impexp/xp'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/ap/@PLATFORM@'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/ap/xp/ToolbarIcons'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/ap/@PLATFORM@/gnome'
else
WP_INCLUDES=-I'$(top_srcdir)/src/wp/ap/xp'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/impexp/xp'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/ap/@PLATFORM@'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/ap/xp/ToolbarIcons'
endif

OTHER_INCLUDES=-I'$(top_srcdir)/src/other/spell/xp'
OTHER_INCLUDES+=-I'$(top_srcdir)/src/other/fribidi/xp'
OTHER_INCLUDES+=-I'$(top_srcdir)/src/other/ttftool/@BE_PLATFORM@'

TEXT_INCLUDES=-I'$(top_srcdir)/src/text/ptbl/xp'
TEXT_INCLUDES+=-I'$(top_srcdir)/src/text/fmt/xp'

TOOLS_INCLUDES=-I'$(top_srcdir)/src/tools/cdump/xp'

WV_INCLUDES=-I'$(top_srcdir)/../wv/'
WV_INCLUDES+=-I'$(top_srcdir)/../wv/libole2'
WV_INCLUDES+=-I'$(top_srcdir)/../wv/exporter'
WV_INCLUDES+=-I'$(top_srcdir)/../wv/glib-wv'

# expat includes are handled by @XML_INCLUDES@
# iconv includes are handled by @ICONV_INCLUDES@

# these are appropriately empty when the various --enable-foo's are
# off 

ABI_CFLAGS=@WARNING_CFLAGS@ @DEBUG_CFLAGS@ @OPTIMIZE_CFLAGS@ \
	@PROFILE_CFLAGS@ @XML_CFLAGS@ @SCRIPT_CFLAGS@ @BIDI_CFLAGS@ \
	@WV_CFLAGS@ @LIBPOPT_CFLAGS@ @XFT_CFLAGS@ \
	@LIBPNG_CFLAGS@ @ZLIB_CFLAGS@ @CURLHASH_CFLAGS@ @LIBCURL_CFLAGS@

MACOSX_CFLAGS=-DXP_MAC_TARGET_MACOSX -DXP_MAC_TARGET_QUARTZ 

if WITH_CARBON
CARBON_CFLAGS = -DXP_MAC_TARGET_CARBON $(MACOSX_CFLAGS)
else
CARBON_CFLAGS = 
endif

if WITH_COCOA
COCOA_CFLAGS = -DXP_TARGET_COCOA $(MACOSX_CFLAGS) 
else
COCOA_CFLAGS = 
endif

PLATFORM_CFLAGS = @PLATFORM_CFLAGS@ $(CARBON_CFLAGS) $(COCOA_CFLAGS)

CXXFLAGS = @CXXFLAGS@ $(ABI_CFLAGS) $(PLATFORM_CFLAGS) -DABI_BUILD_VERSION=\"@VERSION@\"
CFLAGS   = @CFLAGS@   $(ABI_CFLAGS) $(PLATFORM_CFLAGS)

ABI_FE = @ABI_FE@
ABI_BE = @ABI_BE@
ABI_GNOME_PREFIX = Gnome

OTHER_LIBS=@SPELL_LIBS@ @XML_LIBS@ @SCRIPT_LIBS@ \
	@WV_LIBS@ @GLIB_LIBS@ @ICONV_LIBS@ \
	@LIBPNG_LIBS@ @ZLIB_LIBS@ @LIBPOPT_LIBS@ @LIBCURL_LIBS@ @XFT_LIBS@

# BiDi needs a specific lib
if BIDI_ENABLED
BIDI_LIBS=$(top_builddir)/src/other/fribidi/xp/libFribidi.a
else
BIDI_LIBS=
endif
ABI_LIBS=$(top_builddir)/src/wp/ap/libAp.a $(BIDI_LIBS)
ABI_LIBS+=$(top_builddir)/src/wp/impexp/libImpExp.a
ABI_LIBS+=$(top_builddir)/src/af/xap/libXap.a
ABI_LIBS+=$(top_builddir)/src/af/util/libUtil.a
ABI_LIBS+=$(top_builddir)/src/af/gr/libGr.a
ABI_LIBS+=$(top_builddir)/src/af/ev/libEv.a
ABI_LIBS+=$(top_builddir)/src/other/spell/xp/libSpell.a
ABI_LIBS+=$(top_builddir)/src/other/ttftool/@BE_PLATFORM@/libTtfTool.a
ABI_LIBS+=$(top_builddir)/src/other/fribidi/xp/libFribidi.a
ABI_LIBS+=$(top_builddir)/src/text/fmt/xp/libFmt.a
ABI_LIBS+=$(top_builddir)/src/text/ptbl/xp/libPtbl.a

# we don't assume that WITH_GNOME => unix, on the off chance that
# someday it won't
if WITH_GNOME
ABI_GNOME_OBJECTS=xp/*.o @PLATFORM@/*.o @PLATFORM@/gnome/*.o
endif 

ABI_OBJECTS=xp/*.o @PLATFORM@/*.o 

SUFFIXES=.mm
# Added for automake (at least version 1.5) - Frodo Looijaard (frodol@dds.nl)
.mm.lo:
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(AM_OBJCFLAGS) $(OBJCFLAGS) $<
.mm.o:
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(AM_OBJCFLAGS) $(OBJCFLAGS) $<

# MacOS X resource compiling
# TODO add autoconf macros to detect Rez, ResMerger and other stuff. 
#      currently we use the hardcode locations
if WITH_MACOSX
REZ             = /Developer/Tools/Rez
RESMERGER       = /Developer/Tools/ResMerger
ABI_MACREZ_INCS= -i $(top_builddir)/src/af/xap/mac
ABI_MACREZ_OPTS= $(ABI_MACREZ_INCS) -d REZ_CARBON -F Carbon -F HIToolbox -useDF

%.rsrc: %.r
	$(REZ) -o $@ $(ABI_MACREZ_INCS) $(ABI_MACREZ_OPTS) $<
endif
