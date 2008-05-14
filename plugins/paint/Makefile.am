
DIST_SUBDIRS = unix win xp
SUBDIRS = @PLATFORM@ xp

if PAINT_BUILTIN
noinst_LTLIBRARIES = paint.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = paint.la

if TOOLKIT_WIN

plugin_DATA = \
	paint.dll

CLEANFILES = \
	paint.def \
	paint.dll \
	paint.dll.a

paint.dll: .libs/libpaint.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(PAINT_LIBS)

endif # TOOLKIT_WIN

endif

paint_la_LIBADD = \
	@PLATFORM@/lib@PLATFORM@.la \
	xp/libxp.la

paint_la_LDFLAGS = \
	$(PAINT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

paint_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

