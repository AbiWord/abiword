
noinst_LTLIBRARIES = libgtk.la

# TODO: look at refactoring IE_File* stuff so we'd get away using only AF_CPPFLAGS here
# 	(low priority)
AM_CPPFLAGS = \
	$(IMPEXP_CPPFLAGS)

touch_support = \
	fv_UnixSelectionHandles.cpp \
	fv_UnixSelectionHandles.h \
	gtktexthandle.cpp \
	gtktexthandleprivate.h \
	$(NULL)

libgtk_la_SOURCES = \
	fv_UnixFrameEdit.cpp \
	fv_UnixFrameEdit.h \
	fv_UnixInlineImage.cpp \
	fv_UnixInlineImage.h \
	fv_UnixVisualDrag.cpp \
	fv_UnixVisualDrag.h \
	$(touch_support) \
	$(NULL)
