
DIST_SUBDIRS = t

noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(AF_CPPFLAGS)

# list all conditional source files here so they end up in the dist tarball
conditional_src = \
	barbarisms.cpp \
	barbarisms.h \
	enchant_checker.cpp \
	enchant_checker.h \
	ispell_def.h \
	spell_manager.cpp \
	spell_manager.h
	xap_StatusBar.cpp 


if ENABLE_SPELL
spell_src = \
	barbarisms.cpp \
	barbarisms.h \
	enchant_checker.cpp \
	enchant_checker.h \
	ispell_def.h \
	spell_manager.cpp \
	spell_manager.h
endif

if ENABLE_STATUSBAR
statusbar_src = xap_StatusBar.cpp
endif

libxp_la_SOURCES = \
	$(spell_src) \
	$(statusbar_src) \
	xad_Document.cpp \
	xad_Document.h \
	xap_App.cpp \
	xap_App.h \
	xap_AppImpl.cpp \
	xap_AppImpl.h \
	xap_Args.cpp \
	xap_Args.h \
	xap_Clipboard.cpp \
	xap_Clipboard.h \
	xap_CustomWidget.cpp \
	xap_CustomWidget.h \
	xap_Dialog.cpp \
	xap_DialogFactory.cpp \
	xap_DialogFactory.h \
	xap_Dialog.h \
	xap_Dialog_Id.h \
	xap_Dictionary.cpp \
	xap_Dictionary.h \
	xap_Dlg_About.cpp \
	xap_Dlg_About.h \
	xap_Dlg_ClipArt.cpp \
	xap_Dlg_ClipArt.h \
	xap_Dlg_DocComparison.cpp \
	xap_Dlg_DocComparison.h \
	xap_Dlg_Encoding.cpp \
	xap_Dlg_Encoding.h \
	xap_Dlg_FileOpenSaveAs.cpp \
	xap_Dlg_FileOpenSaveAs.h \
	xap_Dlg_FontChooser.cpp \
	xap_Dlg_FontChooser.h \
	xap_Dlg_History.cpp \
	xap_Dlg_History.h \
	xap_Dlg_HTMLOptions.cpp \
	xap_Dlg_HTMLOptions.h \
	xap_Dlg_Image.cpp \
	xap_Dlg_Image.h \
	xap_Dlg_Insert_Symbol.cpp \
	xap_Dlg_Insert_Symbol.h \
	xap_Dlg_Language.cpp \
	xap_Dlg_Language.h \
	xap_Dlg_ListDocuments.cpp \
	xap_Dlg_ListDocuments.h \
	xap_Dlg_MessageBox.cpp \
	xap_Dlg_MessageBox.h \
	xap_Dlg_Password.cpp \
	xap_Dlg_Password.h \
	xap_Dlg_PluginManager.cpp \
	xap_Dlg_PluginManager.h \
	xap_Dlg_Print.cpp \
	xap_Dlg_Print.h \
	xap_Dlg_PrintPreview.cpp \
	xap_Dlg_PrintPreview.h \
	xap_Dlg_WindowMore.cpp \
	xap_Dlg_WindowMore.h \
	xap_Dlg_Zoom.cpp \
	xap_Dlg_Zoom.h \
	xap_Draw_Symbol.cpp \
	xap_Draw_Symbol.h \
	xap_EditMethods.h \
	xap_EncodingManager.cpp \
	xap_EncodingManager.h \
	xap_FakeClipboard.cpp \
	xap_FakeClipboard.h \
	xap_Features.h \
	xap_FontPreview.cpp \
	xap_FontPreview.h \
	xap_Frame.cpp \
	xap_Frame.h \
	xap_FrameImpl.cpp \
	xap_FrameImpl.h \
	xap_InputModes.cpp \
	xap_InputModes.h \
	xap_LoadBindings.cpp \
	xap_LoadBindings.h \
	xap_Log.cpp \
	xap_Log.h \
	xap_Menu_ActionSet.h \
	xap_Menu_LabelSet.h \
	xap_Menu_Layouts.h \
	xap_Module.cpp \
	xap_ModuleFactory.h \
	xap_Module.h \
	xap_ModuleManager.cpp \
	xap_ModuleManager.h \
	xap_Prefs.cpp \
	xap_Prefs.h \
	xap_Prefs_SchemeIds.h \
	xap_Preview.cpp \
	xap_Preview.h \
	xap_Preview_Zoom.cpp \
	xap_Preview_Zoom.h \
	xap_Resource.cpp \
	xap_Resource.h \
	xap_ResourceManager.cpp \
	xap_ResourceManager.h \
	xap_Scrollbar_ViewListener.cpp \
	xap_Scrollbar_ViewListener.h \
	xap_StatusBar.h \
	xap_String_Id.h \
	xap_Strings.cpp \
	xap_Strings.h \
	xap_Toolbar_ActionSet.h \
	xap_Toolbar_ControlFactory.cpp \
	xap_Toolbar_ControlFactory.h \
	xap_Toolbar_Icons.h \
	xap_Toolbar_LabelSet.h \
	xap_Toolbar_Layouts.h \
	xap_Types.h \
	xap_ViewListener.cpp \
	xap_ViewListener.h \
	xap_Widget.h \
	xav_Listener.h \
	xav_View.cpp \
	xav_View.h

EXTRA_DIST = \
	$(conditional_src)

