
DIST_SUBDIRS = unix win xp
SUBDIRS = @PLATFORM@ xp

noinst_LTLIBRARIES = libservice.la

libservice_la_LIBADD = xp/libxp.la @PLATFORM@/lib@PLATFORM@.la

nodist_EXTRA_libservice_la_SOURCES = dummy.cpp
libservice_la_SOURCES = 

certdir=$(ABIWORD_DATADIR)/certs
cert_DATA = \
        cacert.pem

EXTRA_DIST = \
        $(cert_DATA)
