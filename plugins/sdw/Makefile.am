
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libsdw.la

libsdw_la_LIBADD = \
	xp/libxp.la

libsdw_la_LDFLAGS = \
	$(SDW_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libsdw_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
