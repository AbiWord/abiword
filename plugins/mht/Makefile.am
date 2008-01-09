
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libmht.la

libmht_la_LIBADD = \
	xp/libxp.la

libmht_la_LDFLAGS = \
	$(MHT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libmht_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libmht.dll

CLEANFILES = \
	libmht.def \
	libmht.dll \
	libmht.dll.a

libmht.dll: .libs/libmht.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(MHT_LIBS)

endif # TOOLKIT_WIN

