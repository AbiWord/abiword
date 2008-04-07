
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = jpeggfx.la

jpeggfx_la_LIBADD = \
	xp/libxp.la

jpeggfx_la_LDFLAGS = \
	$(JPEGGFX_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

jpeggfx_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	jpeggfx.dll

CLEANFILES = \
	jpeggfx.def \
	jpeggfx.dll \
	jpeggfx.dll.a

jpeggfx.dll: .libs/libjpeggfx.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(JPEGGFX_LIBS)

endif # TOOLKIT_WIN

