
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libhancom.la

libhancom_la_LIBADD = \
	xp/libxp.la

libhancom_la_LDFLAGS = \
	$(HANCOM_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libhancom_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libhancom.dll

CLEANFILES = \
	libhancom.def \
	libhancom.dll \
	libhancom.dll.a

libhancom.dll: .libs/libhancom.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(HANCOM_LIBS)

endif # TOOLKIT_WIN

