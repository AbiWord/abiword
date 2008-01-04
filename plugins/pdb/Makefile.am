
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
