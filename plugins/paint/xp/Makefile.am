
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(PAINT_CFLAGS) \
	-I$(srcdir)/../$(PLATFORM) \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	AbiGeneric.cpp \
	AbiGeneric.h \
	AbiPaint.cpp \
	AbiPaint.h

