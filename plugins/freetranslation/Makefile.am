
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libfreetranslation.la

libfreetranslation_la_LIBADD = \
	xp/libxp.la

libfreetranslation_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libfreetranslation_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libfreetranslation.dll

CLEANFILES = \
	libfreetranslation.def \
	libfreetranslation.dll \
	libfreetranslation.dll.a

libfreetranslation.dll: .libs/libfreetranslation.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

