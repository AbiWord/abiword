
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = gda.la

gda_la_LIBADD = \
	unix/libunix.la \
	$(GDA_LIBS)

gda_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

gda_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

