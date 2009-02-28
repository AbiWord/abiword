
SUBDIRS = xp

if S5_BUILTIN
noinst_LTLIBRARIES = s5.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = s5.la

if TOOLKIT_WIN

plugin_DATA = \
	s5.dll

CLEANFILES = \
	s5.def \
	s5.dll \
	s5.dll.a

s5.dll: .libs/libs5.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

endif

s5_la_LIBADD = \
	xp/libxp.la

s5_la_LDFLAGS = \
	$(S5_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

s5_la_SOURCES = 
nodist_EXTRA_s5_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

