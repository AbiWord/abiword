
SUBDIRS = xp

if WIKIPEDIA_BUILTIN
noinst_LTLIBRARIES = wikipedia.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wikipedia.la

if TOOLKIT_WIN

plugin_DATA = \
	wikipedia.dll

CLEANFILES = \
	wikipedia.def \
	wikipedia.dll \
	wikipedia.dll.a

wikipedia.dll: .libs/libwikipedia.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

endif

wikipedia_la_LIBADD = \
	xp/libxp.la

wikipedia_la_LDFLAGS = \
	$(WIKIPEDIA_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

wikipedia_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

