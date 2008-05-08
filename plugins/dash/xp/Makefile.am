
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(DASH_CFLAGS) \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	AbiDash.cpp

# ugh, apparently this is included from AbiDash.cpp
noinst_HEADERS = \
	dashboard-frontend.c

