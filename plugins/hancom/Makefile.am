
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = hancom.la

hancom_la_LIBADD = \
	xp/libxp.la

hancom_la_LDFLAGS = \
	$(HANCOM_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

hancom_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	hancom.dll

CLEANFILES = \
	hancom.def \
	hancom.dll \
	hancom.dll.a

hancom.dll: .libs/hancom.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(HANCOM_LIBS)

endif # TOOLKIT_WIN

