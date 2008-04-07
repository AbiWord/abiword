
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = bmp.la

bmp_la_LIBADD = \
	xp/libxp.la

bmp_la_LDFLAGS = \
	$(BMP_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

bmp_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	bmp.dll

CLEANFILES = \
	bmp.def \
	bmp.dll \
	bmp.dll.a

bmp.dll: .libs/bmp.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(BMP_LIBS)

endif # TOOLKIT_WIN

