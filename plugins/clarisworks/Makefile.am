
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libclarisworks.la

libclarisworks_la_LIBADD = \
	xp/libxp.la

libclarisworks_la_LDFLAGS = \
	$(CLARISWORKS_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libclarisworks_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
