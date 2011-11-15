SUBDIRS = common imp exp

#FIXME:FIDENCIO: Remove this clause when Cocoa's dialog is implemented
if !TOOLKIT_COCOA
SUBDIRS += dialogs
endif

if EPUB_BUILTIN
noinst_LTLIBRARIES = epub.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = epub.la

endif

epub_la_LIBADD = \
	common/libcommon.la \
	imp/libimp.la \
	exp/libexp.la 
#FIXME:FIDENCIO: Remove this clause when Cocoa's dialog is implemented
if !TOOLKIT_COCOA
epub_la_LIBADD += dialogs/libdialogs.la
endif
epub_la_LIBADD += $(EPUB_LIBS)

epub_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined
if TOOLKIT_COCOA
epub_la_LDFLAGS += -headerpad_max_install_names
endif # TOOLKIT_COCOA

epub_la_SOURCES = 
nodist_EXTRA_epub_la_SOURCES =  dummy.cpp

if TOOLKIT_COCOA
install-data-hook:
	install_name_tool -change $(ABIWORD_LIBDIR)/libabiword-@ABIWORD_SERIES@.dylib \
		@executable_path/../Frameworks/libabiword-@ABIWORD_SERIES@.dylib \
		$(DESTDIR)/$(plugindir)/epub.so
endif # TOOLKIT_COCOA

EXTRA_DIST = \
	plugin.m4
