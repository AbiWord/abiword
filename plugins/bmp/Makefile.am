
SUBDIRS = xp

if BMP_BUILTIN
noinst_LTLIBRARIES = bmp.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = bmp.la

endif

bmp_la_LIBADD = \
	xp/libxp.la

bmp_la_LDFLAGS = \
	$(BMP_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

bmp_la_SOURCES = 
nodist_EXTRA_bmp_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

