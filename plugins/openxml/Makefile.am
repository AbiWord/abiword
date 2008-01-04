
SUBDIRS = common imp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libopenxml.la

libopenxml_la_LIBADD = \
	common/libcommon.la \
	imp/libimp.la

libopenxml_la_LDFLAGS = \
	$(OPENXML_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libopenxml_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4
