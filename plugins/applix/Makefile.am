
SUBDIRS = xp

if APPLIX_BUILTIN
noinst_LTLIBRARIES = applix.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = applix.la

endif # APPLIX_BUILTIN

applix_la_LIBADD = \
	xp/libxp.la \
	$(APPLIX_LIBS)

applix_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

applix_la_SOURCES = 

# needed to call the C++ ld driver
nodist_EXTRA_applix_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

