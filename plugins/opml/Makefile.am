
SUBDIRS = xp

if OPML_BUILTIN
noinst_LTLIBRARIES = opml.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = opml.la

if TOOLKIT_WIN

plugin_DATA = \
	opml.dll

CLEANFILES = \
	opml.def \
	opml.dll \
	opml.dll.a

opml.dll: .libs/libopml.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

endif

opml_la_LIBADD = \
	xp/libxp.la

opml_la_LDFLAGS = \
	$(OPML_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

opml_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

