
DIST_SUBDIRS = xp gtk
SUBDIRS = @TOOLKIT@ xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = grammar.la

grammar_la_LIBADD = \
	@TOOLKIT@/lib@TOOLKIT@.la	\
	xp/libxp.la

grammar_la_LDFLAGS = \
	$(GRAMMAR_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

grammar_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

