
SUBDIRS = backends core

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = collab.la

collab_la_LIBADD = \
	backends/libbackends.la \
	core/libcore.la

collab_la_LDFLAGS = \
	$(COLLAB_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

collab_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	collab.dll

CLEANFILES = \
	collab.def \
	collab.dll \
	collab.dll.a

collab.dll: .libs/libcollab.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(COLLAB_LIBS)

endif # TOOLKIT_WIN

