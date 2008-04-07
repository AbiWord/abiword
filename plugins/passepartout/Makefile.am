
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = passepartout.la

passepartout_la_LIBADD = \
	xp/libxp.la

passepartout_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

passepartout_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	passepartout.dll

CLEANFILES = \
	passepartout.def \
	passepartout.dll \
	passepartout.dll.a

passepartout.dll: .libs/libpassepartout.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

