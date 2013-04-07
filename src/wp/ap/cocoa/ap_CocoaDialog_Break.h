/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003, 2009 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_BREAK_H
#define AP_COCOADIALOG_BREAK_H

#import <Cocoa/Cocoa.h>

#include "ap_Dialog_Break.h"
#include "xap_CocoaDialog_Utilities.h"

class AP_CocoaDialog_Break;

@interface AP_CocoaDialog_BreakController : NSWindowController <XAP_CocoaDialogProtocol>
{
@public
    IBOutlet NSMatrix *m_insertRadioBtns;
	IBOutlet NSMatrix *m_sectionBreakBtns;
	IBOutlet NSBox *m_insertGrp;
	IBOutlet NSBox *m_sectionBrkGrp;
	IBOutlet NSButtonCell *m_pgBrkBtn;
	IBOutlet NSButtonCell *m_colBrkBtn;
	IBOutlet NSButtonCell *m_nxtPgBtn;
	IBOutlet NSButtonCell *m_evenPgBtn;
	IBOutlet NSButtonCell *m_continuousBtn;
	IBOutlet NSButtonCell *m_oddPgBtn;
	AP_CocoaDialog_Break * m_xap;		// the app dialog owner
}
- (void)windowDidLoad;

- (IBAction)cancelAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)insertAction:(id)sender;
@end

/*****************************************************************/

class AP_CocoaDialog_Break: public AP_Dialog_Break
{
public:
	AP_CocoaDialog_Break(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_CocoaDialog_Break(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	void _setAnswer (AP_Dialog_Break::tAnswer answer)
		{ 	m_answer = answer; }
	void _setBreakType(AP_Dialog_Break::breakType type)
		{ 	m_breakType = type; }

private:
	void		_populateWindowData(void);
	void 		_storeWindowData(void);

	NSButtonCell * _findRadioByID(AP_Dialog_Break::breakType b);

	AP_CocoaDialog_BreakController *	m_dlg;
	AP_Dialog_Break::breakType			m_breakType;
};

#endif /* AP_COCOADIALOG_BREAK_H */
