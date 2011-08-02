
SUBDIRS = common imp exp

if OPENXML_BUILTIN
noinst_LTLIBRARIES = openxml.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = openxml.la

endif

openxml_la_LIBADD = \
	common/libcommon.la \
	imp/libimp.la \
	exp/libexp.la

openxml_la_LDFLAGS = \
	$(OPENXML_LIBS) \
	-avoid-version \
	-module \
	-no-undefined
if TOOLKIT_COCOA
openxml_la_LDFLAGS += -headerpad_max_install_names
endif # TOOLKIT_COCOA

openxml_la_SOURCES = 
nodist_EXTRA_openxml_la_SOURCES =  dummy.cpp

if TOOLKIT_COCOA
install-data-hook:
	install_name_tool -change $(ABIWORD_LIBDIR)/libabiword-@ABIWORD_SERIES@.dylib \
		@executable_path/../Frameworks/libabiword-@ABIWORD_SERIES@.dylib \
		$(DESTDIR)/$(plugindir)/openxml.so
endif # TOOLKIT_COCOA

EXTRA_DIST = \
	plugin.m4

