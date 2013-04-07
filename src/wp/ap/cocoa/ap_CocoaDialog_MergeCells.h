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

#ifndef AP_CocoaDIALOG_MERGECELLS_H
#define AP_CocoaDIALOG_MERGECELLS_H

#import <Cocoa/Cocoa.h>

#include "ap_Dialog_MergeCells.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_MergeCellsController;
@protocol XAP_CocoaDialogProtocol;

/*****************************************************************/

class AP_CocoaDialog_MergeCells: public AP_Dialog_MergeCells
{
public:
	AP_CocoaDialog_MergeCells(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_MergeCells(void);

	virtual void			runModeless(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events
	void			event_Close(void);
	virtual void            setSensitivity(AP_Dialog_MergeCells::mergeWithCell mergeThis, bool bsens);
	virtual void            destroy(void);
	virtual void            activate(void);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
	const char* getWindowName() { return m_WindowName; };
private:
	void _populateWindowData(void);
	void _storeWindowData(void);
	AP_CocoaDialog_MergeCellsController* m_dlg;
};

@interface AP_CocoaDialog_MergeCellsController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_mergeAboveBtn;
    IBOutlet NSButton *_mergeBelowBtn;
    IBOutlet NSButton *_mergeLeftBtn;
    IBOutlet NSButton *_mergeRightBtn;
	IBOutlet NSBox *_mergeCellsBox;
	AP_CocoaDialog_MergeCells* _xap;
}
- (IBAction)mergeAbove:(id)sender;
- (IBAction)mergeBelow:(id)sender;
- (IBAction)mergeLeft:(id)sender;
- (IBAction)mergeRight:(id)sender;
- (void)setEnableButton:(AP_Dialog_MergeCells::mergeWithCell)btn to:(bool)val;

@end

#endif /* AP_CocoaDIALOG_MERGECELLS_H */
