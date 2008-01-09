
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libpassepartout.la

libpassepartout_la_LIBADD = \
	xp/libxp.la

libpassepartout_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libpassepartout_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libpassepartout.dll

CLEANFILES = \
	libpassepartout.def \
	libpassepartout.dll \
	libpassepartout.dll.a

libpassepartout.dll: .libs/libpassepartout.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

