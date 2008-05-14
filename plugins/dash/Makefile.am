
# TODO this is not really XP, but unix only

SUBDIRS = xp

if DASH_BUILTIN
noinst_LTLIBRARIES = dash.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = dash.la

if TOOLKIT_WIN

plugin_DATA = \
	dash.dll

CLEANFILES = \
	dash.def \
	dash.dll \
	dash.dll.a

dash.dll: .libs/libdash.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

endif

dash_la_LIBADD = \
	xp/libxp.la

dash_la_LDFLAGS = \
	$(DASH_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

dash_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

