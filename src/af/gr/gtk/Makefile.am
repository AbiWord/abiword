
noinst_LTLIBRARIES = libgtk.la

AM_CPPFLAGS = \
	$(AF_CPPFLAGS)

if TOOLKIT_GTK2
gtk2_support_src = gr_UnixPangoPixmapGraphics.cpp \
	gr_UnixPangoPixmapGraphics.h
else
gtk2_support_src = 
endif

libgtk_la_SOURCES = \
	$(gtk2_support_src) \
	gr_CairoPrintGraphics.cpp \
	gr_CairoPrintGraphics.h \
	gr_UnixCairoGraphics.h \
	gr_UnixCairoGraphics.cpp \
	gr_UnixImage.cpp \
	gr_UnixImage.h \
	gr_CairoImage.h \
	gr_CairoImage.cpp

