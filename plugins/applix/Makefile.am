
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libapplix.la

libapplix_la_LIBADD = \
	xp/libxp.la

libapplix_la_LDFLAGS = \
	$(APPLIX_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libapplix_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libapplix.dll

CLEANFILES = \
	libapplix.def \
	libapplix.dll \
	libapplix.dll.a

libapplix.dll: .libs/libapplix.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(APPLIX_LIBS)

endif # TOOLKIT_WIN

