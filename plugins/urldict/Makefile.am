
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = urldict.la

urldict_la_LIBADD = \
	xp/libxp.la

urldict_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

urldict_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	urldict.dll

CLEANFILES = \
	urldict.def \
	urldict.dll \
	urldict.dll.a

urldict.dll: .libs/urldict.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

