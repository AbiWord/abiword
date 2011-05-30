SUBDIRS = common imp exp

if EPUB_BUILTIN
noinst_LTLIBRARIES = epub.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = epub.la

endif

epub_la_LIBADD = \
	common/libcommon.la \
	imp/libimp.la \
	exp/libexp.la

epub_la_LDFLAGS = \
	$(EPUB_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

epub_la_SOURCES = 
nodist_EXTRA_epub_la_SOURCES =  dummy.cpp

EXTRA_DIST = \
	plugin.m4
