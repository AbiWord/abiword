
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wml.la

wml_la_LIBADD = \
	xp/libxp.la

wml_la_LDFLAGS = \
	$(WML_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

wml_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	wml.dll

CLEANFILES = \
	wml.def \
	wml.dll \
	wml.dll.a

wml.dll: .libs/libwml.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(WML_LIBS)

endif # TOOLKIT_WIN

