
SUBDIRS = xp

if OPENWRITER_BUILTIN
noinst_LTLIBRARIES = openwriter.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = openwriter.la

endif

openwriter_la_LIBADD = \
	xp/libxp.la

openwriter_la_LDFLAGS = \
	$(OPENWRITER_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

openwriter_la_SOURCES = 
nodist_EXTRA_openwriter_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

