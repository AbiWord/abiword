
SUBDIRS = win

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libwin32gfx.la

libwin32gfx_la_LIBADD = \
	win/libwin.la

libwin32gfx_la_LDFLAGS = \
	$(WIN32GFX_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libwin32gfx_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libwin32gfx.dll

CLEANFILES = \
	libwin32gfx.def \
	libwin32gfx.dll \
	libwin32gfx.dll.a

libwin32gfx.dll: .libs/libwin32gfx.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(WIN32GFX_LIBS)

endif # TOOLKIT_WIN

