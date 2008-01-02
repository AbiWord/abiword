
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libwmf.la

libwmf_la_LIBADD = \
	xp/libxp.la

libwmf_la_LDFLAGS = \
	$(WMF_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libwmf_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
