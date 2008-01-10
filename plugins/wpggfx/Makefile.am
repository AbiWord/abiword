
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libwpggfx.la

libwpggfx_la_LIBADD = \
	xp/libxp.la

libwpggfx_la_LDFLAGS = \
	$(WPGGFX_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libwpggfx_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libwpggfx.dll

CLEANFILES = \
	libwpggfx.def \
	libwpggfx.dll \
	libwpggfx.dll.a

libwpggfx.dll: .libs/libwpggfx.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(WPGGFX_LIBS)

endif # TOOLKIT_WIN

