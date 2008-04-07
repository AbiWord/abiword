
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = presentation.la

presentation_la_LIBADD = \
	xp/libxp.la

presentation_la_LDFLAGS = \
	$(PRESENTATION_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

presentation_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	presentation.dll

CLEANFILES = \
	presentation.def \
	presentation.dll \
	presentation.dll.a

presentation.dll: .libs/presentation.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PRESENTATION_LIBS)

endif # TOOLKIT_WIN

