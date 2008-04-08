
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wpg.la

wpg_la_LIBADD = \
	xp/libxp.la

wpg_la_LDFLAGS = \
	$(WPG_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

wpg_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	wpg.dll

CLEANFILES = \
	wpg.def \
	wpg.dll \
	wpg.dll.a

wpg.dll: .libs/libwpg.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(WPG_LIBS)

endif # TOOLKIT_WIN

