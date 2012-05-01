
noinst_LTLIBRARIES = libplugins.la

AM_CPPFLAGS = \
	$(AF_CPPFLAGS)

# the last list of builtin plugins
BUILTIN_STAMP = builtin-stamp

# builtin plugins' la files
libplugins_la_LIBADD = $(BUILTIN_PLUGINS_LA)

libplugins_la_SOURCES = \
	abi-builtin-plugins.cpp \
	abi-builtin-plugins.h

libplugins_la_DEPENDENCIES = \
	abi-builtin-plugins-calls.inc \
	abi-builtin-plugins-decls.inc \
	builtin-stamp

BUILT_SOURCES = \
	abi-builtin-plugins-calls.inc \
	abi-builtin-plugins-decls.inc

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(BUILTIN_STAMP)

# visit the plugins we're going to link statically and build them
all-local:
	@for plugin in $(BUILTIN_PLUGINS); do	\
		(cd ../../plugins/$$plugin && $(MAKE) $(AM_MAKEFLAGS) all); \
	done

$(BUILTIN_STAMP):
	@echo $(BUILTIN_PLUGINS) > $(BUILTIN_STAMP)

abi-builtin-plugins-calls.inc: Makefile all-local
	@if test ! -e $(BUILTIN_STAMP) -o "$(BUILTIN_PLUGINS)" != "`cat $(BUILTIN_STAMP)`" ; then \
	echo "" > $@ ; \
	for plugin in $(BUILTIN_PLUGINS); do \
		echo "abipgn_"$$plugin"_register(pmi);" >> $@;	\
	done \
	fi

abi-builtin-plugins-decls.inc: Makefile all-local
	@if test ! -e $(BUILTIN_STAMP) -o "$(BUILTIN_PLUGINS)" != "`cat $(BUILTIN_STAMP)`" ; then \
	echo "" > $@ ; \
	for plugin in $(BUILTIN_PLUGINS); do \
		echo "extern \"C\" int abipgn_"$$plugin"_register(XAP_ModuleInfo *);" >> $@;	\
	done \
	fi

