
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = ots.la

ots_la_LIBADD = \
	xp/libxp.la

ots_la_LDFLAGS = \
	$(OTS_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

ots_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	ots.dll

CLEANFILES = \
	ots.def \
	ots.dll \
	ots.dll.a

ots.dll: .libs/ots.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(OTS_LIBS)

endif # TOOLKIT_WIN

