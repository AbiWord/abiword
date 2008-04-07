
DIST_SUBDIRS = unix win xp
SUBDIRS = @PLATFORM@ xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = paint.la

paint_la_LIBADD = \
	@PLATFORM@/lib@PLATFORM@.la \
	xp/libxp.la

paint_la_LDFLAGS = \
	$(PAINT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

paint_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	paint.dll

CLEANFILES = \
	paint.def \
	paint.dll \
	paint.dll.a

paint.dll: .libs/paint.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PAINT_LIBS)

endif # TOOLKIT_WIN

