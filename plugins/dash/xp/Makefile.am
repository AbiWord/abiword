
noinst_LTLIBRARIES = libxp.la

libxp_la_CPPFLAGS = \
	$(PLUGIN_CFLAGS) \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	AbiDash.cpp

# ugh, apparently this is included from AbiDash.cpp
noinst_HEADERS = \
	dashboard-frontend.c

