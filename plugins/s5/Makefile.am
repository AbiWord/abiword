
SUBDIRS = xp

if S5_BUILTIN
noinst_LTLIBRARIES = s5.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = s5.la

endif

s5_la_LIBADD = \
	xp/libxp.la \
	$(S5_LIBS)

s5_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

s5_la_SOURCES = 
nodist_EXTRA_s5_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

