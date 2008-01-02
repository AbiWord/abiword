
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libpassepartout.la

libpassepartout_la_LIBADD = \
	xp/libxp.la

libpassepartout_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libpassepartout_la_SOURCES = 

