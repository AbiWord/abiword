
SUBDIRS = xp

if FREETRANSLATION_BUILTIN
noinst_LTLIBRARIES = freetranslation.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = freetranslation.la

endif

freetranslation_la_LIBADD = \
	xp/libxp.la \
	$(FREETRANSLATION_LIBS)

freetranslation_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

freetranslation_la_SOURCES = 

nodist_EXTRA_freetranslation_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

