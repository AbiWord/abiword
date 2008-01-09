
DIST_SUBDIRS = win xp
SUBDIRS = xp

if TOOLKIT_WIN
SUBDIRS += win
platform_lib = win/libwin.la
endif

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libaiksaurus.la

libaiksaurus_la_LIBADD = \
	$(platform_lib) \
	xp/libxp.la

libaiksaurus_la_LDFLAGS = \
	$(AIKSAURUS_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libaiksaurus_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libaiksaurus.dll

CLEANFILES = \
	libaiksaurus.def \
	libaiksaurus.dll \
	libaiksaurus.dll.a

libaiksaurus.dll: .libs/libaiksaurus.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(AIKSAURUS_LIBS)

endif # TOOLKIT_WIN

