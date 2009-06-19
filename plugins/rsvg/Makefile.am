
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = rsvg.la

rsvg_la_LIBADD = \
	xp/libxp.la \
	$(RSVG_LIBS) -lpng12

rsvg_la_LDFLAGS = \
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

.libs/rsvg.a: rsvg.la

rsvg.dll: .libs/rsvg.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(RSVG_LIBS) -lpng12

endif # TOOLKIT_WIN

