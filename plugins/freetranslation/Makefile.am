
SUBDIRS = xp

if FREETRANSLATION_BUILTIN
noinst_LTLIBRARIES = freetranslation.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = freetranslation.la

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

endif

freetranslation_la_LIBADD = \
	xp/libxp.la

freetranslation_la_LDFLAGS = \
	$(FREETRANSLATION_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

freetranslation_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

