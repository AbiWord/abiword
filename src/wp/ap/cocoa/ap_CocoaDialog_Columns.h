/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003-2021 Hubert Figuière
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

#pragma once

#include "ap_Dialog_Columns.h"

#include "ut_types.h"
#include "ut_string.h"

class GR_CocoaGraphics;

class XAP_CocoaFrame;
@class AP_CocoaDialog_ColumnsController;

/*****************************************************************/

class AP_CocoaDialog_Columns: public AP_Dialog_Columns
{
public:
	AP_CocoaDialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_Columns(void);

	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			enableLineBetweenControl(bool bState = true);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events
    void                            doSpaceAfterEntry(void);
	void                            doMaxHeightEntry(const char * s);
    void                            doHeightSpin(void);
	void                            doSpaceAfterSpin(void);
	void                            checkLineBetween(void);
	void							 colNumberChanged(void);
	void                            readSpin(void);
	void                            event_Toggle( UT_uint32 icolumns);
	void                            event_previewExposed(void);
	void			event_OK(void);
	void			event_Cancel(void);

	void			incrMaxHeight(bool bIncrement);
	void			incrSpaceAfter(bool bIncrement);

private:
	GR_CocoaGraphics* m_pPreviewWidget;

	AP_CocoaDialog_ColumnsController *	m_dlg;

	UT_Dimension		m_Dim_MaxHeight;
	UT_Dimension		m_Dim_SpaceAfter;
};


@interface AP_CocoaDialog_ColumnsController : NSWindowController <XAP_CocoaDialogProtocol>
{
	IBOutlet NSButton *			_cancelBtn;
	IBOutlet NSButton *			_okBtn;

	IBOutlet NSButton *			_oneBtn;
	IBOutlet NSButton *			_twoBtn;
	IBOutlet NSButton *			_threeBtn;

	IBOutlet NSButton *			_useRTLBtn;
	IBOutlet NSButton *			_lineBetweenBtn;

	IBOutlet NSStepper *		_maxColSizeStepper;
	IBOutlet NSStepper *		_numOfColumnStepper;
	IBOutlet NSStepper *		_spaceAfterColStepper;

	IBOutlet NSTextField *		_maxColSizeData;
	IBOutlet NSTextField *		_maxColSizeLabel;
	IBOutlet NSTextField *		_numColumn2Label;
	IBOutlet NSTextField *		_numColumnLabel;
	IBOutlet NSTextField *		_numOfColumnData;
	IBOutlet NSTextField *		_spaceAfterColData;
	IBOutlet NSTextField *		_spaceAfterColLabel;
	IBOutlet NSTextField *		_oneLabel;
	IBOutlet NSTextField *		_twoLabel;
	IBOutlet NSTextField *		_threeLabel;

	IBOutlet NSBox *			_previewBox;

	IBOutlet XAP_CocoaNSView *	_preview;

	AP_CocoaDialog_Columns	*_xap;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)lineBetweenAction:(id)sender;
- (IBAction)maxColSizeAction:(id)sender;
- (IBAction)maxColSizeStepperAction:(id)sender;
- (IBAction)numOfColAction:(id)sender;
- (IBAction)numOfColStepperAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)oneAction:(id)sender;
- (IBAction)spaceAfterColAction:(id)sender;
- (IBAction)spaceAfterColStepperAction:(id)sender;
- (IBAction)threeAction:(id)sender;
- (IBAction)twoAction:(id)sender;

- (int)colNum;
- (void)setColNum:(int)num;

- (NSString*)spaceAfter;
- (void)setSpaceAfter:(const char *)str;
- (void)setMaxColHeight:(const char *)str;
- (bool)lineBetween;
- (void)setLineBetween:(bool)b;
- (UT_uint32)columnRTLOrder;
- (void)setColumnRTLOrder:(UT_uint32)val;

- (XAP_CocoaNSView*)preview;

@end
