
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = psion.la

psion_la_LIBADD = \
	xp/libxp.la \
	$(PSION_LIBS)

psion_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

psion_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

