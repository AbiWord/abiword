
DIST_SUBDIRS = fake service sugar tcp telepathy xmpp sipsimple
SUBDIRS = 

noinst_LTLIBRARIES = libbackends.la

libbackends_la_LIBADD =  

if COLLAB_BACKEND_FAKE
SUBDIRS += fake
libbackends_la_LIBADD += fake/libfake.la
endif

if COLLAB_BACKEND_SERVICE
SUBDIRS += service
libbackends_la_LIBADD += service/libservice.la
endif

if COLLAB_BACKEND_SUGAR
SUBDIRS += sugar
libbackends_la_LIBADD += sugar/libsugar.la
endif

if COLLAB_BACKEND_TCP
SUBDIRS += tcp
libbackends_la_LIBADD += tcp/libtcp.la
endif

if COLLAB_BACKEND_XMPP
SUBDIRS += xmpp
libbackends_la_LIBADD += xmpp/libxmpp.la
endif

if COLLAB_BACKEND_TELEPATHY
SUBDIRS += telepathy
libbackends_la_LIBADD += telepathy/libtelepathy.la
endif

if COLLAB_BACKEND_SIPSIMPLE
SUBDIRS += sipsimple
libbackends_la_LIBADD += sipsimple/libsipsimple.la
endif

libbackends_la_SOURCES = 

