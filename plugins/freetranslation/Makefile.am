
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = freetranslation.la

freetranslation_la_LIBADD = \
	xp/libxp.la

freetranslation_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

freetranslation_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	freetranslation.dll

CLEANFILES = \
	freetranslation.def \
	freetranslation.dll \
	freetranslation.dll.a

freetranslation.dll: .libs/libfreetranslation.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

