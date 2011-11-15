
if TOOLKIT_COCOA
SUBDIRS =
else
SUBDIRS = backends core

if TOOLKIT_WIN
SYSTEM_LIBS=-lws2_32 -lmswsock -lcomdlg32 -lgdi32 -lcomctl32
else
SYSTEM_LIBS=
endif

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = collab.la

collab_la_LIBADD = \
	backends/libbackends.la \
	core/libcore.la \
	$(COLLAB_LIBS) \
	$(SYSTEM_LIBS)

collab_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

collab_la_SOURCES = 
nodist_EXTRA_collab_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

endif # TOOLKIT_COCOA
