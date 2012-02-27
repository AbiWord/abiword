
SUBDIRS = af plugins text wp

libabiword_cpp = 
opt_libs = 
platform_ldflags = 
resources = 

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

if TOOLKIT_GTK
libabiword_cpp += $(top_srcdir)/src/wp/main/gtk/libabiword.cpp
main_cpp = $(top_srcdir)/src/wp/main/gtk/UnixMain.cpp
opt_libs += $(GOFFICE_BUILTIN_LIBS)
endif

if TOOLKIT_WIN
main_cpp = $(top_srcdir)/src/wp/main/win/Win32Main.cpp
opt_libs += Win32Resources.lo
platform_ldflags += -mwindows -Wl,--gc-sections -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--out-implib,abiword.dll.a
resources += Win32Resources.lo
endif

if TOOLKIT_COCOA
frameworks_LTLIBRARIES = libabiword-2.8.la
else
lib_LTLIBRARIES = libabiword-2.8.la
endif

libabiword_2_8_la_CPPFLAGS = \
	$(WP_CPPFLAGS) \
	-DABIWORD_DATADIR="\"$(ABIWORD_DATADIR)\""

libabiword_2_8_la_LDFLAGS = \
	$(DEPS_LIBS) \
	-Wl,--no-undefined \
	-avoid-version \
	-export-dynamic


if TOOLKIT_COCOA
libabiword_2_8_la_LDFLAGS += -headerpad_max_install_names
endif	

libabiword_2_8_la_LIBADD = \
	$(opt_libs) \
	plugins/libplugins.la \
	text/fmt/xp/libxp.la \
	text/ptbl/xp/libxp.la \
	wp/ap/$(TOOLKIT)/lib$(TOOLKIT).la \
	wp/ap/xp/libxp.la \
	wp/impexp/$(TOOLKIT)/lib$(TOOLKIT).la \
	wp/impexp/xp/libxp.la \
	af/xap/$(TOOLKIT)/lib$(TOOLKIT).la \
	af/xap/xp/libxp.la \
	af/gr/$(TOOLKIT)/lib$(TOOLKIT).la \
	af/gr/xp/libxp.la \
	af/util/$(PLATFORM)/lib$(PLATFORM).la \
	af/util/xp/libxp.la \
	af/ev/$(TOOLKIT)/lib$(TOOLKIT).la \
	af/ev/xp/libxp.la

if TOOLKIT_GTK
libabiword_2_8_la_LIBADD += text/fmt/gtk/libgtk.la

# headers are GTK only.
libabiword_2_8_includedir = @ABIWORD_HEADERSDIR@
libabiword_2_8_include_HEADERS = \
	wp/ap/gtk/abiwidget.h \
	wp/main/gtk/libabiword.h \
	af/xap/gtk/xap_UnixTableWidget.h
endif

libabiword_2_8_la_DEPENDENCIES = \
	$(libabiword_2_8_la_LIBADD)

libabiword_2_8_la_SOURCES = \
	$(top_srcdir)/src/wp/main/xp/abi_ver.cpp \
	$(libabiword_cpp)


# TODO is it possible to track dependencies for that?
WINDRES_CPPFLAGS = -I$(top_srcdir)/src/af/xap/win -I$(top_srcdir)/src/af/util/win -I$(top_srcdir)/src/wp/ap/win
Win32Resources.lo: $(top_srcdir)/src/wp/main/win/Win32Main.rc $(libabiword_2_8_la_OBJECTS)
	CPPFLAGS="$(WINDRES_CPPFLAGS)" $(top_srcdir)/lt-compile-resource $< $@

bin_PROGRAMS =

if TOOLKIT_COCOA
# MacOS
macos_PROGRAMS = AbiWord

AbiWord_CPPFLAGS = \
	-x objective-c++ \
	$(libabiword_2_8_la_CPPFLAGS)

AbiWord_LDFLAGS = \
	$(DEPS_LIBS) \
	-Wl,--no-undefined \
	-avoid-version \
	-export-dynamic \
	-headerpad_max_install_names 

AbiWord_LDADD = \
	libabiword-2.8.la

AbiWord_SOURCES = \
	$(main_cpp)


else
# not MacOS

bin_PROGRAMS += abiword

abiword_CPPFLAGS = \
	$(libabiword_2_8_la_CPPFLAGS)

abiword_LDFLAGS = \
	$(platform_ldflags) \
	-Wl,--no-undefined      \
	-avoid-version      \
	-export-dynamic

abiword_LDADD = \
	libabiword-2.8.la \
	$(resources) \
	$(DEPS_LIBS)

abiword_DEPENDENCIES = \
	libabiword-2.8.la \
	$(resources)

abiword_SOURCES = \
	$(main_cpp)

endif # TOOLKIT_COCOA

CLEANFILES = \
	abiword.dll.a
