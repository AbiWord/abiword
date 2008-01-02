
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgimp.la

libgimp_la_LIBADD = \
	xp/libxp.la

libgimp_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgimp_la_SOURCES = 

