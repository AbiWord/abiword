
SUBDIRS = common exp imp

if OPENDOCUMENT_BUILTIN
noinst_LTLIBRARIES = opendocument.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = opendocument.la

endif # OPENDOCUMENT_BUILTIN

opendocument_la_LIBADD = \
	common/libcommon.la \
	exp/libexp.la \
	imp/libimp.la \
	$(OPENDOCUMENT_LIBS) \
	$(LIBGCRYPT_LIBS)

opendocument_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined
if TOOLKIT_COCOA
opendocument_la_LDFLAGS += -headerpad_max_install_names
endif # TOOLKIT_COCOA

opendocument_la_SOURCES = 

# needed to call the C++ ld driver
nodist_EXTRA_opendocument_la_SOURCES = dummy.cpp

if TOOLKIT_COCOA
install-data-hook:
	install_name_tool -change $(ABIWORD_LIBDIR)/libabiword-@ABIWORD_SERIES@.dylib \
		@executable_path/../Frameworks/libabiword-@ABIWORD_SERIES@.dylib \
		$(DESTDIR)/$(plugindir)/opendocument.so
endif # TOOLKIT_COCOA

EXTRA_DIST = \
	plugin.m4

