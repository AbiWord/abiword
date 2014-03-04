AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	-I$(srcdir)/crypto \
	$(OPENDOCUMENT_CFLAGS) \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	crypto/gc-pbkdf2-sha1.c \
	crypto/gc-pbkdf2-sha1.h \
	crypto/hmac-sha1.c \
	crypto/hmac.h \
	crypto/memxor.c \
	crypto/memxor.h \
	crypto/sha1.c \
	crypto/sha1.h \
	ie_impexp_OpenDocument.cpp \
	ODc_Crypto.cpp \
	ODc_Crypto.h \
	ODc_util.cpp \
	ODc_util.h

EXTRA_DIST = \
	crypto/README
