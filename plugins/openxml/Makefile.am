
SUBDIRS = common imp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libopenxml.la

libopenxml_la_LIBADD = \
	common/libcommon.la \
	imp/libimp.la

libopenxml_la_LDFLAGS = \
	$(OPENXML_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libopenxml_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libopenxml.dll

CLEANFILES = \
	libopenxml.def \
	libopenxml.dll \
	libopenxml.dll.a

libopenxml.dll: .libs/libopenxml.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(OPENXML_LIBS)

endif # TOOLKIT_WIN

