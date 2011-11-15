
SUBDIRS = xp

if EML_BUILTIN
noinst_LTLIBRARIES = eml.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = eml.la

endif

eml_la_LIBADD = \
	xp/libxp.la \
	$(EML_LIBS)

eml_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

eml_la_SOURCES = 
nodist_EXTRA_eml_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

