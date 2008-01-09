
SUBDIRS = common exp imp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libopendocument.la

libopendocument_la_LIBADD = \
	common/libcommon.la \
	exp/libexp.la \
	imp/libimp.la

libopendocument_la_LDFLAGS = \
	$(OPENDOCUMENT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libopendocument_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libopendocument.dll

CLEANFILES = \
	libopendocument.def \
	libopendocument.dll \
	libopendocument.dll.a

libopendocument.dll: .libs/libopendocument.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(OPENDOCUMENT_LIBS)

endif # TOOLKIT_WIN

