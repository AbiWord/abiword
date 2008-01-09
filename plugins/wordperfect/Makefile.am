
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libwordperfect.la

libwordperfect_la_LIBADD = \
	xp/libxp.la

libwordperfect_la_LDFLAGS = \
	$(WORDPERFECT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libwordperfect_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libwordperfect.dll

CLEANFILES = \
	libwordperfect.def \
	libwordperfect.dll \
	libwordperfect.dll.a

libwordperfect.dll: .libs/libwordperfect.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(WORDPERFECT_LIBS)

endif # TOOLKIT_WIN

