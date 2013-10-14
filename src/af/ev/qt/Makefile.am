
include ${top_srcdir}/moc.mk

noinst_LTLIBRARIES = libqt.la

# TODO: refactor so we can get away using only AF_CPPFLAGS here
AM_CPPFLAGS = \
	$(WP_CPPFLAGS)

$(libqt_la_OBJECTS) : \
	moc_ev_QtMenuSlot.cpp \
	$(NULL)

libqt_la_SOURCES = \
	ev_QtKeyboard.cpp \
	ev_QtKeyboard.h \
	ev_QtMouse.cpp \
	ev_QtMouse.h \
	ev_QtMenu.cpp \
	ev_QtMenu.h \
	ev_QtMenuBar.cpp \
	ev_QtMenuBar.h \
	ev_QtToolbar.cpp \
	ev_QtToolbar.h \
	ev_QtMenuSlot.cpp \
	ev_QtMenuSlot.h \
	$(NULL)
#	ev_UnixFontCombo.cpp \
#	ev_UnixFontCombo.h \
#	ev_UnixKeysym2ucs.cpp \
#	ev_UnixKeysym2ucs.h \
#	ev_UnixMenuPopup.h \
#	ev_UnixToolbar_ViewListener.cpp \
#	ev_UnixToolbar_ViewListener.h

