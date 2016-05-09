
SUBDIRS = ToolbarIcons

# TODO set this in configure.in
CDUMPAPP = $(top_srcdir)/tools/cdump/xp/cdump.pl

noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(WP_CPPFLAGS) \
	-I$(srcdir)/ToolbarIcons

# list all conditional source files here so they end up in the dist tarball
conditional_src = \
	ap_LB_Emacs.cpp \
	ap_LB_EmacsCtrlX.cpp \
	ap_LB_EmacsCtrlX.h \
	ap_LB_Emacs.h \
	ap_Dialog_Spell.cpp \
	ap_StatusBar.cpp \
	ap_LB_viEdit_c.cpp \
	ap_LB_viEdit_c.h \
	ap_LB_viEdit_colon.cpp \
	ap_LB_viEdit_colon.h \
	ap_LB_viEdit.cpp \
	ap_LB_viEdit_d.cpp \
	ap_LB_viEdit_d.h \
	ap_LB_viEdit.h \
	ap_LB_viEdit_r.cpp \
	ap_LB_viEdit_r.h \
	ap_LB_viEdit_y.cpp \
	ap_LB_viEdit_y.h \
	ap_LB_viInput.cpp \
	ap_LB_viInput.h

if ENABLE_SPELL
spell_src = ap_Dialog_Spell.cpp
endif

if ENABLE_STATUSBAR
statusbar_src = ap_StatusBar.cpp
endif

if ENABLE_EMACS_KEYBINDING
emacs_keybinding_src = \
	ap_LB_Emacs.cpp \
	ap_LB_EmacsCtrlX.cpp \
	ap_LB_EmacsCtrlX.h \
	ap_LB_Emacs.h
endif

if ENABLE_VI_KEYBINDING
vi_keybinding_src = \
	ap_LB_viEdit_c.cpp \
	ap_LB_viEdit_c.h \
	ap_LB_viEdit_colon.cpp \
	ap_LB_viEdit_colon.h \
	ap_LB_viEdit.cpp \
	ap_LB_viEdit_d.cpp \
	ap_LB_viEdit_d.h \
	ap_LB_viEdit.h \
	ap_LB_viEdit_r.cpp \
	ap_LB_viEdit_r.h \
	ap_LB_viEdit_y.cpp \
	ap_LB_viEdit_y.h \
	ap_LB_viInput.cpp \
	ap_LB_viInput.h
endif

libxp_la_SOURCES = \
	$(spell_src) \
	$(statusbar_src) \
	$(emacs_keybinding_src) \
	$(vi_keybinding_src) \
	ap_App.cpp \
	ap_App.h \
	ap_Args.cpp \
	ap_Args.h \
	ap_Clipboard.h \
	ap_Convert.cpp \
	ap_Convert.h \
	ap_Dialog_Modeless.h \
	ap_Dialog_Modeless.cpp \
	ap_Dialog_Modal.h \
	ap_Dialog_Modal.cpp \
	ap_Dialog_Annotation.h \
	ap_Dialog_Background.cpp \
	ap_Dialog_Background.h \
	ap_Dialog_Border_Shading.cpp \
	ap_Dialog_Border_Shading.h \
	ap_Dialog_Break.cpp \
	ap_Dialog_Break.h \
	ap_Dialog_Columns.cpp \
	ap_Dialog_Columns.h \
	ap_Dialog_Field.cpp \
	ap_Dialog_Field.h \
	ap_Dialog_FormatFootnotes.cpp \
	ap_Dialog_FormatFootnotes.h \
	ap_Dialog_FormatFrame.cpp \
	ap_Dialog_FormatFrame.h \
	ap_Dialog_FormatTable.cpp \
	ap_Dialog_FormatTable.h \
	ap_Dialog_FormatTOC.cpp \
	ap_Dialog_FormatTOC.h \
	ap_Dialog_Goto.cpp \
	ap_Dialog_Goto.h \
	ap_Dialog_HdrFtr.cpp \
	ap_Dialog_HdrFtr.h \
	ap_Dialog_Id.h \
	ap_Dialog_GetStringCommon.cpp \
	ap_Dialog_GetStringCommon.h \
	ap_Dialog_InsertBookmark.cpp \
	ap_Dialog_InsertBookmark.h \
	ap_Dialog_InsertXMLID.cpp \
	ap_Dialog_InsertXMLID.h \
	ap_Dialog_Insert_DateTime.cpp \
	ap_Dialog_Insert_DateTime.h \
	ap_Dialog_InsertHyperlink.cpp \
	ap_Dialog_InsertHyperlink.h \
	ap_Dialog_InsertTable.cpp \
	ap_Dialog_InsertTable.h \
	ap_Dialog_Latex.cpp \
	ap_Dialog_Latex.h \
	ap_Dialog_ListRevisions.cpp \
	ap_Dialog_ListRevisions.h \
	ap_Dialog_Lists.cpp \
	ap_Dialog_Lists.h \
	ap_Dialog_MailMerge.cpp \
	ap_Dialog_MailMerge.h \
	ap_Dialog_MarkRevisions.cpp \
	ap_Dialog_MarkRevisions.h \
	ap_Dialog_MergeCells.cpp \
	ap_Dialog_MergeCells.h \
	ap_Dialog_MetaData.h \
	ap_Dialog_New.cpp \
	ap_Dialog_New.h \
	ap_Dialog_Options.cpp \
	ap_Dialog_Options.h \
	ap_Dialog_PageNumbers.cpp \
	ap_Dialog_PageNumbers.h \
	ap_Dialog_PageSetup.cpp \
	ap_Dialog_PageSetup.h \
	ap_Dialog_Paragraph.cpp \
	ap_Dialog_Paragraph.h \
	ap_Dialog_Replace.cpp \
	ap_Dialog_Replace.h \
	ap_Dialog_Spell.h \
	ap_Dialog_SplitCells.cpp \
	ap_Dialog_SplitCells.h \
	ap_Dialog_Stub.h \
	ap_Dialog_Styles.cpp \
	ap_Dialog_Styles.h \
	ap_Dialog_Stylist.cpp \
	ap_Dialog_Stylist.h \
	ap_Dialog_Tab.cpp \
	ap_Dialog_Tab.h \
	ap_Dialog_ToggleCase.cpp \
	ap_Dialog_ToggleCase.h \
	ap_Dialog_WordCount.cpp \
	ap_Dialog_WordCount.h \
	ap_Dialog_RDFQuery.cpp \
	ap_Dialog_RDFQuery.h \
	ap_Dialog_RDFEditor.cpp \
	ap_Dialog_RDFEditor.h \
	ap_EditMethods.cpp \
	ap_EditMethods.h \
	ap_EmbeddedFeatures.h \
	ap_Features.h \
	ap_Frame.cpp \
	ap_FrameData.cpp \
	ap_FrameData.h \
	ap_Frame.h \
	ap_FrameListener.h \
	ap_LB_DeadAbovedot.cpp \
	ap_LB_DeadAbovedot.h \
	ap_LB_DeadAcute.cpp \
	ap_LB_DeadAcute.h \
	ap_LB_DeadBreve.cpp \
	ap_LB_DeadBreve.h \
	ap_LB_DeadCaron.cpp \
	ap_LB_DeadCaron.h \
	ap_LB_DeadCedilla.cpp \
	ap_LB_DeadCedilla.h \
	ap_LB_DeadCircumflex.cpp \
	ap_LB_DeadCircumflex.h \
	ap_LB_DeadDiaeresis.cpp \
	ap_LB_DeadDiaeresis.h \
	ap_LB_DeadDoubleacute.cpp \
	ap_LB_DeadDoubleacute.h \
	ap_LB_DeadGrave.cpp \
	ap_LB_DeadGrave.h \
	ap_LB_DeadMacron.cpp \
	ap_LB_DeadMacron.h \
	ap_LB_DeadOgonek.cpp \
	ap_LB_DeadOgonek.h \
	ap_LB_DeadTilde.cpp \
	ap_LB_DeadTilde.h \
	ap_LB_Default.cpp \
	ap_LB_Default.h \
	ap_LeftRuler.cpp \
	ap_LeftRuler.h \
	ap_LoadBindings.cpp \
	ap_LoadBindings.h \
	ap_Menu_ActionSet.cpp \
	ap_Menu_Functions.cpp \
	ap_Menu_Functions.h \
	ap_Menu_Id.h \
	ap_Menu_Id_List.h \
	ap_Menu_Layouts_All.h \
	ap_Menu_Layouts.cpp \
	ap_Menu_Layouts_Embedded.h \
	ap_Menu_Layouts_MainMenu.h \
	ap_ML_ContextEmbed.h \
	ap_ML_ContextEquation.h \
	ap_ML_ContextFrame.h \
	ap_ML_ContextHyperlink.h \
	ap_ML_ContextImage.h \
	ap_ML_ContextPosObject.h \
	ap_ML_ContextRevision.h \
	ap_ML_ContextSquiggle.h \
	ap_ML_ContextText.h \
	ap_ML_ContextTOC.h \
	ap_Prefs.cpp \
	ap_Prefs.h \
	ap_Prefs_SchemeIds.h \
	ap_Preview_Abi.cpp \
	ap_Preview_Abi.h \
	ap_Preview_Annotation.cpp \
	ap_Preview_Annotation.h \
	ap_Preview_Paragraph.cpp \
	ap_Preview_Paragraph.h \
	ap_Ruler.cpp \
	ap_Ruler.h \
	ap_StatusBar.h \
	ap_String_Id.h \
	ap_Strings.cpp \
	ap_Strings.h \
	ap_TB_Layouts_Embedded.h \
	ap_TB_Layouts_ExtraOps.h \
	ap_TB_Layouts_FileEditOps.h \
	ap_TB_Layouts_FormatOps.h \
	ap_TB_Layouts_SimpleOps.h \
	ap_TB_Layouts_TableOps.h \
	ap_Toolbar_ActionSet.cpp \
	ap_Toolbar_Functions.cpp \
	ap_Toolbar_Functions.h \
	ap_Toolbar_Iconmap.h \
	ap_Toolbar_Icons_All.h \
	ap_Toolbar_Icons.cpp \
	ap_Toolbar_Id.h \
	ap_Toolbar_Id_List.h \
	ap_Toolbar_LabelSet.cpp \
	ap_Toolbar_Layouts_All.h \
	ap_Toolbar_Layouts.cpp \
	ap_TopRuler.cpp \
	ap_TopRuler.h \
	ap_wp_sidebar.cpp \
	ap_RDFContact.h \
	ap_RDFContact.cpp \
	ap_RDFEvent.h \
	ap_RDFEvent.cpp \
	ap_RDFLocation.h \
	ap_RDFLocation.cpp

EXTRA_DIST = \
	$(conditional_src) \
	margin.xpm \
	orient-horizontal.xpm \
	orient-vertical.xpm \
	sidebar.png \
	ap_wp_sidebar_static.cpp

BUILT_SOURCES = ap_wp_sidebar.cpp \
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(NULL)

ap_wp_sidebar.cpp: $(srcdir)/sidebar.png
	$(CDUMPAPP) $(srcdir)/sidebar.png g_pngSidebar > ap_wp_sidebar.cpp

