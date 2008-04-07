
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wpggfx.la

wpggfx_la_LIBADD = \
	xp/libxp.la

wpggfx_la_LDFLAGS = \
	$(WPGGFX_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

wpggfx_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	wpggfx.dll

CLEANFILES = \
	wpggfx.def \
	wpggfx.dll \
	wpggfx.dll.a

wpggfx.dll: .libs/wpggfx.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(WPGGFX_LIBS)

endif # TOOLKIT_WIN

