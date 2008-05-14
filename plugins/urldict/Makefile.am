
SUBDIRS = xp

if URLDICT_BUILTIN
noinst_LTLIBRARIES = urldict.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = urldict.la

if TOOLKIT_WIN

plugin_DATA = \
	urldict.dll

CLEANFILES = \
	urldict.def \
	urldict.dll \
	urldict.dll.a

urldict.dll: .libs/liburldict.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

endif

urldict_la_LIBADD = \
	xp/libxp.la

urldict_la_LDFLAGS = \
	$(URLDICT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

urldict_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

