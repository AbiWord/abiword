
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = psion.la

psion_la_LIBADD = \
	xp/libxp.la

psion_la_LDFLAGS = \
	$(PSION_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

psion_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

