/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003 Hubert Figuiere
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

#import "ap_CocoaFrame.h"

#include "ap_Dialog_Options.h"
#import "xap_Cocoa_NSTableUtils.h"


class XAP_Frame;
class AP_CocoaDialog_Options;


@interface AP_CocoaDialog_OptionsController : NSWindowController
{
    IBOutlet NSButton *m_cancelBtn;
    IBOutlet NSButton *m_defaultsBtn;
    IBOutlet NSTextField *m_chooseScreenColorLabel;
	IBOutlet NSColorWell* m_screenColor;
    IBOutlet NSButton *m_layoutCursorBlinkBtn;
    IBOutlet NSButton *m_layoutCustomToolbarBtn;
    IBOutlet NSButton *m_layoutEnableSmartQuotesBtn;
    IBOutlet NSBox *m_layoutUIBox;
    IBOutlet NSTextField *m_layoutUnitsLabel;
    IBOutlet NSPopUpButton *m_layoutUnitsPopup;
    IBOutlet NSBox *m_prefsAutoSaveBox;
    IBOutlet NSButton *m_prefsAutoSaveCurrentBtn;
    IBOutlet NSTextField *m_prefsAutoSaveMinField;
	IBOutlet NSStepper *m_prefsAutoSaveMinStepper;
    IBOutlet NSBox *m_prefsBidiBox;
    IBOutlet NSButton *m_prefsDefaultToRTLBtn;
    IBOutlet NSButton *m_prefsLoadAllPluginsBtn;
    IBOutlet NSTextField *m_prefsMinutesLabel;
    IBOutlet NSBox *m_generalAppStartupBox;
    IBOutlet NSButton *m_prefsOtherHebrwContextGlyphBtn;
    IBOutlet NSButton *m_prefsShowSplashBtn;
    IBOutlet NSTextField *m_prefsWithExtField;
    IBOutlet NSTextField *m_prefsWithExtLabel;
    IBOutlet NSButton *m_spellCheckAsTypeBtn;
    IBOutlet NSBox *m_spellGeneralBox;
    IBOutlet NSButton *m_spellHighlightMisspellBtn;
    IBOutlet NSBox *m_spellIgnoreBox;
    IBOutlet NSButton *m_spellIgnoreUppercaseBtn;
    IBOutlet NSButton *m_spellIgnoreWordsWithNumBtn;
	IBOutlet NSBox *m_spellDictBox;
    IBOutlet NSButton *m_spellAlwaysSuggestBtn;
    IBOutlet NSButton *m_spellMainDictSuggOnlyBtn;	
    IBOutlet NSTabView *m_tab;
	
	AP_CocoaDialog_Options * m_xap;
	XAP_StringListDataSource* m_tlbTlbListDataSource;
}
- (id)initFromNib;
- (oneway void)dealloc;
- (void)windowDidLoad;
- (void)setXAPOwner:(AP_CocoaDialog_Options *)owner;
- (id)_lookupWidget:(AP_Dialog_Options::tControl)controlId;
- (IBAction)cancelAction:(id)sender;
- (IBAction)chooseScreenAction:(id)sender;
- (IBAction)defaultAction:(id)sender;
- (IBAction)increaseMinutesAction:(id)sender;
- (IBAction)autoSaveStepperAction:(id)sender;
- (IBAction)autoSaveFieldAction:(id)sender;
- (IBAction)_defaultControlAction:(id)sender;
@end

/*****************************************************************/
class AP_CocoaDialog_Options: public AP_Dialog_Options
{
public:
	AP_CocoaDialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_Options(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	
	AP_CocoaFrame * _getFrame()
		{ return static_cast<AP_CocoaFrame*>(m_pFrame); };
	//void initializeTransperentToggle(void);
	void event_ChooseTransparentColor(void);
	void event_AllowTransparentColor(void);

    virtual void event_Cancel(void);

 protected:
    void _saveCocoaOnlyPrefs();
    void _initCocoaOnlyPrefs();
    virtual void _storeWindowData(void);

	virtual void _controlEnable( tControl ctlid, bool value );

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void _set##a(t)

 	SET_GATHER			(SpellCheckAsType,	bool );
 	SET_GATHER			(SpellHideErrors,	bool );
 	SET_GATHER			(SpellSuggest,		bool );
 	SET_GATHER			(SpellMainOnly,		bool );
 	SET_GATHER			(SpellUppercase,	bool );
 	SET_GATHER			(SpellNumbers,		bool );

 	SET_GATHER			(ShowSplash,	bool);

	SET_GATHER			(SmartQuotesEnable,	bool );

 	SET_GATHER			(PrefsAutoSave,		bool );

	SET_GATHER			(ViewRulerUnits,	UT_Dimension);
	SET_GATHER			(ViewCursorBlink,	bool);


 	SET_GATHER			(ViewAll,			bool );
 	SET_GATHER			(ViewHiddenText,	bool );
 	SET_GATHER			(ViewUnprintable,	bool );
    SET_GATHER          (AllowCustomToolbars, bool);
    SET_GATHER          (AutoLoadPlugins,    bool);
 	SET_GATHER			(NotebookPageNum,	int );

	SET_GATHER			(OtherDirectionRtl, bool);
	SET_GATHER			(OtherHebrewContextGlyphs, bool);

	SET_GATHER			(AutoSaveFile, bool);
	virtual void _gatherAutoSaveFilePeriod(UT_String &stRetVal);
	virtual void _setAutoSaveFilePeriod(const UT_String &stPeriod);
	virtual void _gatherAutoSaveFileExt(UT_String &stRetVal);
	virtual void _setAutoSaveFileExt(const UT_String &stExt);

#undef SET_GATHER

private:
	friend class AP_CocoaDialog_OptionsController_proxy;	// this private class is to allow accessing protected methods 
	                                                        // from the Obj-C interfaces.
	AP_CocoaDialog_OptionsController*	m_dlg;
};

#endif /* AP_COCOADIALOG_OPTIONS_H */

