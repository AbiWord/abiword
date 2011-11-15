
SUBDIRS = xp

if T602_BUILTIN
noinst_LTLIBRARIES = t602.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = t602.la

endif

t602_la_LIBADD = \
	xp/libxp.la \
	$(T602_LIBS)

t602_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

t602_la_SOURCES = 
nodist_EXTRA_t602_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

