
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wordperfect.la

wordperfect_la_LIBADD = \
	xp/libxp.la

wordperfect_la_LDFLAGS = \
	$(WORDPERFECT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

wordperfect_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	wordperfect.dll

CLEANFILES = \
	wordperfect.def \
	wordperfect.dll \
	wordperfect.dll.a

wordperfect.dll: .libs/libwordperfect.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(WORDPERFECT_LIBS)

endif # TOOLKIT_WIN

