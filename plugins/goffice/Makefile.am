
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = goffice.la

goffice_la_LIBADD = \
	unix/libunix.la

goffice_la_LDFLAGS = \
	$(GOFFICE_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

goffice_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

