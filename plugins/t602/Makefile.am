
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libt602.la

libt602_la_LIBADD = \
	xp/libxp.la

libt602_la_LDFLAGS = \
	$(T602_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libt602_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
