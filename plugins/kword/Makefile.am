
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libkword.la

libkword_la_LIBADD = \
	xp/libxp.la

libkword_la_LDFLAGS = \
	$(KWORD_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libkword_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libkword.dll

CLEANFILES = \
	libkword.def \
	libkword.dll \
	libkword.dll.a

libkword.dll: .libs/libkword.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(KWORD_LIBS)

endif # TOOLKIT_WIN

