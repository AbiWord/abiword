
SUBDIRS = xp

if PDF_BUILTIN
noinst_LTLIBRARIES = pdf.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = pdf.la

endif # PDF_BUILTIN

pdf_la_LIBADD = \
	xp/libxp.la \
	$(PDF_LIBS)

pdf_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

pdf_la_SOURCES = 
nodist_EXTRA_pdf_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

