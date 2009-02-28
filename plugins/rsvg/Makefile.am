
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = rsvg.la

rsvg_la_LIBADD = \
	unix/libunix.la

rsvg_la_LDFLAGS = \
	$(RSVG_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

rsvg_la_SOURCES = 
nodist_EXTRA_rsvg_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	rsvg.dll

CLEANFILES = \
	rsvg.def \
	rsvg.dll \
	rsvg.dll.a

rsvg.dll: .libs/librsvg.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(RSVG_LIBS)

endif # TOOLKIT_WIN

