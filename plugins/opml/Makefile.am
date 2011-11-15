
SUBDIRS = xp

if OPML_BUILTIN
noinst_LTLIBRARIES = opml.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = opml.la

endif

opml_la_LIBADD = \
	xp/libxp.la \
	$(OPML_LIBS)

opml_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined
if TOOLKIT_COCOA
opml_la_LDFLAGS += -headerpad_max_install_names
endif # TOOLKIT_COCOA

opml_la_SOURCES = 
nodist_EXTRA_opml_la_SOURCES = dummy.cpp

if TOOLKIT_COCOA
install-data-hook:
	install_name_tool -change $(ABIWORD_LIBDIR)/libabiword-@ABIWORD_SERIES@.dylib \
		@executable_path/../Frameworks/libabiword-@ABIWORD_SERIES@.dylib \
		$(DESTDIR)/$(plugindir)/opml.so
endif # TOOLKIT_COCOA

EXTRA_DIST = \
	plugin.m4

