
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = librsvg.la

librsvg_la_LIBADD = \
	unix/libunix.la

librsvg_la_LDFLAGS = \
	$(RSVG_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

librsvg_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
