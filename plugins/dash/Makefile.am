
# TODO this is not really XP, but unix only

SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libdash.la

libdash_la_LIBADD = \
	xp/libxp.la

libdash_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libdash_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libdash.dll

CLEANFILES = \
	libdash.def \
	libdash.dll \
	libdash.dll.a

libdash.dll: .libs/libdash.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

