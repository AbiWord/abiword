
if HAVE_LIBXSLT
SUBDIRS = xp xsltml
else
SUBDIRS = xp
endif

if LATEX_BUILTIN
noinst_LTLIBRARIES = latex.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = latex.la

endif

latex_la_LIBADD = \
	xp/libxp.la

latex_la_LDFLAGS = \
	$(LATEX_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

latex_la_SOURCES = 
nodist_EXTRA_latex_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

