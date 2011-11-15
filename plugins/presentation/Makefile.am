
SUBDIRS = xp

if PRESENTATION_BUILTIN
noinst_LTLIBRARIES = presentation.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = presentation.la

endif

presentation_la_LIBADD = \
	xp/libxp.la \
	$(PRESENTATION_LIBS)

presentation_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

presentation_la_SOURCES = 
nodist_EXTRA_presentation_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

