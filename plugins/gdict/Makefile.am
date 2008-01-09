
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgdict.la

libgdict_la_LIBADD = \
	unix/libunix.la

libgdict_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgdict_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libgdict.dll

CLEANFILES = \
	libgdict.def \
	libgdict.dll \
	libgdict.dll.a

libgdict.dll: .libs/libgdict.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(GDICT_LIBS)

endif # TOOLKIT_WIN

