
SUBDIRS = backends core

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libcollab.la

libcollab_la_LIBADD = \
	backends/libbackends.la \
	core/libcore.la

libcollab_la_LDFLAGS = \
	$(COLLAB_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libcollab_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libcollab.dll

CLEANFILES = \
	libcollab.def \
	libcollab.dll \
	libcollab.dll.a

libcollab.dll: .libs/libcollab.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(COLLAB_LIBS)

endif # TOOLKIT_WIN

