
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libwml.la

libwml_la_LIBADD = \
	xp/libxp.la

libwml_la_LDFLAGS = \
	$(WML_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libwml_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
