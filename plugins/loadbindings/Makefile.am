
SUBDIRS = xp

if LOADBINDINGS_BUILTIN
noinst_LTLIBRARIES = loadbindings.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = loadbindings.la

endif

loadbindings_la_LIBADD = \
	xp/libxp.la \
	$(LOADBINDINGS_LIBS)

loadbindings_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

loadbindings_la_SOURCES = 
nodist_EXTRA_loadbindings_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

