
# TODO this is not really XP, but unix only

SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = dash.la

dash_la_LIBADD = \
	xp/libxp.la

dash_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

dash_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	dash.dll

CLEANFILES = \
	dash.def \
	dash.dll \
	dash.dll.a

dash.dll: .libs/libdash.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

