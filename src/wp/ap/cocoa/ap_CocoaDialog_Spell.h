/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_COCOADIALOG_SPELL_H
#define AP_COCOADIALOG_SPELL_H

#ifdef ENABLE_SPELL

#include "ap_Dialog_Spell.h"

#import "xap_Cocoa_NSTableUtils.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_Spell_Controller;


/*****************************************************************/

class AP_CocoaDialog_Spell: public AP_Dialog_Spell
{
public:
	AP_CocoaDialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_Spell(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events
	virtual void event_Change(void);
	virtual void event_ChangeAll(void);
	virtual void event_Ignore(void);
	virtual void event_IgnoreAll(void);
	virtual void event_AddToDict(void);
	virtual void event_Cancel(void);
	virtual void event_SuggestionSelected(int row, int column);
	virtual void event_ReplacementChanged(void);

private:
	void	    _populateWindowData(void);
	void 	    _storeWindowData(void);

	void _showMisspelledWord(void);

	AP_CocoaDialog_Spell_Controller* m_dlg;
	XAP_StringListDataSource* m_suggestionList;
};


@interface AP_CocoaDialog_Spell_Controller : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_addBtn;
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSButton *_changeAllBtn;
    IBOutlet NSButton *_changeBtn;
    IBOutlet NSButton *_ignoreAllBtn;
    IBOutlet NSButton *_ignoreBtn;
    IBOutlet NSTextField *_replData;
    IBOutlet NSTextField *_replLabel;
    IBOutlet NSTableView *_suggestionList;
    IBOutlet NSTextView *_unknownData;
    IBOutlet NSTextField *_unknownLabel;
	AP_CocoaDialog_Spell* _xap;
}
- (IBAction)addToDictAction:(id)sender;
- (IBAction)cancelAction:(id)sender;
- (IBAction)changeAction:(id)sender;
- (IBAction)changeAllAction:(id)sender;
- (IBAction)ignoreAction:(id)sender;
- (IBAction)ignoreAllAction:(id)sender;
- (IBAction)replacementChanged:(id)sender;
- (void)suggestionSelected:(id)sender;

- (void)setMisspelled:(NSAttributedString*)attr scroll:(float)offset;
- (void)setReplace:(NSString*)str;
- (void)selectSuggestion:(int)idx;
- (void)reloadSuggestionList;
- (void)setSuggestionList:(id)list;
- (NSString*)replace;
@end

#endif

#endif /* AP_COCOADIALOG_SPELL_H */
