
DIST_SUBDIRS = unix win xp
SUBDIRS = @PLATFORM@ xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libpaint.la

libpaint_la_LIBADD = \
	@PLATFORM@/lib@PLATFORM@.la \
	xp/libxp.la

libpaint_la_LDFLAGS = \
	$(PAINT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libpaint_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libpaint.dll

CLEANFILES = \
	libpaint.def \
	libpaint.dll \
	libpaint.dll.a

libpaint.dll: .libs/libpaint.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PAINT_LIBS)

endif # TOOLKIT_WIN

