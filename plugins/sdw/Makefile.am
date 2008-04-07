
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = sdw.la

sdw_la_LIBADD = \
	xp/libxp.la

sdw_la_LDFLAGS = \
	$(SDW_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

sdw_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	sdw.dll

CLEANFILES = \
	sdw.def \
	sdw.dll \
	sdw.dll.a

sdw.dll: .libs/libsdw.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(SDW_LIBS)

endif # TOOLKIT_WIN

