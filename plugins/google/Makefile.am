
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = google.la

google_la_LIBADD = \
	xp/libxp.la

google_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

google_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	google.dll

CLEANFILES = \
	google.def \
	google.dll \
	google.dll.a

google.dll: .libs/libgoogle.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

