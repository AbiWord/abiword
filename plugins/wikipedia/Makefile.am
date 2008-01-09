
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libwikipedia.la

libwikipedia_la_LIBADD = \
	xp/libxp.la

libwikipedia_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libwikipedia_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libwikipedia.dll

CLEANFILES = \
	libwikipedia.def \
	libwikipedia.dll \
	libwikipedia.dll.a

libwikipedia.dll: .libs/libwikipedia.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

