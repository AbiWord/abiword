
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libpdb.la

libpdb_la_LIBADD = \
	xp/libxp.la

libpdb_la_LDFLAGS = \
	$(PDB_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libpdb_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libpdb.dll

CLEANFILES = \
	libpdb.def \
	libpdb.dll \
	libpdb.dll.a

libpdb.dll: .libs/libpdb.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PDB_LIBS)

endif # TOOLKIT_WIN

