
SUBDIRS = linkgrammarwrap xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = grammar.la

grammar_la_LIBADD = \
	linkgrammarwrap/liblinkgrammarwrap.la \
	xp/libxp.la

grammar_la_LDFLAGS = \
	$(GRAMMAR_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

nodist_EXTRA_grammar_la_SOURCES = dummy.cpp
grammar_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
