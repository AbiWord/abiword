
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libmswrite.la

libmswrite_la_LIBADD = \
	xp/libxp.la

libmswrite_la_LDFLAGS = \
	$(MSWRITE_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libmswrite_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libmswrite.dll

CLEANFILES = \
	libmswrite.def \
	libmswrite.dll \
	libmswrite.dll.a

libmswrite.dll: .libs/libmswrite.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(MSWRITE_LIBS)

endif # TOOLKIT_WIN

