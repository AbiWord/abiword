
noinst_LTLIBRARIES = libwin.la

# TODO: fix the wp include, needed because of ap_Features.h
AM_CPPFLAGS = \
	$(WP_CPPFLAGS)

libwin_la_SOURCES = \
	winezmouse.h \
	xap_Win32App.cpp \
	xap_Win32App.h \
	xap_Win32AppImpl.cpp \
	xap_Win32AppImpl.h \
	xap_Win32Clipboard.cpp \
	xap_Win32Clipboard.h \
	xap_Win32ColourButton.h \
	xap_Win32DialogBase.cpp \
	xap_Win32DialogBase.h \
	xap_Win32DialogFactory.h \
	xap_Win32DialogHelper.cpp \
	xap_Win32DialogHelper.h \
	xap_Win32Dlg_About.cpp \
	xap_Win32Dlg_About.h \
	xap_Win32Dlg_ClipArt.cpp \
	xap_Win32Dlg_ClipArt.h \
	xap_Win32Dlg_DocComparison.cpp \
	xap_Win32Dlg_DocComparison.h \
	xap_Win32Dlg_Encoding.cpp \
	xap_Win32Dlg_Encoding.h \
	xap_Win32Dlg_FileOpenSaveAs.cpp \
	xap_Win32Dlg_FileOpenSaveAs.h \
	xap_Win32Dlg_FontChooser.cpp \
	xap_Win32Dlg_FontChooser.h \
	xap_Win32Dlg_History.cpp \
	xap_Win32Dlg_History.h \
	xap_Win32Dlg_HTMLOptions.cpp \
	xap_Win32Dlg_HTMLOptions.h \
	xap_Win32Dlg_Image.cpp \
	xap_Win32Dlg_Image.h \
	xap_Win32Dlg_Insert_Symbol.cpp \
	xap_Win32Dlg_Insert_Symbol.h \
	xap_Win32Dlg_Language.cpp \
	xap_Win32Dlg_Language.h \
	xap_Win32Dlg_ListDocuments.cpp \
	xap_Win32Dlg_ListDocuments.h \
	xap_Win32Dlg_MessageBox.cpp \
	xap_Win32Dlg_MessageBox.h \
	xap_Win32Dlg_Password.cpp \
	xap_Win32Dlg_Password.h \
	xap_Win32Dlg_PluginManager.cpp \
	xap_Win32Dlg_PluginManager.h \
	xap_Win32Dlg_Print.cpp \
	xap_Win32Dlg_Print.h \
	xap_Win32Dlg_PrintPreview.cpp \
	xap_Win32Dlg_PrintPreview.h \
	xap_Win32Dlg_WindowMore.cpp \
	xap_Win32Dlg_WindowMore.h \
	xap_Win32Dlg_Zoom.cpp \
	xap_Win32Dlg_Zoom.h \
	xap_Win32DragAndDrop.cpp \
	xap_Win32DragAndDrop.h \
	xap_Win32EncodingManager.cpp \
	xap_Win32EncodingManager.h \
	xap_Win32Features.h \
	xap_Win32FontPreview.cpp \
	xap_Win32FontPreview.h \
	xap_Win32FrameImpl.cpp \
	xap_Win32FrameImpl.h \
	xap_Win32LabelledSeparator.cpp \
	xap_Win32LabelledSeparator.h \
	xap_Win32Module.cpp \
	xap_Win32Module.h \
	xap_Win32PreviewWidget.cpp \
	xap_Win32PreviewWidget.h \
	xap_Win32PropertySheet.cpp \
	xap_Win32PropertySheet.h \
	xap_Win32Slurp.cpp \
	xap_Win32Slurp.h \
	xap_Win32_TB_CFactory.h \
	xap_Win32Toolbar_Icons.cpp \
	xap_Win32Toolbar_Icons.h

rc2files = \
	xap_Win32Res_Cursors.rc2 \
	xap_Win32Res_DlgDocComparison.rc2 \
	xap_Win32Res_DlgEncoding.rc2 \
	xap_Win32Res_DlgFont.rc2 \
	xap_Win32Res_DlgHistory.rc2 \
	xap_Win32Res_DlgHTMLOptions.rc2 \
	xap_Win32Res_DlgImage.rc2 \
	xap_Win32Res_DlgInsertPicture.rc2 \
	xap_Win32Res_DlgInsertSymbol.rc2 \
	xap_Win32Res_DlgLanguage.rc2 \
	xap_Win32Res_DlgListDocuments.rc2 \
	xap_Win32Res_DlgPassword.rc2 \
	xap_Win32Res_DlgPlugin.rc2 \
	xap_Win32Res_DlgWindowMore.rc2 \
	xap_Win32Res_DlgZoom.rc2 \
	xap_Win32Resources.rc2

curfiles = \
	downarrow.cur \
	exchange.cur \
	hand1.cur \
	rightcur.cur

EXTRA_DIST = \
	$(rc2files) \
	$(curfiles)

