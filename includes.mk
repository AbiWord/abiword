## AbiSource Applications
## Copyright (C) 2001 Sam Tobin-Hochstadt
## Copyright (C) 2001, 2005 Hubert Figuiere <hfiguiere@teaser.fr>
## Copyright (C) 2005 J.M. Maurer <uwog@abisource.com>
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

# automake complains at us if we just if out the hildon-specific parts
if EMBEDDED_HILDON
AF_INCLUDES=-I'$(top_srcdir)/src/af/util/xp' 
AF_INCLUDES+=-I'$(top_srcdir)/src/af/tf/xp' 
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/gr/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/xap/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/util/@BE_PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/@PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/gr/@PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/xap/@PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/xap/@PLATFORM@/hildon'
else
AF_INCLUDES=-I'$(top_srcdir)/src/af/util/xp' 
AF_INCLUDES+=-I'$(top_srcdir)/src/af/tf/xp' 
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/gr/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/xap/xp'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/util/@BE_PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/ev/@PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/gr/@PLATFORM@'
AF_INCLUDES+=-I'$(top_srcdir)/src/af/xap/@PLATFORM@'
endif

if EMBEDDED_HILDON
WP_INCLUDES=-I'$(top_srcdir)/src/wp/ap/xp'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/impexp/xp'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/ap/@PLATFORM@'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/ap/xp/ToolbarIcons'
AF_INCLUDES+=-I'$(top_srcdir)/src/wp/ap/@PLATFORM@/hildon'
else
WP_INCLUDES=-I'$(top_srcdir)/src/wp/ap/xp'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/impexp/xp'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/ap/@PLATFORM@'
WP_INCLUDES+=-I'$(top_srcdir)/src/wp/ap/xp/ToolbarIcons'
WP_INCLUDES+=-I'$(top_srcdir)/src/pkg/linux/apkg'
endif

OTHER_INCLUDES=

if ENABLE_SPELL
OTHER_INCLUDES+=-I'$(top_srcdir)/src/other/spell/xp'
endif

TEXT_INCLUDES=-I'$(top_srcdir)/src/text/ptbl/xp'
TEXT_INCLUDES+=-I'$(top_srcdir)/src/text/fmt/xp'
if WITH_UNIX
TEXT_INCLUDES+=-I'$(top_srcdir)/src/text/fmt/unix'
endif

TOOLS_INCLUDES=-I'$(top_srcdir)/src/tools/cdump/xp'

# expat includes are handled by @XML_INCLUDES@
# iconv includes are handled by @ICONV_INCLUDES@

# these are appropriately empty when the various --enable-foo's are
# off 

ABI_CFLAGS=@WARNING_CFLAGS@ @DEBUG_CFLAGS@ @OPTIMIZE_CFLAGS@ \
	@PROFILE_CFLAGS@ @XML_CFLAGS@ @PLUGIN_CFLAGS@ @FRIBIDI_CFLAGS@ \
	@WV_CFLAGS@ @LIBPOPT_CFLAGS@ @XFT_CFLAGS@ @FREETYPE_CFLAGS@ \
	@LIBPNG_CFLAGS@ @ZLIB_CFLAGS@ @THREAD_CFLAGS@ @ABI_FEATURE_DEFS@ @ABITYPES_CFLAGS@ \
	@GSF_CFLAGS@ @GOFFICE_CFLAGS@ @SPELL_CFLAGS@ @DEFS@


if WITH_WIN32
# the _WIN32_IE define is a bit hackish (should get it from the MINGW environment), but it works
WIN32_CFLAGS = \
				-DWIN32 \
				-D_WIN32_IE=0x0300 \
				-UHAVE_STRCASECMP -UHAVE_STRICMP -U__STRICT_ANSI__ -U_NO_OLDNAMES
else
WIN32_CFLAGS = 
endif

MACOSX_CFLAGS=-DXP_MAC_TARGET_MACOSX -DXP_MAC_TARGET_QUARTZ 

if WITH_COCOA
COCOA_CFLAGS = -DXP_TARGET_COCOA $(MACOSX_CFLAGS) 
else
COCOA_CFLAGS = 
endif

PLATFORM_CFLAGS = @PLATFORM_CFLAGS@ $(COCOA_CFLAGS) $(WIN32_CFLAGS)

CFLAGS   = @CFLAGS@   $(ABI_CFLAGS) $(PLATFORM_CFLAGS)
CXXFLAGS = @CXXFLAGS@ $(ABI_CFLAGS) $(PLATFORM_CFLAGS) \
	-DABI_BUILD_VERSION=\"@VERSION@\"

ABI_FE = @ABI_FE@
ABI_BE = @ABI_BE@
ABI_GNOME_PREFIX = Gnome

OTHER_LIBS=@SPELL_LIBS@ @XML_LIBS@ @PLUGIN_LIBS@ @FRIBIDI_LIBS@ \
	@WV_LIBS@ @GSF_LIBS@ @GLIB_LIBS@ @ICONV_LIBS@ @LIBPNG_LIBS@ @ZLIB_LIBS@ \
	@LIBPOPT_LIBS@ @XFT_LIBS@ @FREETYPE_LIBS@ @THREAD_LIBS@

ABI_LIBS=
if WITH_SYSTEM_GOFFICE
OTHER_LIBS += @GOFFICE_LIBS@
else
ABI_LIBS += @GOFFICE_LIBS@
endif

# BiDi needs a specific lib
#automake is TEH SILLY!
ABI_LIBS+=$(top_builddir)/src/wp/ap/libAp.a
ABI_LIBS+=$(top_builddir)/src/wp/impexp/libImpExp.a
ABI_LIBS+=$(top_builddir)/src/af/xap/libXap.a
ABI_LIBS+=$(top_builddir)/src/af/util/libUtil.a
ABI_LIBS+=$(top_builddir)/src/af/gr/libGr.a
ABI_LIBS+=$(top_builddir)/src/af/ev/libEv.a
ABI_LIBS+=$(top_builddir)/src/text/fmt/libFmt.a
ABI_LIBS+=$(top_builddir)/src/text/ptbl/xp/libPtbl.a


if ENABLE_SPELL
ABI_LIBS+=$(top_builddir)/src/other/spell/xp/libSpell.a
endif

ABI_TEST_LIBS=$(top_builddir)/src/af/util/libTestUtil.a
ABI_TEST_LIBS+=$(top_builddir)/src/text/ptbl/xp/t/libTestPtbl.a
ABI_TEST_LIBS+=$(top_builddir)/src/af/tf/libTF.a
ABI_TEST_LIBS+=$(top_builddir)/src/af/xap/libTestXap.a

if EMBEDDED_HILDON
ABI_HILDON_OBJECTS=xp/*.o @PLATFORM@/*.o @PLATFORM@/hildon/*.o
endif

ABI_OBJECTS=xp/*.o @PLATFORM@/*.o 
ABI_TEST_OBJECTS=xp/t/*.o 
#@PLATFORM@/t/*.o 

