
DIST_SUBDIRS = unix win xp
SUBDIRS = @PLATFORM@ xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libpaint.la

libpaint_la_LIBADD = \
	@PLATFORM@/lib@PLATFORM@.la \
	xp/libxp.la

libpaint_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libpaint_la_SOURCES = 

