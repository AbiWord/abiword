/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
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

#ifndef AP_CocoaDIALOG_SPLITCELLS_H
#define AP_CocoaDIALOG_SPLITCELLS_H

#import <Cocoa/Cocoa.h>

#include "ap_Dialog_SplitCells.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_SplitCellsController;
@protocol XAP_CocoaDialogProtocol;

/*****************************************************************/

class AP_CocoaDialog_SplitCells: public AP_Dialog_SplitCells
{
public:
	AP_CocoaDialog_SplitCells(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_SplitCells(void);

	virtual void			runModeless(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events
	void			event_Close(void);
	virtual void            setSensitivity(AP_CellSplitType splittype, bool bsens);
	virtual void            destroy(void);
	virtual void            activate(void);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
	const char* getWindowName() { return m_WindowName; };
private:
	void _populateWindowData(void);
	void _storeWindowData(void);
	AP_CocoaDialog_SplitCellsController* m_dlg;
};

@interface AP_CocoaDialog_SplitCellsController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_splitLeftBtn;
    IBOutlet NSButton *_splitMiddleHBtn;
    IBOutlet NSButton *_splitRightBtn;
    IBOutlet NSButton *_splitTopBtn;
    IBOutlet NSButton *_splitMiddleVBtn;
    IBOutlet NSButton *_splitBottomBtn;
	IBOutlet NSBox *_splitCellsBox;
	AP_CocoaDialog_SplitCells* _xap;
}
- (IBAction)splitLeft:(id)sender;
- (IBAction)splitMiddleH:(id)sender;
- (IBAction)splitRight:(id)sender;
- (IBAction)splitTop:(id)sender;
- (IBAction)splitMiddleV:(id)sender;
- (IBAction)splitBottom:(id)sender;
- (void)setEnableButton:(AP_CellSplitType)btn to:(bool)val;

@end

#endif /* AP_CocoaDIALOG_MERGECELLS_H */
