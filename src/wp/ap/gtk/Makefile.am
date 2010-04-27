
noinst_LTLIBRARIES = libgtk.la

AM_CPPFLAGS = 						\
	$(WP_CPPFLAGS) 						\
	-I$(top_srcdir)/src/wp/ap/xp/ToolbarIcons 		\
	-DPREFIX=\""$(prefix)"\" 				\
	-DSYSCONFDIR=\""$(sysconfdir)"\" 			\
	-DABIWORD_PLUGINSDIR="\"$(ABIWORD_PLUGINSDIR)"\"	\
	-DDATADIR="\"$(datadir)\""


# source and ui files for Hildon
hildon_src = hildon/ap_UnixHildonFrameImpl.cpp \
	hildon/ap_UnixHildonFrameImpl.h
hildon_ui = hildon/ap_UnixHildonDialog_HdrFtr.xml \
	hildon/ap_UnixHildonDialog_Options.xml
# these are alternate file for a plain Gtk build.
altgtk_ui = ap_UnixDialog_Options.xml \
	ap_UnixDialog_HdrFtr.xml \
	ap_UnixDialog_Options_ColorSel.xml \
	$(NULL)

spell_src = ap_UnixDialog_Spell.cpp
spell_ui = ap_UnixDialog_Spell.xml

statusbar_src = ap_UnixStatusBar.cpp


# list all conditional source files here so they end up in the dist tarball
conditional_src = \
	$(hildon_src) \
	$(spell_src) \
	$(statusbar_src) \
	$(NULL)


conditional_ui = \
	$(hildon_ui) \
	$(altgtk_ui) \
	$(spell_ui) \
	$(NULL)


libgtk_la_SOURCES = \
	abiwidget.cpp \
	abiwidget.h \
	ap_DocView.cpp \
	ap_DocView.h \
	ap_UnixApp.cpp \
	ap_UnixApp.h \
	ap_UnixAppImpl.cpp \
	ap_UnixAppImpl.h \
	ap_UnixClipboard.cpp \
	ap_UnixClipboard.h \
	ap_UnixDialog_All.h \
	ap_UnixDialog_Annotation.cpp \
	ap_UnixDialog_Annotation.h \
	ap_UnixDialog_Background.cpp \
	ap_UnixDialog_Background.h \
	ap_UnixDialog_Break.cpp \
	ap_UnixDialog_Break.h \
	ap_UnixDialog_Columns.cpp \
	ap_UnixDialog_Columns.h \
	ap_UnixDialogFactory.cpp \
	ap_UnixDialog_Field.cpp \
	ap_UnixDialog_Field.h \
	ap_UnixDialog_FormatFootnotes.cpp \
	ap_UnixDialog_FormatFootnotes.h \
	ap_UnixDialog_FormatFrame.cpp \
	ap_UnixDialog_FormatFrame.h \
	ap_UnixDialog_FormatTable.cpp \
	ap_UnixDialog_FormatTable.h \
	ap_UnixDialog_FormatTOC.cpp \
	ap_UnixDialog_FormatTOC.h \
	ap_UnixDialog_Goto.cpp \
	ap_UnixDialog_Goto.h \
	ap_UnixDialog_HdrFtr.cpp \
	ap_UnixDialog_HdrFtr.h \
	ap_UnixDialog_InsertBookmark.cpp \
	ap_UnixDialog_InsertBookmark.h \
	ap_UnixDialog_Insert_DateTime.cpp \
	ap_UnixDialog_Insert_DateTime.h \
	ap_UnixDialog_InsertHyperlink.cpp \
	ap_UnixDialog_InsertHyperlink.h \
	ap_UnixDialog_InsertTable.cpp \
	ap_UnixDialog_InsertTable.h \
	ap_UnixDialog_Latex.cpp \
	ap_UnixDialog_Latex.h \
	ap_UnixDialog_ListRevisions.cpp \
	ap_UnixDialog_ListRevisions.h \
	ap_UnixDialog_Lists.cpp \
	ap_UnixDialog_Lists.h \
	ap_UnixDialog_MailMerge.cpp \
	ap_UnixDialog_MailMerge.h \
	ap_UnixDialog_MarkRevisions.cpp \
	ap_UnixDialog_MarkRevisions.h \
	ap_UnixDialog_MergeCells.cpp \
	ap_UnixDialog_MergeCells.h \
	ap_UnixDialog_MetaData.cpp \
	ap_UnixDialog_MetaData.h \
	ap_UnixDialog_New.cpp \
	ap_UnixDialog_New.h \
	ap_UnixDialog_Options.cpp \
	ap_UnixDialog_Options.h \
	ap_UnixDialog_PageNumbers.cpp \
	ap_UnixDialog_PageNumbers.h \
	ap_UnixDialog_PageSetup.cpp \
	ap_UnixDialog_PageSetup.h \
	ap_UnixDialog_Paragraph.cpp \
	ap_UnixDialog_Paragraph.h \
	ap_UnixDialog_Replace.cpp \
	ap_UnixDialog_Replace.h \
	ap_UnixDialog_Spell.h \
	ap_UnixDialog_SplitCells.cpp \
	ap_UnixDialog_SplitCells.h \
	ap_UnixDialog_Styles.cpp \
	ap_UnixDialog_Styles.h \
	ap_UnixDialog_Stylist.cpp \
	ap_UnixDialog_Stylist.h \
	ap_UnixDialog_Tab.cpp \
	ap_UnixDialog_Tab.h \
	ap_UnixDialog_ToggleCase.cpp \
	ap_UnixDialog_ToggleCase.h \
	ap_UnixDialog_WordCount.cpp \
	ap_UnixDialog_WordCount.h \
	ap_UnixFrame.cpp \
	ap_UnixFrame.h \
	ap_UnixFrameImpl.cpp \
	ap_UnixFrameImpl.h \
	ap_UnixLeftRuler.cpp \
	ap_UnixLeftRuler.h \
	ap_UnixPrefs.cpp \
	ap_UnixPrefs.h \
	ap_UnixPreview_Annotation.cpp \
	ap_UnixPreview_Annotation.h \
	ap_UnixStatusBar.h \
	ap_UnixStockIcons.cpp \
	ap_UnixStockIcons.h \
	ap_Unix_TB_CFactory.cpp \
	ap_UnixToolbar_Control_All.h \
	ap_UnixToolbar_FontCombo.cpp \
	ap_UnixToolbar_FontCombo.h \
	ap_UnixToolbar_SizeCombo.cpp \
	ap_UnixToolbar_SizeCombo.h \
	ap_UnixToolbar_StyleCombo.cpp \
	ap_UnixToolbar_StyleCombo.h \
	ap_UnixToolbar_ZoomCombo.cpp \
	ap_UnixToolbar_ZoomCombo.h \
	ap_UnixTopRuler.cpp \
	ap_UnixTopRuler.h \
	ap_UnixViewListener.cpp \
	ap_UnixViewListener.h



uidir=$(ABIWORD_UIDIR)
ui_DATA = \
	ap_UnixDialog_Annotation.xml \
	ap_UnixDialog_Break.xml \
	ap_UnixDialog_Field.xml \
	ap_UnixDialog_FormatTOC.xml \
	ap_UnixDialog_FormatFootnotes.xml \
	ap_UnixDialog_FormatTable.xml \
	ap_UnixDialog_FormatFrame.xml \
	ap_UnixDialog_Goto.xml \
	ap_UnixDialog_InsertTable.xml \
	ap_UnixDialog_Insert_DateTime.xml \
	ap_UnixDialog_MailMerge.xml \
	ap_UnixDialog_MetaData.xml \
	ap_UnixDialog_PageNumbers.xml	\
	ap_UnixDialog_PageSetup.xml \
	ap_UnixDialog_Replace.xml \
	ap_UnixDialog_New.xml \
	ap_UnixDialog_Styles.xml \
	ap_UnixDialog_Stylist.xml \
	ap_UnixDialog_Stylist_modal.xml \
	ap_UnixDialog_Tab.xml	\
	ap_UnixDialog_WordCount.xml \
	ap_UnixDialog_Latex.xml \
	$(NULL)


if ENABLE_SPELL
libgtk_la_SOURCES += $(spell_src)
ui_DATA += $(spell_ui)
endif

if ENABLE_STATUSBAR
libgtk_la_SOURCES += $(statusbar_src)
endif

if EMBEDDED_HILDON

libgtk_la_SOURCES += $(hildon_src)
ui_DATA += $(hildon_ui)

hildondesktopdir = @datadir@/applications/hildon
hildondesktop_DATA = hildon/abiword.desktop

hildondbusdir = @datadir@/dbus-1/services
hildondbus_DATA = hildon/com.abisource.abiword.service

hildonicondir = @datadir@/pixmaps
hildonicon_DATA = hildon/qgn_list_abiword.png

else 

ui_DATA += $(altgtk_ui)

endif



EXTRA_DIST = \
	$(conditional_src) \
	$(conditional_ui) \
	$(ui_DATA) \
	hildon/qgn_list_abiword.png \
	hildon/com.abisource.abiword.service.in \
	hildon/abiword.desktop.in \
	$(NULL)

