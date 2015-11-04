SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = testharness.la

testharness_la_LIBADD = \
	xp/libxp.la \
	$(COMMAND_LIBS)

testharness_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	$(top_builddir)/src/af/tf/xp/libxp.la

testharness_la_SOURCES =

