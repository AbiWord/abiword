
SUBDIRS = xp

if DOCBOOK_BUILTIN
noinst_LTLIBRARIES = docbook.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = docbook.la

endif

docbook_la_LIBADD = \
	xp/libxp.la \
	$(DOCBOOK_LIBS)

docbook_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined
if TOOLKIT_COCOA
docbook_la_LDFLAGS += -headerpad_max_install_names
endif # TOOLKIT_COCOA

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

