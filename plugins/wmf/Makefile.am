
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wmf.la

wmf_la_LIBADD = \
	xp/libxp.la \
	$(WMF_LIBS)

wmf_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

wmf_la_SOURCES = 
nodist_EXTRA_wmf_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

