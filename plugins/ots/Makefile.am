
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = ots.la

ots_la_LIBADD = \
	xp/libxp.la \
	$(OTS_LIBS)

ots_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

ots_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

