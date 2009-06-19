
SUBDIRS = win

if WIN32GFX_BUILTIN
noinst_LTLIBRARIES = win32gfx.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = win32gfx.la

if TOOLKIT_WIN

plugin_DATA = \
	win32gfx.dll

CLEANFILES = \
	win32gfx.def \
	win32gfx.dll \
	win32gfx.dll.a

.libs/win32gfx.a: win32gfx.la

win32gfx.dll: .libs/win32gfx.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(WIN32GFX_LIBS)

endif # TOOLKIT_WIN

endif

win32gfx_la_LIBADD = \
	win/libwin.la

win32gfx_la_LDFLAGS = \
	$(WIN32GFX_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

win32gfx_la_SOURCES = 
nodist_EXTRA_win32gfx_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

