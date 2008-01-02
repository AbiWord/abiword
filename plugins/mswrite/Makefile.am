
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libmswrite.la

libmswrite_la_LIBADD = \
	xp/libxp.la

libmswrite_la_LDFLAGS = \
	$(MSWRITE_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libmswrite_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
