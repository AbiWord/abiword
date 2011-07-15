
SUBDIRS = xp

if DOCBOOK_BUILTIN
noinst_LTLIBRARIES = docbook.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = docbook.la

endif

docbook_la_LIBADD = \
	xp/libxp.la

docbook_la_LDFLAGS = \
	$(DOCBOOK_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

docbook_la_SOURCES = 

nodist_EXTRA_docbook_la_SOURCES = dummy.cpp

if TOOLKIT_COCOA
install-data-hook:
	install_name_tool -change $(ABIWORD_LIBDIR)/libabiword-@ABIWORD_SERIES@.dylib \
		@executable_path/../Frameworks/libabiword-@ABIWORD_SERIES@.dylib \
		$(DESTDIR)/$(plugindir)/docbook.so
endif # TOOLKIT_COCOA

EXTRA_DIST = \
	plugin.m4

