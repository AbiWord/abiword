
noinst_LTLIBRARIES = libgtk.la

# TODO: refactor so we can get away using only AF_CPPFLAGS here
AM_CPPFLAGS = \
	$(WP_CPPFLAGS)

conditional_src = \
	ev_X11Keyboard.cpp \
	ev_X11Keyboard.h

if WITH_X
x_src = \
	ev_X11Keyboard.cpp \
	ev_X11Keyboard.h
endif

libgtk_la_SOURCES = \
	$(x_src) \
	ev_UnixFontCombo.cpp \
	ev_UnixFontCombo.h \
	ev_UnixKeyboard.cpp \
	ev_UnixKeyboard.h \
	ev_UnixKeyboardFactory.cpp \
	ev_UnixKeysym2ucs.cpp \
	ev_UnixKeysym2ucs.h \
	ev_UnixMenuBar.h \
	ev_UnixMenu.cpp \
	ev_UnixMenu.h \
	ev_UnixMenuPopup.h \
	ev_UnixMouse.cpp \
	ev_UnixMouse.h \
	ev_UnixToolbar.cpp \
	ev_UnixToolbar.h \
	ev_UnixToolbar_ViewListener.cpp \
	ev_UnixToolbar_ViewListener.h

EXTRA_DIST = \
	$(conditional_src)

