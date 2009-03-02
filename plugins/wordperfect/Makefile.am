
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
nodist_EXTRA_wordperfect_la_SOURCES = dummy.cpp

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
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(WORDPERFECT_LIBS)

endif # TOOLKIT_WIN

