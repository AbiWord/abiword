
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libhrtext.la

libhrtext_la_LIBADD = \
	xp/libxp.la

libhrtext_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libhrtext_la_SOURCES = 

