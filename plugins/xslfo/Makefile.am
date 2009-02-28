
SUBDIRS = xp

if XSLFO_BUILTIN
noinst_LTLIBRARIES = xslfo.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = xslfo.la

if TOOLKIT_WIN

plugin_DATA = \
	xslfo.dll

CLEANFILES = \
	xslfo.def \
	xslfo.dll \
	xslfo.dll.a

xslfo.dll: .libs/libxslfo.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(XSLFO_LIBS)

endif # TOOLKIT_WIN

endif

xslfo_la_LIBADD = \
	xp/libxp.la

xslfo_la_LDFLAGS = \
	$(XSLFO_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

xslfo_la_SOURCES = 
nodist_EXTRA_xslfo_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

