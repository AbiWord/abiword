
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgdict.la

libgdict_la_LIBADD = \
	unix/libunix.la

libgdict_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgdict_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

