
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wmf.la

wmf_la_LIBADD = \
	xp/libxp.la

wmf_la_LDFLAGS = \
	$(WMF_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

wmf_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	wmf.dll

CLEANFILES = \
	wmf.def \
	wmf.dll \
	wmf.dll.a

wmf.dll: .libs/libwmf.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(WMF_LIBS)

endif # TOOLKIT_WIN

