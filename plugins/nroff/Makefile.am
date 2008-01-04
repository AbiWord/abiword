
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libnroff.la

libnroff_la_LIBADD = \
	xp/libxp.la

libnroff_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libnroff_la_SOURCES = 

