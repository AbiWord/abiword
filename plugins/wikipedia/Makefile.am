
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libwikipedia.la

libwikipedia_la_LIBADD = \
	xp/libxp.la

libwikipedia_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libwikipedia_la_SOURCES = 

