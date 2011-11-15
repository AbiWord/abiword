
SUBDIRS = xp

if MIF_BUILTIN
noinst_LTLIBRARIES = mif.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = mif.la

endif

mif_la_LIBADD = \
	xp/libxp.la \
	$(MIF_LIBS)

mif_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

mif_la_SOURCES = 
nodist_EXTRA_mif_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

