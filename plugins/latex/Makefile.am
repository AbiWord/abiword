
if HAVE_LIBXSLT
SUBDIRS = xp xsltml
else
SUBDIRS = xp
endif

if LATEX_BUILTIN
noinst_LTLIBRARIES = latex.la

else

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = latex.la

endif

latex_la_LIBADD = \
	xp/libxp.la \
	$(LATEX_LIBS)

latex_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined
if TOOLKIT_COCOA
latex_la_LDFLAGS += -headerpad_max_install_names
endif # TOOLKIT_COCOA

latex_la_SOURCES = 
nodist_EXTRA_latex_la_SOURCES = dummy.cpp

if TOOLKIT_COCOA
install-data-hook:
	install_name_tool -change $(ABIWORD_LIBDIR)/libabiword-@ABIWORD_SERIES@.dylib \
		@executable_path/../Frameworks/libabiword-@ABIWORD_SERIES@.dylib \
		$(DESTDIR)/$(plugindir)/latex.so
endif # TOOLKIT_COCOA

EXTRA_DIST = \
	plugin.m4

