
SUBDIRS = xp

if PDB_BUILTIN
noinst_LTLIBRARIES = pdb.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = pdb.la

if TOOLKIT_WIN

plugin_DATA = \
	pdb.dll

CLEANFILES = \
	pdb.def \
	pdb.dll \
	pdb.dll.a

pdb.dll: .libs/libpdb.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PDB_LIBS)

endif # TOOLKIT_WIN

endif

pdb_la_LIBADD = \
	xp/libxp.la

pdb_la_LDFLAGS = \
	$(PDB_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

pdb_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

