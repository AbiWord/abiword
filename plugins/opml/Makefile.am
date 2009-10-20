
SUBDIRS = xp

if OPML_BUILTIN
noinst_LTLIBRARIES = opml.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = opml.la

endif

opml_la_LIBADD = \
	xp/libxp.la

opml_la_LDFLAGS = \
	$(OPML_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

opml_la_SOURCES = 
nodist_EXTRA_opml_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

