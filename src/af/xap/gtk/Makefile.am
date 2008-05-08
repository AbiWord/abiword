
SUBDIRS = t

noinst_LTLIBRARIES = libgtk.la

# TODO: fix the wp include, needed because of ap_Features.h
AM_CPPFLAGS = \
	$(IMPEXP_CPPFLAGS) \
	-I$(top_srcdir)/src/wp/ap/gtk \
	-I$(top_srcdir)/src/wp/ap/xp \
	-DDATADIR="\"$(datadir)\""

# list all conditional source files here so they end up in the dist tarball
conditional_src = \
	xap_UnixDlg_PrintPreview.cpp \
	xap_UnixDlg_Print.cpp			

if ENABLE_PRINT
print_src = \
	xap_UnixDlg_PrintPreview.cpp \
	xap_UnixDlg_Print.cpp			
endif

libgtk_la_SOURCES = \
	$(print_src) \
	xap_EmbeddedFeatures.h \
	xap_UnixApp.cpp \
	xap_UnixApp.h \
	xap_UnixAppImpl.cpp \
	xap_UnixAppImpl.h \
	xap_UnixClipboard.cpp \
	xap_UnixClipboard.h \
	xap_UnixDialogFactory.h \
	xap_UnixDialogHelper.cpp \
	xap_UnixDialogHelper.h \
	xap_UnixDlg_About.cpp \
	xap_UnixDlg_About.h \
	xap_UnixDlg_ClipArt.cpp \
	xap_UnixDlg_ClipArt.h \
	xap_UnixDlg_DocComparison.cpp \
	xap_UnixDlg_DocComparison.h \
	xap_UnixDlg_Encoding.cpp \
	xap_UnixDlg_Encoding.h \
	xap_UnixDlg_FileOpenSaveAs.cpp \
	xap_UnixDlg_FileOpenSaveAs.h \
	xap_UnixDlg_FontChooser.cpp \
	xap_UnixDlg_FontChooser.h \
	xap_UnixDlg_History.cpp \
	xap_UnixDlg_History.h \
	xap_UnixDlg_HTMLOptions.cpp \
	xap_UnixDlg_HTMLOptions.h \
	xap_UnixDlg_Image.cpp \
	xap_UnixDlg_Image.h \
	xap_UnixDlg_Insert_Symbol.cpp \
	xap_UnixDlg_Insert_Symbol.h \
	xap_UnixDlg_Language.cpp \
	xap_UnixDlg_Language.h \
	xap_UnixDlg_ListDocuments.cpp \
	xap_UnixDlg_ListDocuments.h \
	xap_UnixDlg_MessageBox.cpp \
	xap_UnixDlg_MessageBox.h \
	xap_UnixDlg_Password.cpp \
	xap_UnixDlg_Password.h \
	xap_UnixDlg_PluginManager.cpp \
	xap_UnixDlg_PluginManager.h \
	xap_UnixDlg_Print.h \
	xap_UnixDlg_PrintPreview.h \
	xap_UnixDlg_WindowMore.cpp \
	xap_UnixDlg_WindowMore.h \
	xap_UnixDlg_Zoom.cpp \
	xap_UnixDlg_Zoom.h \
	xap_UnixEncodingManager.cpp \
	xap_UnixEncodingManager.h \
	xap_UnixFeatures.h \
	xap_UnixFontPreview.cpp \
	xap_UnixFontPreview.h \
	xap_UnixFrameImpl.cpp \
	xap_UnixFrameImpl.h \
	xap_UnixModule.cpp \
	xap_UnixModule.h \
	xap_UnixTableWidget.cpp \
	xap_UnixTableWidget.h \
	xap_Unix_TB_CFactory.h \
	xap_UnixWidget.cpp \
	xap_UnixWidget.h

gladedir=$(ABIWORD_GLADEDIR)
glade_DATA = \
	xap_UnixDlg_Encoding.glade \
	xap_UnixDlg_Language.glade \
	xap_UnixDlg_Password.glade \
	xap_UnixDlg_PluginManager.glade \
	xap_UnixDlg_WindowMore.glade \
	xap_UnixDlg_Image.glade	\
	xap_UnixDlg_Zoom.glade \
	xap_UnixDlg_History.glade \
	xap_UnixDlg_DocComparison.glade \
	xap_UnixDlg_ListDocuments.glade

EXTRA_DIST = \
	$(conditional_src) \
	$(glade_DATA)

