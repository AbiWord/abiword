
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	-I$(srcdir)/crypto \
	-I$(srcdir)/crypto/blowfish \
	$(OPENDOCUMENT_CFLAGS) \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	crypto/blowfish/bf_cfb64.c \
	crypto/blowfish/bf_enc.c \
	crypto/blowfish/bf_locl.h \
	crypto/blowfish/bf_pi.h \
	crypto/blowfish/bf_skey.c \
	crypto/blowfish/blowfish.h \
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
	ODc_Crypto.h

EXTRA_DIST = \
	crypto/blowfish/COPYRIGHT \
	crypto/blowfish/README \
	crypto/README