
SUBDIRS = xp

if OPENWRITER_BUILTIN
noinst_LTLIBRARIES = openwriter.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = openwriter.la

endif

openwriter_la_LIBADD = \
	xp/libxp.la \
	$(OPENWRITER_LIBS)

openwriter_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined
if TOOLKIT_COCOA
openwriter_la_LDFLAGS += -headerpad_max_install_names
endif # TOOLKIT_COCOA

openwriter_la_SOURCES = 
nodist_EXTRA_openwriter_la_SOURCES = dummy.cpp

if TOOLKIT_COCOA
install-data-hook:
	install_name_tool -change $(ABIWORD_LIBDIR)/libabiword-@ABIWORD_SERIES@.dylib \
		@executable_path/../Frameworks/libabiword-@ABIWORD_SERIES@.dylib \
		$(DESTDIR)/$(plugindir)/openwriter.so
endif # TOOLKIT_COCOA

EXTRA_DIST = \
	plugin.m4

