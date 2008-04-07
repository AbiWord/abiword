
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wmfgfx.la

wmfgfx_la_LIBADD = \
	xp/libxp.la

wmfgfx_la_LDFLAGS = \
	$(WMFGFX_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

wmfgfx_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	wmfgfx.dll

CLEANFILES = \
	wmfgfx.def \
	wmfgfx.dll \
	wmfgfx.dll.a

wmfgfx.dll: .libs/libwmfgfx.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(WMFGFX_LIBS)

endif # TOOLKIT_WIN

