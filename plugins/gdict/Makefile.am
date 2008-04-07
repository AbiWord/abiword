
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = gdict.la

gdict_la_LIBADD = \
	unix/libunix.la

gdict_la_LDFLAGS = \
	$(GDICT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

gdict_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

