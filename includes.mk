## AbiSource Applications
## Copyright (C) 2001 Sam Tobin-Hochstadt
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
AF_INCLUDES+=-I'$(top_srcdir)/src/af/util/@PLATFORM@'
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
AF_INCLUDES+=-I'$(top_srcdir)/src/af/util/@PLATFORM@'
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
OTHER_INCLUDES+=-I'$(top_srcdir)/src/other/fribidi'

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

if WITH_GNOME
ABI_CFLAGS=@PLATFORM_CFLAGS@ @WARNING_CFLAGS@ @DEBUG_CFLAGS@ @OPTIMIZE_CFLAGS@ \
	@PROFILE_CFLAGS@ @XML_CFLAGS@ @SCRIPT_CFLAGS@ @BIDI_CFLAGS@ \
	@PSICONV_CFLAGS@ @WV_CFLAGS@ @GNOME_CFLAGS@
else
ABI_CFLAGS=@PLATFORM_CFLAGS@ @WARNING_CFLAGS@ @DEBUG_CFLAGS@ @OPTIMIZE_CFLAGS@ \
	@PROFILE_CFLAGS@ @XML_CFLAGS@ @SCRIPT_CFLAGS@ @BIDI_CFLAGS@ \
	@PSICONV_CFLAGS@ @WV_CFLAGS@ 
endif

# this is done here because of m4 macro conflict in configure.in
if WITH_MACOSX
MACOSX_CFLAGS=-I'$(top_builddir)/src/af/util/unix' -DXP_MAC_TARGET_MACOSX -DXP_MAC_TARGET_CARBON -DXP_MAC_TARGET_QUARTZ $(ABI_CFLAGS)
else
MACOSX_CFLAGS=
endif

CXXFLAGS=$(ABI_CFLAGS) $(MACOSX_CFLAGS)
CFLAGS=$(ABI_CFLAGS) $(MACOSX_CFLAGS)

# these are currently hacks.  They need to be auto-detected
ABI_FE = @ABI_FE@
ABI_GNOME_PREFIX = Gnome

# PSPELL_LIBS is empty if pspell is not enabled
OTHER_LIBS=-lpng -lz @PSPELL_LIBS@ @XML_LIBS@ @SCRIPT_LIBS@ \
	@PSICONV_LIBS@ @WV_LIBS@

ABI_LIBS=$(top_builddir)/src/wp/ap/libAp.a 
ABI_LIBS+=$(top_builddir)/src/wp/impexp/xp/libImpexp.a
ABI_LIBS+=$(top_builddir)/src/af/xap/libXap.a
ABI_LIBS+=$(top_builddir)/src/af/util/libUtil.a
ABI_LIBS+=$(top_builddir)/src/af/gr/libGr.a
ABI_LIBS+=$(top_builddir)/src/af/ev/libEv.a
ABI_LIBS+=$(top_builddir)/src/other/spell/xp/libSpell.a
ABI_LIBS+=$(top_builddir)/src/text/fmt/xp/libFmt.a
ABI_LIBS+=$(top_builddir)/src/text/ptbl/xp/libPtbl.a

# we don't assume that WITH_GNOME => unix, on the off chance that
# someday it won't
if WITH_GNOME
ABI_GNOME_OBJECTS=xp/*.o @PLATFORM@/*.o @PLATFORM@/gnome/*.o
endif 

ABI_OBJECTS=xp/*.o @PLATFORM@/*.o 


