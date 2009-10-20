
SUBDIRS = common imp exp

if OPENXML_BUILTIN
noinst_LTLIBRARIES = openxml.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = openxml.la

endif

openxml_la_LIBADD = \
	common/libcommon.la \
	imp/libimp.la \
	exp/libexp.la

openxml_la_LDFLAGS = \
	$(OPENXML_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

openxml_la_SOURCES = 
nodist_EXTRA_openxml_la_SOURCES =  dummy.cpp

EXTRA_DIST = \
	plugin.m4

