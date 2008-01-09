
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = liburldict.la

liburldict_la_LIBADD = \
	xp/libxp.la

liburldict_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

liburldict_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	liburldict.dll

CLEANFILES = \
	liburldict.def \
	liburldict.dll \
	liburldict.dll.a

liburldict.dll: .libs/liburldict.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

