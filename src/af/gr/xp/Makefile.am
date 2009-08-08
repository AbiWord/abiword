
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(AF_CPPFLAGS)

if WITH_CAIRO
CAIRO_SOURCES=gr_CairoGraphics.h \
	gr_CairoGraphics.cpp \
	gr_CairoNullGraphics.h \
	gr_CairoNullGraphics.cpp \
	$(NULL)
else
CAIRO_SOURCES=
endif

libxp_la_SOURCES = \
	gr_Caret.cpp \
	gr_Caret.h \
	gr_CharWidthsCache.cpp \
	gr_CharWidthsCache.h \
	gr_CharWidths.cpp \
	gr_CharWidths.h \
	gr_Control.h \
	gr_DrawArgs.cpp \
	gr_DrawArgs.h \
	gr_EmbedManager.cpp \
	gr_EmbedManager.h \
	gr_Graphics.cpp \
	gr_Graphics.h \
	gr_Image.cpp \
	gr_Image.h \
	gr_Painter.cpp \
	gr_Painter.h \
	gr_RenderInfo.cpp \
	gr_RenderInfo.h \
	gr_Transform.cpp \
	gr_Transform.h \
	gr_VectorImage.cpp \
	gr_VectorImage.h \
	$(CAIRO_SOURCES) \
	$(null)

