/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef AP_COCOADIALOG_OPTIONS_H
#define AP_COCOADIALOG_OPTIONS_H

#import <Cocoa/Cocoa.h>

#include "ap_Dialog_Options.h"

class XAP_CocoaFrame;
class AP_CocoaDialog_Options;


@interface AP_CocoaDialog_OptionsController : NSWindowController
{
    IBOutlet NSButton *m_applyBtn;
    IBOutlet NSButton *m_cancelBtn;
    IBOutlet NSButton *m_defaultsBtn;
    IBOutlet NSButton *m_layoutAllowScreenColorsBtn;
    IBOutlet NSButton *m_layoutChooseScreenBtn;
    IBOutlet NSButton *m_layoutCursorBlinkBtn;
    IBOutlet NSButton *m_layoutCustomToolbarBtn;
    IBOutlet NSTextField *m_layoutDefaultPageSizeLabel;
    IBOutlet NSPopUpButton *m_layoutDefaultPageSizePopup;
    IBOutlet NSButton *m_layoutEnableSmartQuotesBtn;
    IBOutlet NSButton *m_layoutHiddenTextBtn;
    IBOutlet NSButton *m_layoutInvisbleMarksBtn;
    IBOutlet NSButton *m_layoutRulerBtn;
    IBOutlet NSBox *m_layoutShowHideBox;
    IBOutlet NSButton *m_layoutStatusBarBtn;
    IBOutlet NSTextField *m_layoutUnitsLabel;
    IBOutlet NSPopUpButton *m_layoutUnitsPopup;
    IBOutlet NSButton *m_layoutViewAllBtn;
    IBOutlet NSBox *m_layoutViewBox;
    IBOutlet NSButton *m_okBtn;
    IBOutlet NSBox *m_prefsAutoSaveBox;
    IBOutlet NSButton *m_prefsAutoSaveCurrentBtn;
    IBOutlet NSTextField *m_prefsAutoSaveMinField;
    IBOutlet NSButton *m_prefsAutoSavePrefsBtn;
    IBOutlet NSBox *m_prefsBidiBox;
    IBOutlet NSComboBox *m_prefsCurrentSetCombo;
    IBOutlet NSTextField *m_prefsCurrentSetLabel;
    IBOutlet NSButton *m_prefsDefaultToRTLBtn;
    IBOutlet NSButton *m_prefsLoadAllPluginsBtn;
    IBOutlet NSTextField *m_prefsMinutesLabel;
    IBOutlet NSBox *m_prefsMiscBox;
    IBOutlet NSButton *m_prefsOtherHebrwContextGlyphBtn;
    IBOutlet NSButton *m_prefsOtherUseContextGlyphsBtn;
    IBOutlet NSBox *m_prefsPrefsBox;
    IBOutlet NSButton *m_prefsShowSplashBtn;
    IBOutlet NSTextField *m_prefsWithExtField;
    IBOutlet NSTextField *m_prefsWithExtLabel;
    IBOutlet NSButton *m_spellAlwaysSuggBtn;
    IBOutlet NSButton *m_spellCheckAsTypeBtn;
    IBOutlet NSTextField *m_spellCustomDictLabel;
    IBOutlet NSButton *m_spellDictEditBtn;
    IBOutlet NSPopUpButton *m_spellDictionaryPopup;
    IBOutlet NSBox *m_spellGeneralBox;
    IBOutlet NSButton *m_spellHideErrBtn;
    IBOutlet NSBox *m_spellIgnoreBox;
    IBOutlet NSTextField *m_spellIgnoredWordLabel;
    IBOutlet NSButton *m_spellIgnoreEditBtn;
    IBOutlet NSButton *m_spellIgnoreFileAddrBtn;
    IBOutlet NSButton *m_spellIgnoreUppercaseBtn;
    IBOutlet NSButton *m_spellIgnoreWordsWithNumBtn;
    IBOutlet NSButton *m_spellResetDictBtn;
    IBOutlet NSButton *m_spellSuggFromMainDictBtn;
    IBOutlet NSTabView *m_tab;
    IBOutlet NSBox *m_tlbBtnStylBox;
    IBOutlet NSMatrix *m_tlbBtnStylGroup;
    IBOutlet NSMatrix *m_tlbShowHideGroup;
    IBOutlet NSBox *m_tlbTlbBox;
    IBOutlet NSTableView *m_tlbTlbList;
    IBOutlet NSButton *m_tlbViewTooltipBtn;
    IBOutlet NSBox *m_tlbVisibleBox;
	
	AP_CocoaDialog_Options * m_xap;
}
+ (AP_CocoaDialog_OptionsController *)loadFromNib;
- (void)windowDidLoad;
- (void)setXAPOwner:(AP_CocoaDialog_Options *)owner;
- (NSView *)_lookupWidget:(AP_Dialog_Options::tControl)controlId;
- (IBAction)applyAction:(id)sender;
- (IBAction)cancelAction:(id)sender;
- (IBAction)chooseDictAction:(id)sender;
- (IBAction)chooseScreenAction:(id)sender;
- (IBAction)defaultAction:(id)sender;
- (IBAction)editDictAction:(id)sender;
- (IBAction)increaseMinutesAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)resetDictAction:(id)sender;
@end

/*****************************************************************/
class AP_CocoaDialog_Options: public AP_Dialog_Options
{
public:
	AP_CocoaDialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_CocoaDialog_Options(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
	XAP_CocoaFrame * _getFrame()
		{ return static_cast<XAP_CocoaFrame*>(m_pFrame); };
	//void initializeTransperentToggle(void);
	void event_ChooseTransparentColor(void);
	void event_AllowTransparentColor(void);

    virtual void event_OK(void);
    virtual void event_Cancel(void);
    virtual void event_Apply(void);

 protected:
    void _saveCocoaOnlyPrefs();
    void _initCocoaOnlyPrefs();
    virtual void _storeWindowData(void);

	virtual void _controlEnable( tControl id, bool value );

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void _set##a(t)

 	SET_GATHER			(SpellCheckAsType,	bool );
 	SET_GATHER			(SpellHideErrors,	bool );
 	SET_GATHER			(SpellSuggest,		bool );
 	SET_GATHER			(SpellMainOnly,		bool );
 	SET_GATHER			(SpellUppercase,	bool );
 	SET_GATHER			(SpellNumbers,		bool );
 	SET_GATHER			(SpellInternet,		bool );

 	SET_GATHER			(ShowSplash,	bool);

	SET_GATHER			(SmartQuotesEnable,	bool );
	SET_GATHER			(DefaultPageSize,  fp_PageSize::Predefined);

 	SET_GATHER			(PrefsAutoSave,		bool );

 	SET_GATHER			(ViewShowRuler,		bool );

 	SET_GATHER			(ViewShowStandardBar,	bool );
 	SET_GATHER			(ViewShowFormatBar,	bool );
 	SET_GATHER			(ViewShowExtraBar,	bool );

 	SET_GATHER			(ViewShowStatusBar,	bool );

	SET_GATHER			(ViewRulerUnits,	UT_Dimension);
	SET_GATHER			(ViewCursorBlink,	bool);


 	SET_GATHER			(ViewAll,			bool );
 	SET_GATHER			(ViewHiddenText,	bool );
 	SET_GATHER			(ViewUnprintable,	bool );
    SET_GATHER          (AllowCustomToolbars, bool);
    SET_GATHER          (AutoLoadPlugins,    bool);
 	SET_GATHER			(NotebookPageNum,	int );

	SET_GATHER			(OtherDirectionRtl, bool);
	SET_GATHER			(OtherUseContextGlyphs, bool);
	SET_GATHER			(OtherSaveContextGlyphs, bool);
	SET_GATHER			(OtherHebrewContextGlyphs, bool);

	SET_GATHER			(AutoSaveFile, bool);
	virtual void _gatherAutoSaveFilePeriod(UT_String &stRetVal);
	virtual void _setAutoSaveFilePeriod(const UT_String &stPeriod);
	virtual void _gatherAutoSaveFileExt(UT_String &stRetVal);
	virtual void _setAutoSaveFileExt(const UT_String &stExt);

#undef SET_GATHER

 protected:
#if 0
	// private construction functions
	virtual GtkWidget * _constructWindow(void);
	GtkWidget *         _constructWindowContents(GtkWidget *);

	// pointers to widgets we need to query/set
	// there are a ton of them in this dialog

	GtkWidget * m_windowMain;
	GtkWidget * m_notebook;

    GtkWidget * m_checkbuttonSpellCheckAsType;
    GtkWidget * m_checkbuttonSpellHideErrors;
    GtkWidget * m_checkbuttonSpellSuggest;
    GtkWidget * m_checkbuttonSpellMainOnly;
    GtkWidget * m_checkbuttonSpellUppercase;
    GtkWidget * m_checkbuttonSpellNumbers;
    GtkWidget * m_checkbuttonSpellInternet;
	GtkWidget * m_listSpellDicts;
	GtkWidget * m_listSpellDicts_menu;
	GtkWidget * m_buttonSpellDictionary;
	GtkWidget * m_buttonSpellIgnoreEdit;
	GtkWidget * m_buttonSpellIgnoreReset;

    GtkWidget * m_checkbuttonSmartQuotesEnable;
    GtkWidget * m_listDefaultPageSize;

    GtkWidget * m_checkbuttonPrefsAutoSave;
	GtkWidget * m_comboPrefsScheme;

    GtkWidget * m_checkbuttonViewShowRuler;
    GtkWidget * m_listViewRulerUnits;
    GtkWidget * m_listViewRulerUnits_menu;
    GtkWidget * m_checkbuttonViewCursorBlink;
    GtkWidget * m_checkbuttonViewShowStatusBar;

	GtkWidget * m_checkbuttonTransparentIsWhite;
	GtkWidget * m_pushbuttonNewTransparentColor;

	GtkWidget * m_checkbuttonAllowCustomToolbars;
	GtkWidget * m_checkbuttonAutoLoadPlugins;

    GtkWidget * m_checkbuttonViewShowTB;
    GtkWidget * m_checkbuttonViewHideTB;
    GtkWidget * m_toolbarClist;

    GtkWidget * m_checkbuttonViewAll;
    GtkWidget * m_checkbuttonViewHiddenText;
    GtkWidget * m_checkbuttonViewUnprintable;

    GtkWidget * m_checkbuttonOtherDirectionRtl;
    GtkWidget * m_checkbuttonOtherUseContextGlyphs;
    GtkWidget * m_checkbuttonOtherSaveContextGlyphs;
    GtkWidget * m_checkbuttonOtherHebrewContextGlyphs;

	GtkWidget * m_checkbuttonAutoSaveFile;
	GtkWidget * m_textAutoSaveFilePeriod;
	GtkWidget * m_textAutoSaveFileExt;
	GtkWidget * m_checkbuttonShowSplash;
	GtkWidget * m_checkbuttonFontWarning;
	GtkWidget * m_checkbuttonFontPath;
	GtkWidget * m_buttonDefaults;
	GtkWidget * m_buttonApply;
	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;

protected:
	// Cocoa call back handlers
	static void s_delete_clicked		( GtkWidget *, GdkEvent *, gpointer );
	static void s_allowTransparentColor ( GtkWidget *, gpointer );
	static void s_color_changed(GtkWidget * csel,  AP_CocoaDialog_Options * dlg);
	static void s_clist_clicked (GtkWidget *, gint, gint, GdkEvent *, gpointer);

	static void s_checkbutton_toggle	( GtkWidget *, gpointer );
	static gint s_menu_item_activate	( GtkWidget *, gpointer );

	// callbacks can fire these events
    virtual void event_WindowDelete(void);
    virtual void event_clistClicked (int row, int col);
#endif
private:
	friend class AP_CocoaDialog_OptionsController_proxy;	// this private class is to allow accessing protected methods 
	                                                        // from the Obj-C interfaces.
	AP_CocoaDialog_OptionsController*	m_dlg;
};

#endif /* AP_COCOADIALOG_OPTIONS_H */

