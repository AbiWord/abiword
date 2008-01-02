
SUBDIRS = unix xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libolpctoolbar.la

libolpctoolbar_la_LIBADD = \
	unix/libunix.la \
	xp/libxp.la

libolpctoolbar_la_LDFLAGS = \
	$(OLPCTOOLBAR_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libolpctoolbar_la_SOURCES = 

