
SUBDIRS = gtk

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = goffice.la

goffice_la_LIBADD = \
	gtk/libgtk.la \
	$(GOFFICE_LIBS)

goffice_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

goffice_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

