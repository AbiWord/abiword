
SUBDIRS = xp

if CLARISWORKS_BUILTIN
noinst_LTLIBRARIES = clarisworks.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = clarisworks.la

if TOOLKIT_WIN

plugin_DATA = \
	clarisworks.dll

CLEANFILES = \
	clarisworks.def \
	clarisworks.dll \
	clarisworks.dll.a

clarisworks.dll: .libs/libclarisworks.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(CLARISWORKS_LIBS)

endif # TOOLKIT_WIN

endif

clarisworks_la_LIBADD = \
	xp/libxp.la

clarisworks_la_LDFLAGS = \
	$(CLARISWORKS_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

clarisworks_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

