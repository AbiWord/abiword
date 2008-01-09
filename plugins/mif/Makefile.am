
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libmif.la

libmif_la_LIBADD = \
	xp/libxp.la

libmif_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libmif_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libmif.dll

CLEANFILES = \
	libmif.def \
	libmif.dll \
	libmif.dll.a

libmif.dll: .libs/libmif.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

