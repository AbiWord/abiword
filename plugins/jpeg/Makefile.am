
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libjpeg.la

libjpeg_la_LIBADD = \
	xp/libxp.la

libjpeg_la_LDFLAGS = \
	$(JPEG_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libjpeg_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libjpeg.dll

CLEANFILES = \
	libjpeg.def \
	libjpeg.dll \
	libjpeg.dll.a

libjpeg.dll: .libs/libjpeg.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(JPEG_LIBS)

endif # TOOLKIT_WIN

