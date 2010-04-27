
SUBDIRS = xp

if DOCBOOK_BUILTIN
noinst_LTLIBRARIES = docbook.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = docbook.la

endif

docbook_la_LIBADD = \
	xp/libxp.la

docbook_la_LDFLAGS = \
	$(DOCBOOK_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

docbook_la_SOURCES = 

nodist_EXTRA_docbook_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

