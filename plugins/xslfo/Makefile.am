
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = xslfo.la

xslfo_la_LIBADD = \
	xp/libxp.la

xslfo_la_LDFLAGS = \
	$(XSLFO_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

xslfo_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	xslfo.dll

CLEANFILES = \
	xslfo.def \
	xslfo.dll \
	xslfo.dll.a

xslfo.dll: .libs/libxslfo.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(XSLFO_LIBS)

endif # TOOLKIT_WIN

