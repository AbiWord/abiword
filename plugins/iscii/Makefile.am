
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libiscii.la

libiscii_la_LIBADD = \
	xp/libxp.la

libiscii_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libiscii_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libiscii.dll

CLEANFILES = \
	libiscii.def \
	libiscii.dll \
	libiscii.dll.a

libiscii.dll: .libs/libiscii.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

