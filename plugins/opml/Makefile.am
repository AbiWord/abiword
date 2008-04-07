
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = opml.la

opml_la_LIBADD = \
	xp/libxp.la

opml_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

opml_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	opml.dll

CLEANFILES = \
	opml.def \
	opml.dll \
	opml.dll.a

opml.dll: .libs/opml.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

