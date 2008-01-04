
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libwordperfect.la

libwordperfect_la_LIBADD = \
	xp/libxp.la

libwordperfect_la_LDFLAGS = \
	$(WORDPERFECT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libwordperfect_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
