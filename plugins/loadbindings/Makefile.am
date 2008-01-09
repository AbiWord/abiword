
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libloadbindings.la

libloadbindings_la_LIBADD = \
	xp/libxp.la

libloadbindings_la_LDFLAGS = \
	$(LOADBINDINGS_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libloadbindings_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libloadbindings.dll

CLEANFILES = \
	libloadbindings.def \
	libloadbindings.dll \
	libloadbindings.dll.a

libloadbindings.dll: .libs/libloadbindings.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(LOADBINDINGS_LIBS)

endif # TOOLKIT_WIN

