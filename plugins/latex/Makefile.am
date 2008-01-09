
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = liblatex.la

liblatex_la_LIBADD = \
	xp/libxp.la

liblatex_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

liblatex_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	liblatex.dll

CLEANFILES = \
	liblatex.def \
	liblatex.dll \
	liblatex.dll.a

liblatex.dll: .libs/liblatex.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

