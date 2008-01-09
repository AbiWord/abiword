
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libt602.la

libt602_la_LIBADD = \
	xp/libxp.la

libt602_la_LDFLAGS = \
	$(T602_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libt602_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libt602.dll

CLEANFILES = \
	libt602.def \
	libt602.dll \
	libt602.dll.a

libt602.dll: .libs/libt602.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(T602_LIBS)

endif # TOOLKIT_WIN

