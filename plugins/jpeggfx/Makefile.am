
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libjpeggfx.la

libjpeggfx_la_LIBADD = \
	xp/libxp.la

libjpeggfx_la_LDFLAGS = \
	$(JPEGGFX_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libjpeggfx_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libjpeggfx.dll

CLEANFILES = \
	libjpeggfx.def \
	libjpeggfx.dll \
	libjpeggfx.dll.a

libjpeggfx.dll: .libs/libjpeggfx.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(JPEGGFX_LIBS)

endif # TOOLKIT_WIN

