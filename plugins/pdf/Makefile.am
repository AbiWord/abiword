
SUBDIRS = xp

if PDF_BUILTIN
noinst_LTLIBRARIES = pdf.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = pdf.la

if TOOLKIT_WIN

plugin_DATA = \
	pdf.dll

CLEANFILES = \
	pdf.def \
	pdf.dll \
	pdf.dll.a

pdf.dll: .libs/libpdf.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(PDF_LIBS)

endif # TOOLKIT_WIN

endif # PDF_BUILTIN

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

