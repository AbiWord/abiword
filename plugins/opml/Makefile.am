
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libopml.la

libopml_la_LIBADD = \
	xp/libxp.la

libopml_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libopml_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libopml.dll

CLEANFILES = \
	libopml.def \
	libopml.dll \
	libopml.dll.a

libopml.dll: .libs/libopml.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

