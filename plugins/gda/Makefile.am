
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgda.la

libgda_la_LIBADD = \
	unix/libunix.la

libgda_la_LDFLAGS = \
	$(GDA_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgda_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

