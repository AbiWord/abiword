
if TOOLKIT_GTK_ALL

SUBDIRS = unix

if GDICT_BUILTIN
noinst_LTLIBRARIES = gdict.la
else
plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = gdict.la

endif

gdict_la_LIBADD = \
	unix/libunix.la \
	$(GDICT_LIBS)

gdict_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

gdict_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

endif
