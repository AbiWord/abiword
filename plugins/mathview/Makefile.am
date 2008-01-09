
SUBDIRS = data itex2mml xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libmathview.la

libmathview_la_LIBADD = \
	itex2mml/libitex2mml.la \
	xp/libxp.la

libmathview_la_LDFLAGS = \
	$(MATHVIEW_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libmathview_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libmathview.dll

CLEANFILES = \
	libmathview.def \
	libmathview.dll \
	libmathview.dll.a

libmathview.dll: .libs/libmathview.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(MATHVIEW_LIBS)

endif # TOOLKIT_WIN

