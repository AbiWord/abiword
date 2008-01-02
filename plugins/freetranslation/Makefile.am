
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libfreetranslation.la

libfreetranslation_la_LIBADD = \
	xp/libxp.la

libfreetranslation_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libfreetranslation_la_SOURCES = 

