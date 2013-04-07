/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2004, 2009 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_STYLIST_H
#define AP_COCOADIALOG_STYLIST_H

#import <Cocoa/Cocoa.h>

#include "ap_Dialog_Stylist.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_Stylist_Controller;
@class StyleNode;
@protocol XAP_CocoaDialogProtocol;

/*****************************************************************/

class AP_CocoaDialog_Stylist: public AP_Dialog_Stylist
{
public:
	AP_CocoaDialog_Stylist(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_Stylist(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	virtual void      setSensitivity(bool bSens);

	// callbacks can fire these events
	void			event_Close(void);
	void            event_Apply(void);
	void            styleClicked(UT_sint32 row, UT_sint32 col);

	virtual void            destroy(void);
	virtual void            activate(void);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
	virtual void            setStyleInGUI(void);

	NSMutableArray *		getItems(void) { return m_items; }

private:
	void            _fillTree(void);
	void			_populateWindowData(void);

	AP_CocoaDialog_Stylist_Controller *	m_dlg;

	NSMutableArray *	m_items;
	bool				m_bModal;
	bool				m_bDialogClosed;
};

@interface AP_CocoaDialog_Stylist_Controller : NSWindowController
	<XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_applyBtn;
    IBOutlet NSOutlineView *_stylistList;
	AP_CocoaDialog_Stylist* _xap;
	BOOL _enabled;
}
- (void)refresh;
- (void)selectStyleNode:(StyleNode *)childNode childOf:(StyleNode *)parentNode;
- (IBAction)applyAction:(id)sender;
- (IBAction)outlineAction:(id)sender;
- (IBAction)outlineDoubleAction:(id)sender;
- (void)setSensitivity:(bool)bSens;
@end

#endif /* AP_COCOADIALOG_STYLIST_H */
