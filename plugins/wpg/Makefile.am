
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libwpg.la

libwpg_la_LIBADD = \
	xp/libxp.la

libwpg_la_LDFLAGS = \
	$(WPG_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libwpg_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
