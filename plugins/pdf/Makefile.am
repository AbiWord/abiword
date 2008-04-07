
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = pdf.la

pdf_la_LIBADD = \
	xp/libxp.la

pdf_la_LDFLAGS = \
	$(PDF_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

pdf_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	pdf.dll

CLEANFILES = \
	pdf.def \
	pdf.dll \
	pdf.dll.a

pdf.dll: .libs/libpdf.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PDF_LIBS)

endif # TOOLKIT_WIN

