
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libeml.la

libeml_la_LIBADD = \
	xp/libxp.la

libeml_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libeml_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libeml.dll

CLEANFILES = \
	libeml.def \
	libeml.dll \
	libeml.dll.a

libeml.dll: .libs/libeml.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

