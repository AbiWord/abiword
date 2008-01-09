
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libpdf.la

libpdf_la_LIBADD = \
	xp/libxp.la

libpdf_la_LDFLAGS = \
	$(PDF_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libpdf_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libpdf.dll

CLEANFILES = \
	libpdf.def \
	libpdf.dll \
	libpdf.dll.a

libpdf.dll: .libs/libpdf.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PDF_LIBS)

endif # TOOLKIT_WIN

