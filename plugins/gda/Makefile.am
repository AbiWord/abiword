
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = gda.la

gda_la_LIBADD = \
	unix/libunix.la

gda_la_LDFLAGS = \
	$(GDA_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

gda_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

