
SUBDIRS = xp

if MSWRITE_BUILTIN
noinst_LTLIBRARIES = mswrite.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = mswrite.la

endif

mswrite_la_LIBADD = \
	xp/libxp.la \
	$(MSWRITE_LIBS)

mswrite_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

mswrite_la_SOURCES = 
nodist_EXTRA_mswrite_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

