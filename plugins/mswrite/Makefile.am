
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = mswrite.la

mswrite_la_LIBADD = \
	xp/libxp.la

mswrite_la_LDFLAGS = \
	$(MSWRITE_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

mswrite_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	mswrite.dll

CLEANFILES = \
	mswrite.def \
	mswrite.dll \
	mswrite.dll.a

mswrite.dll: .libs/libmswrite.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(MSWRITE_LIBS)

endif # TOOLKIT_WIN

