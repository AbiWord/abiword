
SUBDIRS = t

noinst_LTLIBRARIES = libgtk.la

# TODO: fix the wp include, needed because of ap_Features.h
AM_CPPFLAGS = \
	$(IMPEXP_CPPFLAGS) \
	-I$(top_srcdir)/src/wp/ap/gtk \
	-I$(top_srcdir)/src/wp/ap/xp \
	-DDATADIR="\"$(datadir)\""

hildon_src = hildon/xap_UnixHildonApp.cpp \
	hildon/xap_UnixHildonApp.h \
	hildon/xap_UnixHildonDlg_FontChooser.cpp    \
	hildon/xap_UnixHildonDlg_FontChooser.h	\
	$(NULL)
hildon_ui = hildon/xap_UnixHildonDlg_Image.xml \
	$(NULL)
altgtk_src = xap_UnixDlg_FontChooser.cpp \
	xap_UnixDlg_FontChooser.h \
	$(NULL)
altgtk_ui = xap_UnixDlg_Image.xml \
	$(NULL)
# list all conditional source files here so they end up in the dist tarball
conditional_src = $(hildon_src) $(altgtk_src)

conditional_ui = $(hildon_ui) $(altgtk_ui)

libgtk_la_SOURCES = \
	xap_GtkObjectHolder.h \
	xap_GtkSignalBlocker.h \
	xap_GtkComboBoxHelpers.h \
	xap_GtkComboBoxHelpers.cpp \
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
	xap_UnixDlg_Print.cpp \
	xap_UnixDlg_Print.h \
	xap_UnixDlg_PrintPreview.cpp \
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


uidir=$(ABIWORD_UIDIR)
ui_DATA = \
	xap_UnixDlg_Encoding.xml \
	xap_UnixDlg_Language.xml \
	xap_UnixDlg_Password.xml \
	xap_UnixDlg_PluginManager.xml \
	xap_UnixDlg_WindowMore.xml \
	xap_UnixDlg_Zoom.xml \
	xap_UnixDlg_History.xml \
	xap_UnixDlg_DocComparison.xml \
	xap_UnixDlg_ListDocuments.xml \
	$(NULL)

if EMBEDDED_HILDON
libgtk_la_SOURCES += $(hildon_src)
ui_DATA += $(hildon_ui)
else
libgtk_la_SOURCES += $(altgtk_src)
ui_DATA += $(altgtk_ui)	
endif


EXTRA_DIST = \
	$(conditional_src) \
	$(conditional_ui) \
	$(ui_DATA)

