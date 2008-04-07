
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = gimp.la

gimp_la_LIBADD = \
	xp/libxp.la

gimp_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

gimp_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	gimp.dll

CLEANFILES = \
	gimp.def \
	gimp.dll \
	gimp.dll.a

gimp.dll: .libs/gimp.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

