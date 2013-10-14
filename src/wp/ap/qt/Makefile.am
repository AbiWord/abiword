
noinst_LTLIBRARIES = libqt.la

AM_CPPFLAGS = 						\
	$(WP_CPPFLAGS) 						\
	-I$(top_srcdir)/src/wp/ap/xp/ToolbarIcons 		\
	-DPREFIX=\""$(prefix)"\" 				\
	-DSYSCONFDIR=\""$(sysconfdir)"\" 			\
	-DABIWORD_PLUGINSDIR="\"$(ABIWORD_PLUGINSDIR)"\"	\
	-DDATADIR="\"$(datadir)\""

#        $(CHAMPLAIN_CFLAGS)


libqt_la_SOURCES = \
	ap_QtApp.cpp \
	ap_QtApp.h \
	ap_QtAppImpl.h \
	ap_QtAppImpl.cpp \
	ap_QtClipboard.h \
	ap_QtDialog_All.h \
	ap_QtDialogFactory.cpp \
	ap_QtFrame.h \
	ap_QtFrame.cpp \
	ap_QtFrameImpl.h \
	ap_QtFrameImpl.cpp \
	ap_QtPrefs.h \
	ap_QtPrefs.cpp \
	ap_Qt_TB_CFactory.cpp \
	ap_QtToolbar_Control_All.h \
	ap_QtToolbar_ZoomCombo.cpp \
	ap_QtToolbar_ZoomCombo.h \
	ap_QtToolbar_SizeCombo.cpp \
	ap_QtToolbar_SizeCombo.h \
	ap_QtToolbar_FontCombo.cpp \
	ap_QtToolbar_FontCombo.h \
	ap_QtToolbar_StyleCombo.cpp \
	ap_QtToolbar_StyleCombo.h \
	ap_QtStatusBar.cpp \
	ap_QtStatusBar.h \
	ap_QtStockIcons.cpp \
	ap_QtStockIcons.h \
	$(NULL)



#	GTKCommon.h \
#	GTKCommon.cpp \
#	$(abiwidget_src) \
#	ap_DocView.cpp \
#	ap_DocView.h \
#	ap_UnixClipboard.cpp \
#	ap_UnixDialog_Annotation.cpp \
#	ap_UnixDialog_Annotation.h \
#	ap_UnixDialog_Background.cpp \
#	ap_UnixDialog_Background.h \
#	ap_UnixDialog_Border_Shading.cpp \
#	ap_UnixDialog_Border_Shading.h \
#	ap_UnixDialog_Break.cpp \
#	ap_UnixDialog_Break.h \
#	ap_UnixDialog_Columns.cpp \
#	ap_UnixDialog_Columns.h \
#	ap_UnixDialog_Field.cpp \
#	ap_UnixDialog_Field.h \
#	ap_UnixDialog_FormatFootnotes.cpp \
#	ap_UnixDialog_FormatFootnotes.h \
#	ap_UnixDialog_FormatFrame.cpp \
#	ap_UnixDialog_FormatFrame.h \
#	ap_UnixDialog_FormatTable.cpp \
#	ap_UnixDialog_FormatTable.h \
#	ap_UnixDialog_FormatTOC.cpp \
#	ap_UnixDialog_FormatTOC.h \
#	ap_UnixDialog_Goto.cpp \
#	ap_UnixDialog_Goto.h \
#	ap_UnixDialog_HdrFtr.cpp \
#	ap_UnixDialog_HdrFtr.h \
#	ap_UnixDialog_InsertBookmark.cpp \
#	ap_UnixDialog_InsertBookmark.h \
#	ap_UnixDialog_InsertXMLID.cpp \
#	ap_UnixDialog_InsertXMLID.h \
#	ap_UnixDialog_Insert_DateTime.cpp \
#	ap_UnixDialog_Insert_DateTime.h \
#	ap_UnixDialog_InsertHyperlink.cpp \
#	ap_UnixDialog_InsertHyperlink.h \
#	ap_UnixDialog_InsertTable.cpp \
#	ap_UnixDialog_InsertTable.h \
#	ap_UnixDialog_Latex.cpp \
#	ap_UnixDialog_Latex.h \
#	ap_UnixDialog_ListRevisions.cpp \
#	ap_UnixDialog_ListRevisions.h \
#	ap_UnixDialog_Lists.cpp \
#	ap_UnixDialog_Lists.h \
#	ap_UnixDialog_MailMerge.cpp \
#	ap_UnixDialog_MailMerge.h \
#	ap_UnixDialog_MarkRevisions.cpp \
#	ap_UnixDialog_MarkRevisions.h \
#	ap_UnixDialog_MergeCells.cpp \
#	ap_UnixDialog_MergeCells.h \
#	ap_UnixDialog_MetaData.cpp \
#	ap_UnixDialog_MetaData.h \
#	ap_UnixDialog_New.cpp \
#	ap_UnixDialog_New.h \
#	ap_UnixDialog_Options.cpp \
#	ap_UnixDialog_Options.h \
#	ap_UnixDialog_PageNumbers.cpp \
#	ap_UnixDialog_PageNumbers.h \
#	ap_UnixDialog_PageSetup.cpp \
#	ap_UnixDialog_PageSetup.h \
#	ap_UnixDialog_Paragraph.cpp \
#	ap_UnixDialog_Paragraph.h \
#	ap_UnixDialog_Replace.cpp \
#	ap_UnixDialog_Replace.h \
#	ap_UnixDialog_Spell.h \
#	ap_UnixDialog_SplitCells.cpp \
#	ap_UnixDialog_SplitCells.h \
#	ap_UnixDialog_Styles.cpp \
#	ap_UnixDialog_Styles.h \
#	ap_UnixDialog_Stylist.cpp \
#	ap_UnixDialog_Stylist.h \
#	ap_UnixDialog_Tab.cpp \
#	ap_UnixDialog_Tab.h \
#	ap_UnixDialog_ToggleCase.cpp \
#	ap_UnixDialog_ToggleCase.h \
#	ap_UnixDialog_WordCount.cpp \
#	ap_UnixDialog_WordCount.h \
#	ap_UnixDialog_RDFQuery.cpp \
#	ap_UnixDialog_RDFQuery.h \
#	ap_UnixDialog_RDFEditor.cpp \
#	ap_UnixDialog_RDFEditor.h \
#	ap_UnixLeftRuler.cpp \
#	ap_UnixLeftRuler.h \
#	ap_UnixPreview_Annotation.cpp \
#	ap_UnixPreview_Annotation.h \
#	ap_UnixStockIcons.cpp \
#	ap_UnixStockIcons.h \
#	ap_UnixTopRuler.cpp \
#	ap_UnixTopRuler.h \
#	ap_UnixViewListener.cpp \
#	ap_UnixViewListener.h \
#	ap_RDFContactGTK.h \
#	ap_RDFContactGTK.cpp \
#	ap_RDFEventGTK.h \
#	ap_RDFEventGTK.cpp \
#	ap_RDFLocationGTK.h \
#	ap_RDFLocationGTK.cpp \
#	ap_RDFSemanticItemFactoryGTK.h \
#	ap_RDFSemanticItemFactoryGTK.cpp \
#	ap_RDFSemanticItemGTKInjected.h \
#	ap_RDFSemanticItemGTKInjected.cpp



if ENABLE_SPELL
libqt_la_SOURCES += $(spell_src)
endif

if ENABLE_STATUSBAR
libqt_la_SOURCES += $(statusbar_src)
endif
