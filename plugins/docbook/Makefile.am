
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libdocbook.la

libdocbook_la_LIBADD = \
	xp/libxp.la

libdocbook_la_LDFLAGS = \
	$(DOCBOOK_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libdocbook_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libdocbook.dll

CLEANFILES = \
	libdocbook.def \
	libdocbook.dll \
	libdocbook.dll.a

libdocbook.dll: .libs/libdocbook.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(DOCBOOK_LIBS)

endif # TOOLKIT_WIN

