
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libnroff.la

libnroff_la_LIBADD = \
	xp/libxp.la

libnroff_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libnroff_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libnroff.dll

CLEANFILES = \
	libnroff.def \
	libnroff.dll \
	libnroff.dll.a

libnroff.dll: .libs/libnroff.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

