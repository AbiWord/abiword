#AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS = af plugins text wp

libabiword_cpp = 
opt_libs = 
platform_ldflags = 
resources = 

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = \
	-v --warn-all --accept-unprefixed \
	--symbol-prefix=abi --symbol-prefix=libabiword
INTROSPECTION_COMPILER_ARGS = \
	--includedir=$(srcdir) \
    --includedir=.

if HAVE_INTROSPECTION
SUBDIRS += gi-overrides
endif

if TOOLKIT_COCOA
main_cpp = $(top_srcdir)/src/wp/main/cocoa/CocoaMain.cpp
include $(top_srcdir)/src/wp/main/cocoa/bundle.mk

install-data-hook:
	install_name_tool \
			-id @executable_path/../Frameworks/libabiword-@ABIWORD_SERIES@.dylib \
			$(DESTDIR)/$(frameworksdir)/libabiword-@ABIWORD_SERIES@.dylib
	install_name_tool -change $(frameworksdir)/libabiword-@ABIWORD_SERIES@.dylib \
		@executable_path/../Frameworks/libabiword-@ABIWORD_SERIES@.dylib \
		$(DESTDIR)/$(macosdir)/AbiWord
endif

if TOOLKIT_GTK_ALL
libabiword_cpp += $(top_srcdir)/src/wp/main/gtk/libabiword.cpp
main_cpp = $(top_srcdir)/src/wp/main/gtk/UnixMain.cpp
endif

if TOOLKIT_QT
libabiword_cpp += $(top_srcdir)/src/wp/main/qt/libabiwordQt.cpp
main_cpp = $(top_srcdir)/src/wp/main/qt/QtMain.cpp
endif

if TOOLKIT_WIN
main_cpp = $(top_srcdir)/src/wp/main/win/Win32Main.cpp
opt_libs += Win32Resources.lo
platform_ldflags += -mwindows -municode
resources += Win32Resources.lo
endif

if TOOLKIT_COCOA
frameworks_LTLIBRARIES = libabiword-@ABIWORD_SERIES@.la
else
lib_LTLIBRARIES = libabiword-@ABIWORD_SERIES@.la
endif

libabiword_@ABIWORD_SERIES@_la_CPPFLAGS = \
	$(WP_CPPFLAGS) \
	-DABIWORD_DATADIR="\"$(ABIWORD_DATADIR)\""

libabiword_@ABIWORD_SERIES@_la_LDFLAGS = \
	$(DEPS_LIBS) \
	$(CHAMPLAIN_LIBS) \
	-no-undefined \
	-avoid-version \
	-export-dynamic


if TOOLKIT_COCOA
libabiword_@ABIWORD_SERIES@_la_LDFLAGS += -headerpad_max_install_names
endif	

libabiword_@ABIWORD_SERIES@_la_LIBADD = \
	plugins/libplugins.la \
	text/fmt/xp/libxp.la \
	text/ptbl/xp/libxp.la \
	wp/ap/$(TOOLKIT)/lib$(TOOLKIT).la \
	wp/ap/xp/libxp.la \
	wp/impexp/xp/libxp.la \
	af/xap/$(TOOLKIT)/lib$(TOOLKIT).la \
	af/xap/xp/libxp.la \
	af/gr/$(TOOLKIT)/lib$(TOOLKIT).la \
	af/gr/xp/libxp.la \
	af/util/$(PLATFORM)/lib$(PLATFORM).la \
	af/util/xp/libxp.la \
	af/ev/$(TOOLKIT)/lib$(TOOLKIT).la \
	af/ev/xp/libxp.la \
	$(opt_libs)

if TOOLKIT_GTK_ALL
libabiword_@ABIWORD_SERIES@_la_LIBADD += \
	wp/impexp/gtk/libgtk.la \
	text/fmt/gtk/libgtk.la

# headers are GTK only.
libabiword_@ABIWORD_SERIES@_includedir = @ABIWORD_HEADERSDIR@
libabiword_@ABIWORD_SERIES@_include_HEADERS = \
	wp/ap/gtk/abiwidget.h \
	wp/main/gtk/libabiword.h \
	af/xap/gtk/xap_UnixTableWidget.h
else
libabiword_@ABIWORD_SERIES@_la_LIBADD += \
	wp/impexp/$(TOOLKIT)/lib$(TOOLKIT).la
endif

libabiword_@ABIWORD_SERIES@_la_DEPENDENCIES = \
	$(libabiword_@ABIWORD_SERIES@_la_LIBADD)

libabiword_@ABIWORD_SERIES@_la_SOURCES = \
	$(top_srcdir)/src/wp/main/xp/abi_ver.cpp \
	$(libabiword_cpp)


# TODO is it possible to track dependencies for that?
WINDRES_CPPFLAGS = -I$(top_srcdir)/src/af/xap/win -I$(top_srcdir)/src/af/util/win -I$(top_srcdir)/src/wp/ap/win
Win32Resources.lo: $(top_srcdir)/src/wp/main/win/Win32Main.rc $(libabiword_@ABIWORD_SERIES@_la_OBJECTS)
	CPPFLAGS="$(WINDRES_CPPFLAGS)" $(top_srcdir)/lt-compile-resource $< $@

bin_PROGRAMS =

if TOOLKIT_COCOA
# MacOS
macos_PROGRAMS = AbiWord

AbiWord_CPPFLAGS = \
	-x objective-c++ \
	$(libabiword_@ABIWORD_SERIES@_la_CPPFLAGS)

AbiWord_LDFLAGS = \
	-no-undefined \
	-avoid-version \
	-export-dynamic \
	-headerpad_max_install_names 

AbiWord_LDADD = \
	libabiword-@ABIWORD_SERIES@.la

AbiWord_SOURCES = \
	$(main_cpp)


else
# not MacOS

bin_PROGRAMS += abiword

abiword_CPPFLAGS = \
	$(libabiword_@ABIWORD_SERIES@_la_CPPFLAGS)

abiword_LDFLAGS = \
	$(platform_ldflags)

abiword_LDADD = \
	libabiword-@ABIWORD_SERIES@.la \
	$(resources)

abiword_DEPENDENCIES = \
	libabiword-@ABIWORD_SERIES@.la \
	$(resources)

abiword_SOURCES = \
	$(main_cpp)

endif # TOOLKIT_COCOA

CLEANFILES = \
	abiword.dll.a


if HAVE_INTROSPECTION

INTROSPECTION_GIRS += Abi-@ABIWORD_SERIES@.gir

Abi-@ABIWORD_SERIES@.gir: $(INTROSPECTION_SCANNER) libabiword-@ABIWORD_SERIES@.la Makefile
Abi_@ABI_SUB_SERIES@_gir_INCLUDES = GObject-2.0 Gtk-3.0 Gsf-1
Abi_@ABI_SUB_SERIES@_gir_CFLAGS = $(libabiword_@ABIWORD_SERIES@_la_CPPFLAGS)
Abi_@ABI_SUB_SERIES@_gir_LIBS = libabiword-@ABIWORD_SERIES@.la
Abi_@ABI_SUB_SERIES@_gir_FILES = \
	af/xap/gtk/xap_UnixTableWidget.h \
	af/xap/gtk/xap_UnixTableWidget.cpp \
	wp/ap/gtk/abiwidget.h \
	wp/ap/gtk/abiwidget.cpp \
	wp/main/gtk/libabiword.h \
	wp/main/gtk/libabiword.cpp

girdir = $(GIRDIR)
gir_DATA = $(INTROSPECTION_GIRS)

typelibsdir = $(TYPELIBDIR)
typelibs_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelibs_DATA)

endif # HAVE_INTROSPECTION

