
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libxslfo.la

libxslfo_la_LIBADD = \
	xp/libxp.la

libxslfo_la_LDFLAGS = \
	$(XSLFO_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libxslfo_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
