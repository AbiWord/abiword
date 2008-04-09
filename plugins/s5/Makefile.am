
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = s5.la

s5_la_LIBADD = \
	xp/libxp.la

s5_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

s5_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	s5.dll

CLEANFILES = \
	s5.def \
	s5.dll \
	s5.dll.a

s5.dll: .libs/libs5.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

