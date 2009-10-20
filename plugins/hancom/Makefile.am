
SUBDIRS = xp

if HANCOM_BUILTIN
noinst_LTLIBRARIES = hancom.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = hancom.la

endif

hancom_la_LIBADD = \
	xp/libxp.la

hancom_la_LDFLAGS = \
	$(HANCOM_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

hancom_la_SOURCES = 
nodist_EXTRA_hancom_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

