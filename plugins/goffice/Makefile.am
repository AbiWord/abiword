
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = goffice.la

goffice_la_LIBADD = \
	unix/libunix.la \
	$(GOFFICE_LIBS)

goffice_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

goffice_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

