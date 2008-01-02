
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
