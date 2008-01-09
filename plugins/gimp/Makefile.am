
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgimp.la

libgimp_la_LIBADD = \
	xp/libxp.la

libgimp_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgimp_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libgimp.dll

CLEANFILES = \
	libgimp.def \
	libgimp.dll \
	libgimp.dll.a

libgimp.dll: .libs/libgimp.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

