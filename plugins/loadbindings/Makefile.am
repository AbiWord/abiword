
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = loadbindings.la

loadbindings_la_LIBADD = \
	xp/libxp.la

loadbindings_la_LDFLAGS = \
	$(LOADBINDINGS_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

loadbindings_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	loadbindings.dll

CLEANFILES = \
	loadbindings.def \
	loadbindings.dll \
	loadbindings.dll.a

loadbindings.dll: .libs/loadbindings.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(LOADBINDINGS_LIBS)

endif # TOOLKIT_WIN

