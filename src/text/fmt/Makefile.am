
DIST_SUBDIRS = gtk xp
SUBDIRS = xp

noinst_LTLIBRARIES = libfmt.la

libfmt_la_LIBADD = xp/libxp.la

libfmt_la_SOURCES = 

if TOOLKIT_GTK
SUBDIRS += gtk
libfmt_la_LIBADD += gtk/libgtk.la
endif

