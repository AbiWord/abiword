
SUBDIRS = xp

if ISCII_BUILTIN
noinst_LTLIBRARIES = iscii.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = iscii.la

endif

iscii_la_LIBADD = \
	xp/libxp.la \
	$(ISCII_LIBS)

iscii_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

iscii_la_SOURCES = 
nodist_EXTRA_iscii_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

