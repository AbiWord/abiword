/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003, 2009-2021 Hubert Figuière
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

#include <stdlib.h>
#include <math.h>

#include <string>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "gr_CocoaGraphics.h"
#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Columns.h"
#include "ap_CocoaDialog_Columns.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_Columns::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_Columns * p = new AP_CocoaDialog_Columns(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_Columns::AP_CocoaDialog_Columns(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_Columns(pDlgFactory,dlgid),
		m_pPreviewWidget(NULL),
		m_dlg(nil)
{
}

AP_CocoaDialog_Columns::~AP_CocoaDialog_Columns(void)
{
	DELETEP (m_pPreviewWidget);
}

/*****************************************************************/

void AP_CocoaDialog_Columns::runModal(XAP_Frame * pFrame)
{
	m_dlg = [[AP_CocoaDialog_ColumnsController alloc] initFromNib];
	[m_dlg setXAPOwner:this];
	setViewAndDoc(pFrame);

	NSWindow * window = [m_dlg window];

	const char * szHeight = getHeightString();
	m_Dim_MaxHeight = UT_determineDimension(szHeight, DIM_none);
	[m_dlg setMaxColHeight:szHeight];

	const char * szAfter = getSpaceAfterString();
	m_Dim_SpaceAfter = UT_determineDimension(szAfter, DIM_none);
	[m_dlg setSpaceAfter:szAfter];

	// make a new Cocoa GC
	DELETEP (m_pPreviewWidget);
	XAP_CocoaNSView *preview = [m_dlg preview];
	GR_CocoaAllocInfo ai(preview);
	m_pPreviewWidget = (GR_CocoaGraphics*)XAP_App::getApp()->newGraphics(ai);

	NSSize size = [preview frame].size;

	_createPreviewFromGC(m_pPreviewWidget,
			     (UT_uint32) lrintf(size.width),
			     (UT_uint32) lrintf(size.height));
	m_dlg.preview.drawable = getColumnsPreview();

//	setLineBetween(getLineBetween());  // isn't that a little useless ? Grafted from GTK...
	[m_dlg setLineBetween:getLineBetween()];
	[m_dlg setColumnRTLOrder:getColumnOrder()];

	event_Toggle(getColumns());

	[NSApp runModalForWindow:window];

	setColumnOrder ([m_dlg columnRTLOrder]);

	DELETEP (m_pPreviewWidget);

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

void AP_CocoaDialog_Columns::checkLineBetween(void)
{
	setLineBetween([m_dlg lineBetween]);
}


void AP_CocoaDialog_Columns::colNumberChanged(void)
{
	UT_sint32 val = (UT_sint32) [m_dlg colNum];
	if(val < 1)
		return;
	event_Toggle(val);
}

void AP_CocoaDialog_Columns::event_Toggle( UT_uint32 icolumns)
{
	checkLineBetween();

	[m_dlg setColNum:((int) icolumns)];
	
	setColumns(icolumns);
	getColumnsPreview()->queueDraw();
}


void AP_CocoaDialog_Columns::event_OK(void)
{
	m_answer = AP_Dialog_Columns::a_OK;
	[NSApp stopModal];
}


void AP_CocoaDialog_Columns::doMaxHeightEntry(const char * szMaxHeight)
{
	UT_ASSERT(szMaxHeight);
	std::string szHeight = szMaxHeight ? szMaxHeight : "";

	UT_Dimension new_dimension = UT_determineDimension(szHeight.c_str(), DIM_none);

	if (new_dimension == DIM_none)
	{
		szHeight += UT_dimensionName(m_Dim_MaxHeight);
	}
	else
	{
		m_Dim_MaxHeight = new_dimension;
	}
	setMaxHeight(szHeight.c_str());

	[m_dlg setMaxColHeight:getHeightString()];
}

void AP_CocoaDialog_Columns::doSpaceAfterEntry(void)
{
	std::string szAfter = [[m_dlg spaceAfter] UTF8String];

    UT_Dimension new_dimension = UT_determineDimension(szAfter.c_str(), DIM_none);

	if (new_dimension == DIM_none)
	{
		szAfter += UT_dimensionName(m_Dim_SpaceAfter);
	}
	else
	{
		m_Dim_SpaceAfter = new_dimension;
	}
	setSpaceAfter(szAfter.c_str());

	[m_dlg setSpaceAfter:getSpaceAfterString()];
}

void AP_CocoaDialog_Columns::event_Cancel(void)
{
	m_answer = AP_Dialog_Columns::a_CANCEL;
	[NSApp stopModal];
}

void AP_CocoaDialog_Columns::incrMaxHeight(bool bIncrement)
{
	incrementMaxHeight(bIncrement);
	[m_dlg setMaxColHeight:getHeightString()];
}

void AP_CocoaDialog_Columns::incrSpaceAfter(bool bIncrement)
{
	incrementSpaceAfter(bIncrement);
	[m_dlg setSpaceAfter:getSpaceAfterString()];
}

/*****************************************************************/

void AP_CocoaDialog_Columns::enableLineBetweenControl(bool /*bState*/)
{
}


@implementation AP_CocoaDialog_ColumnsController

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_Columns"];
}

-(void)discardXAP
{
	_xap = NULL; 
}

-(void)dealloc
{
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_Columns*>(owner);

	// There is no guarantee this is not nullptr, but this makes it consistent.
	self.preview.drawable = _xap->getColumnsPreview();
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();

		LocalizeControl([self window],			pSS, AP_STRING_ID_DLG_Column_ColumnTitle);

		LocalizeControl(_okBtn,					pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn,				pSS, XAP_STRING_ID_DLG_Cancel);

		LocalizeControl(_numColumnLabel,		pSS, AP_STRING_ID_DLG_Column_Number);
		LocalizeControl(_previewBox,			pSS, AP_STRING_ID_DLG_Column_Preview);
		LocalizeControl(_lineBetweenBtn,		pSS, AP_STRING_ID_DLG_Column_Line_Between);
		LocalizeControl(_useRTLBtn,				pSS, AP_STRING_ID_DLG_Column_RtlOrder);
		LocalizeControl(_numColumn2Label,		pSS, AP_STRING_ID_DLG_Column_Number_Cols);
		LocalizeControl(_spaceAfterColLabel,	pSS, AP_STRING_ID_DLG_Column_Space_After);
		LocalizeControl(_maxColSizeLabel,		pSS, AP_STRING_ID_DLG_Column_Size);

		LocalizeControl(_oneLabel,				pSS, AP_STRING_ID_DLG_Column_One);
		LocalizeControl(_twoLabel,				pSS, AP_STRING_ID_DLG_Column_Two);
		LocalizeControl(_threeLabel,			pSS, AP_STRING_ID_DLG_Column_Three);

		[[  _oneBtn cell] setShowsStateBy:NSNoCellMask]; /* TODO: should really do this in XAP_CocoaToolbarButton */
		[[  _oneBtn cell] setHighlightsBy:NSNoCellMask];
		[[  _twoBtn cell] setShowsStateBy:NSNoCellMask];
		[[  _twoBtn cell] setHighlightsBy:NSNoCellMask];
		[[_threeBtn cell] setShowsStateBy:NSNoCellMask];
		[[_threeBtn cell] setHighlightsBy:NSNoCellMask];
	}
}
	
- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_OK();
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel();
}

- (IBAction)lineBetweenAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->checkLineBetween();
}

- (IBAction)maxColSizeAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->doMaxHeightEntry([[_maxColSizeData stringValue] UTF8String]);
}

- (IBAction)maxColSizeStepperAction:(id)sender
{
	UT_UNUSED(sender);
	bool bIncr = ([_maxColSizeStepper intValue] == 0) ? false : true;

	[_maxColSizeStepper setIntValue:1];

	_xap->incrMaxHeight(bIncr);
}

- (IBAction)numOfColAction:(id)sender
{
	UT_UNUSED(sender);
	int count = [sender intValue];

	count = (count < 1) ? 1 : ((count > 20) ? 20 : count);

	_xap->colNumberChanged();
}

- (IBAction)numOfColStepperAction:(id)sender
{
	UT_UNUSED(sender);
	[_numOfColumnData setIntValue:[sender intValue]];

	_xap->colNumberChanged();
}

- (IBAction)oneAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Toggle(1);
}

- (IBAction)twoAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Toggle(2);
}

- (IBAction)threeAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Toggle(3);
}

- (int)colNum
{
	return [_numOfColumnData intValue];
}

- (void)setColNum:(int)num
{
	[_numOfColumnData    setIntValue:num];
	[_numOfColumnStepper setIntValue:num];

	[  _oneBtn setState:((num == 1) ? NSControlStateValueOn : NSControlStateValueOff)];
	[  _twoBtn setState:((num == 2) ? NSControlStateValueOn : NSControlStateValueOff)];
	[_threeBtn setState:((num == 3) ? NSControlStateValueOn : NSControlStateValueOff)];
}

- (IBAction)spaceAfterColAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->doSpaceAfterEntry();
}

- (IBAction)spaceAfterColStepperAction:(id)sender
{
	UT_UNUSED(sender);
	bool bIncr = ([_spaceAfterColStepper intValue] == 0) ? false : true;

	[_spaceAfterColStepper setIntValue:1];

	_xap->incrSpaceAfter(bIncr);
}

- (NSString *)spaceAfter
{
	return [_spaceAfterColData stringValue];
}

- (void)setSpaceAfter:(const char *)str
{
	[_spaceAfterColData setStringValue:[NSString stringWithUTF8String:str]];
}

- (void)setMaxColHeight:(const char *)str
{
	[_maxColSizeData setStringValue:[NSString stringWithUTF8String:str]];
}

- (bool)lineBetween
{
	return ([_lineBetweenBtn state] == NSControlStateValueOn);
}

- (void)setLineBetween:(bool)b
{
	[_lineBetweenBtn setState:(b ? NSControlStateValueOn : NSControlStateValueOff)];
}

- (UT_uint32)columnRTLOrder
{
	return [_useRTLBtn state];
}

- (void)setColumnRTLOrder:(UT_uint32)val
{
	[_useRTLBtn setState:((val == 0) ? NSControlStateValueOff : NSControlStateValueOn)];
}

- (XAP_CocoaNSView *)preview
{
	return _preview;
}

@end
