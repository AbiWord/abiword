
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = nroff.la

nroff_la_LIBADD = \
	xp/libxp.la

nroff_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

nroff_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	nroff.dll

CLEANFILES = \
	nroff.def \
	nroff.dll \
	nroff.dll.a

nroff.dll: .libs/nroff.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

