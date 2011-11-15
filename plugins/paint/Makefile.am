
DIST_SUBDIRS = unix win xp
SUBDIRS = @PLATFORM@ xp

if PAINT_BUILTIN
noinst_LTLIBRARIES = paint.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = paint.la

endif

paint_la_LIBADD = \
	@PLATFORM@/lib@PLATFORM@.la \
	xp/libxp.la \
	$(PAINT_LIBS)

paint_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

paint_la_SOURCES = 
nodist_EXTRA_paint_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

