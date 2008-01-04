
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libkword.la

libkword_la_LIBADD = \
	xp/libxp.la

libkword_la_LDFLAGS = \
	$(KWORD_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libkword_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
