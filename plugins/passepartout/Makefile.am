
SUBDIRS = xp

if PASSEPARTOUT_BUILTIN
noinst_LTLIBRARIES = passepartout.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = passepartout.la

endif

passepartout_la_LIBADD = \
	xp/libxp.la \
	$(PASSEPARTOUT_LIBS)

passepartout_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

passepartout_la_SOURCES = 
nodist_EXTRA_passepartout_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

