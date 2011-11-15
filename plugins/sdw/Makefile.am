
SUBDIRS = xp

if SDW_BUILTIN
noinst_LTLIBRARIES = sdw.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = sdw.la

endif

sdw_la_LIBADD = \
	xp/libxp.la \
	$(SDW_LIBS)

sdw_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

sdw_la_SOURCES = 
nodist_EXTRA_sdw_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

