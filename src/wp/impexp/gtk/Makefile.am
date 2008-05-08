
noinst_LTLIBRARIES = libgtk.la

# The pdf exporter wants to call EditMethods, so we need WP_CPPFLAGS
AM_CPPFLAGS = \
	$(WP_CPPFLAGS)

# list all conditional source files here so they end up in the dist tarball
conditional_src = \
	ie_exp_PDF.cpp

if ENABLE_PRINT
print_src = ie_exp_PDF.cpp
endif

libgtk_la_SOURCES = \
	$(print_src) \
	ie_exp_PDF.h \
	ie_impexp_RegisterUnix.cpp \
	ie_impGraphic_GdkPixbuf.cpp \
	ie_impGraphic_GdkPixbuf.h

EXTRA_DIST = \
	$(conditional_src)

