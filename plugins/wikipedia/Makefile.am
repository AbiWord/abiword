
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wikipedia.la

wikipedia_la_LIBADD = \
	xp/libxp.la

wikipedia_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

wikipedia_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	wikipedia.dll

CLEANFILES = \
	wikipedia.def \
	wikipedia.dll \
	wikipedia.dll.a

wikipedia.dll: .libs/wikipedia.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

