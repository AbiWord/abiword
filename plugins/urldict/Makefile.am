
SUBDIRS = xp

if URLDICT_BUILTIN
noinst_LTLIBRARIES = urldict.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = urldict.la

endif

urldict_la_LIBADD = \
	xp/libxp.la

urldict_la_LDFLAGS = \
	$(URLDICT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

urldict_la_SOURCES = 
nodist_EXTRA_urldict_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

