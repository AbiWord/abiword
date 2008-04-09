
DIST_SUBDIRS = $(ALL_PLUGINS)
SUBDIRS = $(PLUGINS)

noinst_LTLIBRARIES = libplugins.la

libplugins_la_CPPFLAGS = \
	$(AF_CPPFLAGS)

libplugins_la_LIBADD = $(foreach plugin, $(BUILTIN_PLUGINS), $(plugin)/$(plugin).la)

libplugins_la_SOURCES = \
	abi-builtin-plugins.cpp \
	abi-builtin-plugins.h

libplugins_la_DEPENDENCIES = \
	abi-builtin-plugins-calls.inc \
	abi-builtin-plugins-decls.inc

BUILT_SOURCES = \
	abi-builtin-plugins-calls.inc \
	abi-builtin-plugins-decls.inc

CLEANFILES = \
	$(BUILT_SOURCES)

abi-builtin-plugins-calls.inc: Makefile
	echo "" > $@
	for plugin in $(BUILTIN_PLUGINS); do							\
		echo "abipgn_"$$plugin"_register(pmi);" >> $@;					\
	done

abi-builtin-plugins-decls.inc: Makefile
	echo "" > $@
	for plugin in $(BUILTIN_PLUGINS); do							\
		echo "extern \"C\" int abipgn_"$$plugin"_register(XAP_ModuleInfo *);" >> $@;	\
	done

