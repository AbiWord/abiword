if WITH_CAIRO

SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = mathview.la

mathview_la_LIBADD = \
	xp/libxp.la \
	$(MATHVIEW_LIBS)

mathview_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

nodist_EXTRA_mathview_la_SOURCES = dummy.cpp
mathview_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

endif
