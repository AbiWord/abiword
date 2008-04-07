
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = t602.la

t602_la_LIBADD = \
	xp/libxp.la

t602_la_LDFLAGS = \
	$(T602_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

t602_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	t602.dll

CLEANFILES = \
	t602.def \
	t602.dll \
	t602.dll.a

t602.dll: .libs/libt602.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(T602_LIBS)

endif # TOOLKIT_WIN

