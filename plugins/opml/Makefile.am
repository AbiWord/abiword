
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libopml.la

libopml_la_LIBADD = \
	xp/libxp.la

libopml_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libopml_la_SOURCES = 

