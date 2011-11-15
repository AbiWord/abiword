
SUBDIRS = xp

if WML_BUILTIN
noinst_LTLIBRARIES = wml.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wml.la

endif

wml_la_LIBADD = \
	xp/libxp.la \
	$(WML_LIBS)

wml_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

wml_la_SOURCES = 
nodist_EXTRA_wml_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

