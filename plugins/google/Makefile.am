
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgoogle.la

libgoogle_la_LIBADD = \
	xp/libxp.la

libgoogle_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgoogle_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libgoogle.dll

CLEANFILES = \
	libgoogle.def \
	libgoogle.dll \
	libgoogle.dll.a

libgoogle.dll: .libs/libgoogle.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

