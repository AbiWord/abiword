
SUBDIRS = xp

if HRTEXT_BUILTIN
noinst_LTLIBRARIES = hrtext.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = hrtext.la

endif

hrtext_la_LIBADD = \
	xp/libxp.la

hrtext_la_LDFLAGS = \
	$(HRTEXT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

hrtext_la_SOURCES = 
nodist_EXTRA_hrtext_la_SOURCES = dummy.cpp 

EXTRA_DIST = \
	plugin.m4

