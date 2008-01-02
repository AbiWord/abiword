
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgoffice.la

libgoffice_la_LIBADD = \
	unix/libunix.la

libgoffice_la_LDFLAGS = \
	$(GOFFICE_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgoffice_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
