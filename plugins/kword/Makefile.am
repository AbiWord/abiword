
SUBDIRS = xp

if KWORD_BUILTIN
noinst_LTLIBRARIES = kword.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = kword.la

endif

kword_la_LIBADD = \
	xp/libxp.la \
	$(KWORD_LIBS)

kword_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

kword_la_SOURCES = 
nodist_EXTRA_kword_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

