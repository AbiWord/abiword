
DIST_SUBDIRS = t

noinst_LTLIBRARIES = libxp.la

# TODO: look at refactoring IE_File* stuff so we'd get away using only AF_CPPFLAGS here
AM_CPPFLAGS = \
	$(WP_CPPFLAGS)

# list all conditional source files here so they end up in the dist tarball
conditional_src = \
	fl_Squiggles.cpp

if ENABLE_SPELL
spell_src = fl_Squiggles.cpp
endif

libxp_la_SOURCES = \
	$(spell_src) \
	fb_Alignment.cpp \
	fb_Alignment.h \
	fb_ColumnBreaker.cpp \
	fb_ColumnBreaker.h \
	fb_LineBreaker.cpp \
	fb_LineBreaker.h \
	fg_Graphic.cpp \
	fg_Graphic.h \
	fg_GraphicRaster.cpp \
	fg_GraphicRaster.h \
	fg_GraphicVector.cpp \
	fg_GraphicVector.h \
	fl_AutoLists.cpp \
	fl_AutoLists.h \
	fl_AutoNum.cpp \
	fl_AutoNum.h \
	fl_BlockLayout.cpp \
	fl_BlockLayout.h \
	fl_ContainerLayout.cpp \
	fl_ContainerLayout.h \
	fl_DocLayout.cpp \
	fl_DocLayout.h \
	fl_DocListener.cpp \
	fl_DocListener.h \
	fl_FootnoteLayout.cpp \
	fl_FootnoteLayout.h \
	fl_FrameLayout.cpp \
	fl_FrameLayout.h \
	fl_Layout.cpp \
	fl_Layout.h \
	fl_SectionLayout.cpp \
	fl_SectionLayout.h \
	fl_SelectionPreserver.cpp \
	fl_SelectionPreserver.h \
	fl_Squiggles.h \
	fl_TableLayout.cpp \
	fl_TableLayout.h \
	fl_TestRoutines.cpp \
	fl_TOCLayout.cpp \
	fl_TOCLayout.h \
	fp_AnnotationRun.cpp \
	fp_RDFAnchorRun.cpp \
	fp_Column.cpp \
	fp_Column.h \
	fp_ContainerObject.cpp \
	fp_ContainerObject.h \
	fp_DirectionMarkerRun.cpp \
	fp_DirectionMarkerRun.h \
	fp_EmbedRun.cpp \
	fp_EmbedRun.h \
	fp_FieldListLabelRun.cpp \
	fp_FieldListLabelRun.h \
	fp_Fields.h \
	fp_FieldTableSumRun.cpp \
	fp_FieldTOCNum.cpp \
	fp_FieldTOCNum.h \
	fp_FmtMarkRun.cpp \
	fp_FootnoteContainer.cpp \
	fp_FootnoteContainer.h \
	fp_FrameContainer.cpp \
	fp_FrameContainer.h \
	fp_Line.cpp \
	fp_Line.h \
	fp_MathRun.cpp \
	fp_MathRun.h \
	fp_Page.cpp \
	fp_Page.h \
	fp_PageSize.cpp \
	fp_PageSize.h \
	fp_Run.cpp \
	fp_Run.h \
	fp_TableContainer.cpp \
	fp_TableContainer.h \
	fp_TestRoutines.cpp \
	fp_TextRun.cpp \
	fp_TextRun.h \
	fp_TOCContainer.cpp \
	fp_TOCContainer.h \
	fp_types.h \
	fv_Base.cpp \
	fv_Base.h \
	fv_FrameEdit.cpp \
	fv_FrameEdit.h \
	fv_InlineImage.cpp \
	fv_InlineImage.h \
	fv_Selection.cpp \
	fv_Selection.h \
	fv_SelectionHandles.cpp \
	fv_SelectionHandles.h \
	fv_View_cmd.cpp \
	fv_View.cpp \
	fv_View.h \
	fv_View_protected.cpp \
	fv_View_TestRoutines.cpp \
	fv_VisualDragText.cpp \
	fv_VisualDragText.h \
	fv_ViewDoubleBuffering.cpp \
	fv_ViewDoubleBuffering.h

EXTRA_DIST = \
	$(conditional_src)

