
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = liburldict.la

liburldict_la_LIBADD = \
	xp/libxp.la

liburldict_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

liburldict_la_SOURCES = 

