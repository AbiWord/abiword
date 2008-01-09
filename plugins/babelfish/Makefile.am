
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libbabelfish.la

libbabelfish_la_LIBADD = \
	xp/libxp.la

libbabelfish_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libbabelfish_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libbabelfish.dll

CLEANFILES = \
	libbabelfish.def \
	libbabelfish.dll \
	libbabelfish.dll.a

libbabelfish.dll: .libs/libbabelfish.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

