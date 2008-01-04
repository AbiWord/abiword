
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgoogle.la

libgoogle_la_LIBADD = \
	xp/libxp.la

libgoogle_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgoogle_la_SOURCES = 

