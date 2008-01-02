
SUBDIRS = linkgrammarwrap xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgrammar.la

libgrammar_la_LIBADD = \
	linkgrammarwrap/liblinkgrammarwrap.la \
	xp/libxp.la

libgrammar_la_LDFLAGS = \
	$(GRAMMAR_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgrammar_la_SOURCES = 

