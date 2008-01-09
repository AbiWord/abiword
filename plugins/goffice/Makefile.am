
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgoffice.la

libgoffice_la_LIBADD = \
	unix/libunix.la

libgoffice_la_LDFLAGS = \
	$(GOFFICE_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgoffice_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libgoffice.dll

CLEANFILES = \
	libgoffice.def \
	libgoffice.dll \
	libgoffice.dll.a

libgoffice.dll: .libs/libgoffice.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(GOFFICE_LIBS)

endif # TOOLKIT_WIN

