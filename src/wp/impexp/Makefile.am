
DIST_SUBDIRS = cocoa gtk win xp
if TOOLKIT_GTK2
SUBDIRS = gtk xp
else
SUBDIRS = @TOOLKIT@ xp
endif

noinst_LTLIBRARIES = libimpexp.la

if TOOLKIT_GTK2
libimpexp_la_LIBADD = gtk/libgtk.la xp/libxp.la
else
libimpexp_la_LIBADD = @TOOLKIT@/lib@TOOLKIT@.la xp/libxp.la
endif

libimpexp_la_SOURCES = 

