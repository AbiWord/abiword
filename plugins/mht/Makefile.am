
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libkmht.la

libkmht_la_LIBADD = \
	xp/libxp.la

libkmht_la_LDFLAGS = \
	$(MHT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libkmht_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
