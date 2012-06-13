/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_COCOADIALOG_PAGENUMBERS_H
#define AP_COCOADIALOG_PAGENUMBERS_H

#include "ut_types.h"
#include "ap_Dialog_PageNumbers.h"

class XAP_CocoaFrame;
class GR_CocoaCairoGraphics;
@class AP_CocoaDialog_PageNumbersController;
@protocol XAP_CocoaDialogProtocol;

class AP_CocoaDialog_PageNumbers : public AP_Dialog_PageNumbers
{
public:
	AP_CocoaDialog_PageNumbers(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_PageNumbers(void);
	
	virtual void runModal(XAP_Frame * pFrame);
	
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	
	void event_OK(void);
	void event_Cancel(void);
	void event_WindowDelete(void);
	void event_PreviewExposed(void);
	void event_AlignChanged(AP_Dialog_PageNumbers::tAlign);
	void event_HdrFtrChanged(AP_Dialog_PageNumbers::tControl);

private:
	AP_Dialog_PageNumbers::tAlign m_recentAlign;
	AP_Dialog_PageNumbers::tControl m_recentControl;
	GR_CocoaCairoGraphics * m_pG;
	AP_CocoaDialog_PageNumbersController * m_dlg;
};


@interface AP_CocoaDialog_PageNumbersController : NSWindowController <XAP_CocoaDialogProtocol>
{
	IBOutlet NSButton *		_cancelBtn;
	IBOutlet NSButton *		_okBtn;

	IBOutlet NSBox *		_positionBox;
	IBOutlet NSBox *		_alignmentBox;
	IBOutlet NSBox *		_previewBox;

	IBOutlet NSButtonCell *	_headerBtn;
	IBOutlet NSButtonCell *	_footerBtn;

	IBOutlet NSButtonCell *	_leftBtn;
	IBOutlet NSButtonCell *	_centerBtn;
	IBOutlet NSButtonCell *	_rightBtn;

	IBOutlet NSMatrix *		_positionMatrix;
	IBOutlet NSMatrix *		_alignmentMatrix;

	IBOutlet XAP_CocoaNSView *_preview;

	AP_CocoaDialog_PageNumbers*	_xap;
}
- (IBAction)alignmentAction:(id)sender;
- (IBAction)cancelAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)positionAction:(id)sender;
- (XAP_CocoaNSView*)preview;
@end

#endif /* AP_COCOADIALOG_PAGENUBMERS_H */
