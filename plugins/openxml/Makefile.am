
SUBDIRS = common imp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = openxml.la

openxml_la_LIBADD = \
	common/libcommon.la \
	imp/libimp.la

openxml_la_LDFLAGS = \
	$(OPENXML_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

openxml_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	openxml.dll

CLEANFILES = \
	openxml.def \
	openxml.dll \
	openxml.dll.a

openxml.dll: .libs/libopenxml.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(OPENXML_LIBS)

endif # TOOLKIT_WIN

