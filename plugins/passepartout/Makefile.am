
SUBDIRS = xp

if PASSEPARTOUT_BUILTIN
noinst_LTLIBRARIES = passepartout.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = passepartout.la

if TOOLKIT_WIN

plugin_DATA = \
	passepartout.dll

CLEANFILES = \
	passepartout.def \
	passepartout.dll \
	passepartout.dll.a

passepartout.dll: .libs/libpassepartout.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

endif

passepartout_la_LIBADD = \
	xp/libxp.la

passepartout_la_LDFLAGS = \
	$(PASSEPARTOUT_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

passepartout_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

