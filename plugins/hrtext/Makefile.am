
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libhrtext.la

libhrtext_la_LIBADD = \
	xp/libxp.la

libhrtext_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libhrtext_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libhrtext.dll

CLEANFILES = \
	libhrtext.def \
	libhrtext.dll \
	libhrtext.dll.a

libhrtext.dll: .libs/libhrtext.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

