
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wpg.la

wpg_la_LIBADD = \
	xp/libxp.la

wpg_la_LDFLAGS = \
	$(WPG_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

wpg_la_SOURCES = 
nodist_EXTRA_wpg_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

