
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libloadbindings.la

libloadbindings_la_LIBADD = \
	xp/libxp.la

libloadbindings_la_LDFLAGS = \
	$(LOADBINDINGS_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libloadbindings_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

