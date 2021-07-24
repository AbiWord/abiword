SUBDIRS = xp

if !TOOLKIT_WIN

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = testharness.la

testharness_la_LIBADD = \
	xp/libxp.la \
	$(COMMAND_LIBS)

testharness_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-L$(top_builddir)/src/af/tf -labiword-@ABIWORD_SERIES@-test \
	$(null)

testharness_la_SOURCES =

endif
