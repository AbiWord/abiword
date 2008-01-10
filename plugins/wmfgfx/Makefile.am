
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libwmfgfx.la

libwmfgfx_la_LIBADD = \
	xp/libxp.la

libwmfgfx_la_LDFLAGS = \
	$(WMFGFX_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libwmfgfx_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libwmfgfx.dll

CLEANFILES = \
	libwmfgfx.def \
	libwmfgfx.dll \
	libwmfgfx.dll.a

libwmfgfx.dll: .libs/libwmfgfx.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(WMFGFX_LIBS)

endif # TOOLKIT_WIN

