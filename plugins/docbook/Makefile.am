
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libdocbook.la

libdocbook_la_LIBADD = \
	xp/libxp.la

libdocbook_la_LDFLAGS = \
	$(DOCBOOK_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libdocbook_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
