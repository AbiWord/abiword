
SUBDIRS = xp

if KWORD_BUILTIN
noinst_LTLIBRARIES = kword.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = kword.la

if TOOLKIT_WIN

plugin_DATA = \
	kword.dll

CLEANFILES = \
	kword.def \
	kword.dll \
	kword.dll.a

kword.dll: .libs/libkword.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(KWORD_LIBS)

endif # TOOLKIT_WIN

endif

kword_la_LIBADD = \
	xp/libxp.la

kword_la_LDFLAGS = \
	$(KWORD_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

kword_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

