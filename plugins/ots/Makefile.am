
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libots.la

libots_la_LIBADD = \
	xp/libxp.la

libots_la_LDFLAGS = \
	$(OTS_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libots_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

