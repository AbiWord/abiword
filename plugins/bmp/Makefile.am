
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libbmp.la

libbmp_la_LIBADD = \
	xp/libxp.la

libbmp_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libbmp_la_SOURCES = 

