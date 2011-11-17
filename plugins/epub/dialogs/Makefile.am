DIST_SUBDIRS = gtk win xp

noinst_LTLIBRARIES = libdialogs.la

if TOOLKIT_GTK2
SUBDIRS = gtk xp
libdialogs_la_LIBADD = gtk/libgtk.la xp/libxp.la
else
SUBDIRS = @TOOLKIT@ xp
libdialogs_la_LIBADD = @TOOLKIT@/lib@TOOLKIT@.la xp/libxp.la
endif

libdialogs_la_SOURCES = 
