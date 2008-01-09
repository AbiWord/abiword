
SUBDIRS = linkgrammarwrap xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgrammar.la

libgrammar_la_LIBADD = \
	linkgrammarwrap/liblinkgrammarwrap.la \
	xp/libxp.la

libgrammar_la_LDFLAGS = \
	$(GRAMMAR_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgrammar_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libgrammar.dll

CLEANFILES = \
	libgrammar.def \
	libgrammar.dll \
	libgrammar.dll.a

libgrammar.dll: .libs/libgrammar.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(GRAMMAR_LIBS)

endif # TOOLKIT_WIN

