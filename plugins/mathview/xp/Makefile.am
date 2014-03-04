
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(MATHVIEW_CFLAGS) \
	-Wno-deprecated \
	-I$(srcdir)/../itex2mml \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	AbiLasemMathView.h \
	AbiLasemMathView.cpp \
	ie_imp_MathML.cpp \
	ie_imp_MathML.h

EXTRA_DIST = \
	entitiesMap.inc

