
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libpsion.la

libpsion_la_LIBADD = \
	xp/libxp.la

libpsion_la_LDFLAGS = \
	$(PSION_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libpsion_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libpsion.dll

CLEANFILES = \
	libpsion.def \
	libpsion.dll \
	libpsion.dll.a

libpsion.dll: .libs/libpsion.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PSION_LIBS)

endif # TOOLKIT_WIN

