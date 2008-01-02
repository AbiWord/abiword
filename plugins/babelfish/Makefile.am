
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libbabelfish.la

libbabelfish_la_LIBADD = \
	xp/libxp.la

libbabelfish_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libbabelfish_la_SOURCES = 

