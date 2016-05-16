
#DIST_SUBDIRS = t

noinst_LTLIBRARIES = libqt.la

# TODO: fix the wp include, needed because of ap_Features.h
AM_CPPFLAGS = \
	$(IMPEXP_CPPFLAGS) \
	-I$(top_srcdir)/src/wp/ap/qt \
	-I$(top_srcdir)/src/wp/ap/xp \
	-DDATADIR="\"$(datadir)\""

libqt_la_SOURCES = \
	xap_QtApp.h \
	xap_QtApp.cpp \
	xap_QtAppImpl.cpp \
	xap_QtAppImpl.h \
	xap_QtDialogFactory.h \
	xap_QtEncodingManager.cpp \
	xap_QtEncodingManager.h \
	xap_QtFeatures.h \
	xap_QtFrameImpl.h \
	xap_QtFrameImpl.cpp \
	xap_QtModule.h \
	xap_QtModule.cpp \
	xap_Qt_TB_CFactory.h \
	xap_QtDlg_FileOpenSaveAs.cpp \
	xap_QtDlg_FileOpenSaveAs.h \
	$(NULL)

#	xap_GtkObjectHolder.h \
#	xap_GtkSignalBlocker.h \
#	xap_GtkComboBoxHelpers.h \
#	xap_GtkComboBoxHelpers.cpp \
#	xap_EmbeddedFeatures.h \
#	xap_UnixClipboard.cpp \
#	xap_UnixClipboard.h \
#	xap_UnixCustomWidget.cpp \
#	xap_UnixCustomWidget.h \
#	xap_UnixDialogHelper.cpp \
#	xap_UnixDialogHelper.h \
#	xap_UnixDlg_About.cpp \
#	xap_UnixDlg_About.h \
#	xap_UnixDlg_ClipArt.cpp \
#	xap_UnixDlg_ClipArt.h \
#	xap_UnixDlg_DocComparison.cpp \
#	xap_UnixDlg_DocComparison.h \
#	xap_UnixDlg_Encoding.cpp \
#	xap_UnixDlg_Encoding.h \
#	xap_UnixDlg_FileOpenSaveAs.cpp \
#	xap_UnixDlg_FileOpenSaveAs.h \
#	xap_UnixDlg_History.cpp \
#	xap_UnixDlg_History.h \
#	xap_UnixDlg_HTMLOptions.cpp \
#	xap_UnixDlg_HTMLOptions.h \
#	xap_UnixDlg_Image.cpp \
#	xap_UnixDlg_Image.h \
#	xap_UnixDlg_Insert_Symbol.cpp \
#	xap_UnixDlg_Insert_Symbol.h \
#	xap_UnixDlg_Language.cpp \
#	xap_UnixDlg_Language.h \
#	xap_UnixDlg_ListDocuments.cpp \
#	xap_UnixDlg_ListDocuments.h \
#	xap_UnixDlg_MessageBox.cpp \
#	xap_UnixDlg_MessageBox.h \
#	xap_UnixDlg_Password.cpp \
#	xap_UnixDlg_Password.h \
#	xap_UnixDlg_PluginManager.cpp \
#	xap_UnixDlg_PluginManager.h \
#	xap_UnixDlg_Print.cpp \
#	xap_UnixDlg_Print.h \
#	xap_UnixDlg_PrintPreview.cpp \
#	xap_UnixDlg_PrintPreview.h \
#	xap_UnixDlg_WindowMore.cpp \
#	xap_UnixDlg_WindowMore.h \
#	xap_UnixDlg_Zoom.cpp \
#	xap_UnixDlg_Zoom.h \
#	xap_UnixFontPreview.cpp \
#	xap_UnixFontPreview.h \
#	xap_UnixTableWidget.cpp \
#	xap_UnixTableWidget.h \
#	xap_UnixWidget.cpp \
#	xap_UnixWidget.h


