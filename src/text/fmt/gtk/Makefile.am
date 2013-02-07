
noinst_LTLIBRARIES = libgtk.la

# TODO: look at refactoring IE_File* stuff so we'd get away using only AF_CPPFLAGS here
# 	(low priority)
AM_CPPFLAGS = \
	$(IMPEXP_CPPFLAGS)

cut_and_paste_code = \
	gtktexthandle.cpp \
	gtktexthandleprivate.h

libgtk_la_SOURCES = \
	$(cut_and_paste_code) \
	fv_UnixFrameEdit.cpp \
	fv_UnixFrameEdit.h \
	fv_UnixInlineImage.cpp \
	fv_UnixInlineImage.h \
	fv_UnixSelectionHandles.cpp \
	fv_UnixSelectionHandles.h \
	fv_UnixVisualDrag.cpp \
	fv_UnixVisualDrag.h

