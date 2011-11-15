
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = mht.la

mht_la_LIBADD = \
	xp/libxp.la \
	$(MHT_LIBS)

mht_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

mht_la_SOURCES = 
nodist_EXTRA_mht_la_SOURCES =  dummy.cpp

EXTRA_DIST = \
	plugin.m4

