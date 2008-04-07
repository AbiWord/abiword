
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = eml.la

eml_la_LIBADD = \
	xp/libxp.la

eml_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

eml_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	eml.dll

CLEANFILES = \
	eml.def \
	eml.dll \
	eml.dll.a

eml.dll: .libs/libeml.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

