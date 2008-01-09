
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = librsvg.la

librsvg_la_LIBADD = \
	unix/libunix.la

librsvg_la_LDFLAGS = \
	$(RSVG_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

librsvg_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	librsvg.dll

CLEANFILES = \
	librsvg.def \
	librsvg.dll \
	librsvg.dll.a

librsvg.dll: .libs/librsvg.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(RSVG_LIBS)

endif # TOOLKIT_WIN

