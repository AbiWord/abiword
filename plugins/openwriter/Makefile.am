
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libopenwriter.la

libopenwriter_la_LIBADD = \
	xp/libxp.la

libopenwriter_la_LDFLAGS = \
	$(OPENWRITER_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libopenwriter_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libopenwriter.dll

CLEANFILES = \
	libopenwriter.def \
	libopenwriter.dll \
	libopenwriter.dll.a

libopenwriter.dll: .libs/libopenwriter.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(OPENWRITER_LIBS)

endif # TOOLKIT_WIN

