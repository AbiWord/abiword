
SUBDIRS = xp

if PDB_BUILTIN
noinst_LTLIBRARIES = pdb.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = pdb.la

endif

pdb_la_LIBADD = \
	xp/libxp.la \
	$(PDB_LIBS)

pdb_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

pdb_la_SOURCES = 
nodist_EXTRA_pdb_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

