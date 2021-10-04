
AM_CPPFLAGS = \
	$(AF_CPPFLAGS)

if TOOLKIT_COCOA
# Do not install on macOS. XXX figure out how this can be run.
noinst_LTLIBRARIES = libabiword-@ABIWORD_SERIES@-test.la
else
lib_LTLIBRARIES = libabiword-@ABIWORD_SERIES@-test.la
endif

libabiword_@ABIWORD_SERIES@_test_la_LDFLAGS = \
	-no-undefined

libabiword_@ABIWORD_SERIES@_test_la_SOURCES = \
	xp/tf_test.cpp \
	xp/tf_test.h \
	$(null)
