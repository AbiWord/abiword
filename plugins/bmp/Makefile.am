
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libbmp.la

libbmp_la_LIBADD = \
	xp/libxp.la

libbmp_la_LDFLAGS = \
	$(BMP_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libbmp_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libbmp.dll

CLEANFILES = \
	libbmp.def \
	libbmp.dll \
	libbmp.dll.a

libbmp.dll: .libs/libbmp.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(BMP_LIBS)

endif # TOOLKIT_WIN

