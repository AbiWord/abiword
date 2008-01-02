
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libpdf.la

libpdf_la_LIBADD = \
	xp/libxp.la

libpdf_la_LDFLAGS = \
	$(PDF_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libpdf_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
