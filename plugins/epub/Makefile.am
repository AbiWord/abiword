SUBDIRS = common imp exp dialogs

if EPUB_BUILTIN
noinst_LTLIBRARIES = epub.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = epub.la

endif

epub_la_LIBADD = \
	dialogs/libdialogs.la\
	common/libcommon.la \
	imp/libimp.la \
	exp/libexp.la 

epub_la_LDFLAGS = \
	$(EPUB_LIBS) \
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
