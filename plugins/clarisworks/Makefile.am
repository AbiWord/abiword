
SUBDIRS = xp

if CLARISWORKS_BUILTIN
noinst_LTLIBRARIES = clarisworks.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = clarisworks.la

endif

clarisworks_la_LIBADD = \
	xp/libxp.la

clarisworks_la_LDFLAGS = \
	$(CLARISWORKS_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

clarisworks_la_SOURCES = 

nodist_EXTRA_clarisworks_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

