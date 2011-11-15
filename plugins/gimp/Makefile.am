
SUBDIRS = xp

if GIMP_BUILTIN
noinst_LTLIBRARIES = gimp.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = gimp.la

endif

gimp_la_LIBADD = \
	xp/libxp.la \
	$(GIMP_LIBS)

gimp_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

gimp_la_SOURCES = 
nodist_EXTRA_gimp_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

