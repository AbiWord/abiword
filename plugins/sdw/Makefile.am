
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libsdw.la

libsdw_la_LIBADD = \
	xp/libxp.la

libsdw_la_LDFLAGS = \
	$(SDW_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libsdw_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libsdw.dll

CLEANFILES = \
	libsdw.def \
	libsdw.dll \
	libsdw.dll.a

libsdw.dll: .libs/libsdw.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(SDW_LIBS)

endif # TOOLKIT_WIN

