
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = liblatex.la

liblatex_la_LIBADD = \
	xp/libxp.la

liblatex_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

liblatex_la_SOURCES = 

