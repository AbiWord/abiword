
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = psion.la

psion_la_LIBADD = \
	xp/libxp.la

psion_la_LDFLAGS = \
	$(PSION_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

psion_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	psion.dll

CLEANFILES = \
	psion.def \
	psion.dll \
	psion.dll.a

.libs/psion.a: psion.la

psion.dll: .libs/psion.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(PSION_LIBS)

endif # TOOLKIT_WIN

