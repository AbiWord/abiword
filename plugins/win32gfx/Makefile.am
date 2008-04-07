
SUBDIRS = win

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = win32gfx.la

win32gfx_la_LIBADD = \
	win/libwin.la

win32gfx_la_LDFLAGS = \
	$(WIN32GFX_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

win32gfx_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	win32gfx.dll

CLEANFILES = \
	win32gfx.def \
	win32gfx.dll \
	win32gfx.dll.a

win32gfx.dll: .libs/win32gfx.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(WIN32GFX_LIBS)

endif # TOOLKIT_WIN

