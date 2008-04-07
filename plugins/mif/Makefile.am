
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = mif.la

mif_la_LIBADD = \
	xp/libxp.la

mif_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

mif_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	mif.dll

CLEANFILES = \
	mif.def \
	mif.dll \
	mif.dll.a

mif.dll: .libs/libmif.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

