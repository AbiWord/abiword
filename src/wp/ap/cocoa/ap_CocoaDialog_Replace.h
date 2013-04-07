/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef AP_COCOADIALOG_REPLACE_H
#define AP_COCOADIALOG_REPLACE_H

#include "ap_Dialog_Replace.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_ReplaceController;
@protocol XAP_CocoaDialogProtocol;

/*****************************************************************/

class AP_CocoaDialog_Replace: public AP_Dialog_Replace
{
public:
	AP_CocoaDialog_Replace(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_Replace(void);


	virtual void			runModal(XAP_Frame * /*pFrame*/){};
	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);
	virtual void			notifyCloseFrame(XAP_Frame */*pFrame*/){};
	virtual void			destroy(void);
	virtual void			activate(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events
	void					event_Find(void);
	void					event_Replace(void);
	void					event_ReplaceAll(void);
	void					event_MatchCaseToggled(void);
	void					event_WholeWordToggled(void);
	void					event_ReverseFindToggled(void);
	void					event_Cancel(void);
	void					event_CloseWindow(void);

private:
	void			_updateLists();
	void 		_storeWindowData(void) {};
	void		_populateWindowData(void);
	AP_CocoaDialog_ReplaceController* m_dlg;
};


@interface AP_CocoaDialog_ReplaceController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_findAndReplaceBtn;
    IBOutlet NSButton *_findBtn;
	IBOutlet NSButton *_findReverseBtn;
    IBOutlet NSButton *_matchCaseBtn;
    IBOutlet NSButton *_replaceAll;
    IBOutlet NSComboBox *_replaceCombo;
    IBOutlet NSTextField *_replaceLabel;
    IBOutlet NSComboBox *_whatCombo;
    IBOutlet NSTextField *_whatLabel;
    IBOutlet NSButton *_wholeWordBtn;
	AP_CocoaDialog_Replace*	_xap;
}
- (void)windowToFront;

- (IBAction)findAction:(id)sender;
- (IBAction)findAndReplaceAction:(id)sender;
- (IBAction)findReverseAction:(id)sender;
- (IBAction)matchCaseAction:(id)sender;
- (IBAction)wholeWordAction:(id)sender;
- (IBAction)replaceAllAction:(id)sender;


- (NSString*)findWhat;
- (void)setFindWhat:(NSString*)str;
- (NSString*)replaceWith;
- (void)setReplaceWith:(NSString*)str;
- (bool)matchCase;
- (void)setMatchCase:(bool)val;
- (bool)wholeWord;
- (void)setWholeWord:(bool)val;
- (bool)findReverse;
- (void)setFindReverse:(bool)val;

- (void)updateFindWhat:(UT_GenericVector<UT_UCS4Char*>*)list;
- (void)updateReplaceWith:(UT_GenericVector<UT_UCS4Char*>*)list;
- (void)_updateCombo:(NSComboBox*)combo withList:(UT_GenericVector<UT_UCS4Char*>*)list;
@end

#endif /* AP_COCOADIALOG_REPLACE_H */
