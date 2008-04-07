
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = babelfish.la

babelfish_la_LIBADD = \
	xp/libxp.la

babelfish_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

babelfish_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	babelfish.dll

CLEANFILES = \
	babelfish.def \
	babelfish.dll \
	babelfish.dll.a

babelfish.dll: .libs/babelfish.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

