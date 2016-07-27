/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyritht (C) 2003 Hubert Figuiere
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

#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"

#include "ap_CocoaDialog_PageSetup.h"

/*********************************************************************************/

XAP_Dialog * AP_CocoaDialog_PageSetup::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_PageSetup * p = new AP_CocoaDialog_PageSetup(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_PageSetup::AP_CocoaDialog_PageSetup (XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	AP_Dialog_PageSetup(pDlgFactory, dlgid)
{
	// 
}

AP_CocoaDialog_PageSetup::~AP_CocoaDialog_PageSetup(void)
{
	// 
}

void AP_CocoaDialog_PageSetup::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;

	m_ctrl = [[AP_CocoaDialog_PageSetup_Controller alloc] init];

	[m_ctrl setXAPOwner:this];

	if (![NSBundle loadNibNamed:@"ap_CocoaDialog_PageSetup" owner:m_ctrl]) {
		NSLog(@"Couldn't load nib ap_CocoaDialog_PageSetup");
	}

	NSPrintInfo * printInfo = [NSPrintInfo sharedPrintInfo];

	[printInfo setOrientation:NSPaperOrientationPortrait];

	fp_PageSize fp = getPageSize();

	NSSize paperSize;
	paperSize.width  = (float) fp.Width(DIM_PT);
	paperSize.height = (float) fp.Height(DIM_PT);

	[printInfo setPaperSize:paperSize];

	bool bPortrait = (getPageOrientation() == PORTRAIT);

	[printInfo setOrientation:(bPortrait ? NSPaperOrientationPortrait : NSPaperOrientationLandscape)];

	NSPageLayout * pageLayout = [NSPageLayout pageLayout];

	[pageLayout setAccessoryView:[m_ctrl view]];

	while (true) {
		if ([pageLayout runModalWithPrintInfo:printInfo] != NSOKButton) {
			setAnswer(a_CANCEL);
			break;
		}
		if (_validate(m_ctrl, printInfo)) {
			setAnswer(a_OK);
			break;
		}
	}
	[m_ctrl release];

	m_pFrame = 0;
}

bool AP_CocoaDialog_PageSetup::_validate(AP_CocoaDialog_PageSetup_Controller * /*ctrl*/, NSPrintInfo * printInfo)
{
	NSRect bounds = [printInfo imageablePageBounds];

	/* Printable page rectangle (inches)
	 */
#if 0
	float boundsOriginX = (float) UT_convertDimensions((double) bounds.origin.x, DIM_PT, DIM_IN);
	float boundsOriginY = (float) UT_convertDimensions((double) bounds.origin.y, DIM_PT, DIM_IN);
#endif

	float boundedWidth  = (float) UT_convertDimensions((double) bounds.size.width,  DIM_PT, DIM_IN);
	float boundedHeight = (float) UT_convertDimensions((double) bounds.size.height, DIM_PT, DIM_IN);

	if ((boundedWidth < 1.0) || (boundedHeight < 1.0))
	{
		/* Er. How do we handle ultra-small page sizes? For now, pop up: "The margins selected are too large to fit on the page."
		 */
		m_pFrame->showMessageBox(AP_STRING_ID_DLG_PageSetup_ErrBigMargins, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		return false;
	}

	NSSize paperSize = [printInfo paperSize];
#if 0
	/* Paper size (inches)
	 */
	float width  = (float) UT_convertDimensions((double) paperSize.width,  DIM_PT, DIM_IN);
	float height = (float) UT_convertDimensions((double) paperSize.height, DIM_PT, DIM_IN);

	/* Minimum margin sizes (inches)
	 */
	float    top = height - boundsOriginY - boundedHeight;
	float bottom = boundsOriginY;
	float   left = boundsOriginX;
	float  right = width  - boundsOriginX - boundedWidth;
#endif
	/* Get dialog controller to update base class with margin settings
	 */
	[m_ctrl fetchData];

	/* Update base class with other settings
	 */
	bool bPortrait = ([printInfo orientation] == NSPaperOrientationPortrait);

	setPageOrientation(bPortrait ? PORTRAIT : LANDSCAPE);

	fp_PageSize fp((double) paperSize.width, (double) paperSize.height, DIM_PT);

	setPageSize(fp);

	/* ...
	 */

	/* The window will only close (on an OK click) if the margins fit inside the paper size.
	 */
	bool bValid = true;

	if (!validatePageSettings()) {
		/* "The margins selected are too large to fit on the page."
		 */
		m_pFrame->showMessageBox(AP_STRING_ID_DLG_PageSetup_ErrBigMargins, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		bValid = false;
	}
	return bValid;
}

@implementation AP_CocoaDialog_PageSetup_Controller

- (void)awakeFromNib
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	LocalizeControl(_adjustLabel,  pSS, AP_STRING_ID_DLG_PageSetup_Adjust);
	LocalizeControl(_marginBox,    pSS, AP_STRING_ID_DLG_PageSetup_Page);
	LocalizeControl(_percentLabel, pSS, AP_STRING_ID_DLG_PageSetup_Percent);
	LocalizeControl(_scaleBox,     pSS, AP_STRING_ID_DLG_PageSetup_Scale);
	LocalizeControl(_unitLabel,    pSS, AP_STRING_ID_DLG_PageSetup_Units);

	[_unitPopup removeAllItems];
	AppendLocalizedMenuItem(_unitPopup, pSS, XAP_STRING_ID_DLG_Unit_inch, DIM_IN);
	AppendLocalizedMenuItem(_unitPopup, pSS, XAP_STRING_ID_DLG_Unit_cm,   DIM_CM);
	AppendLocalizedMenuItem(_unitPopup, pSS, XAP_STRING_ID_DLG_Unit_mm,   DIM_MM);

	[_icon setImage:[NSImage imageNamed:@"margin"]];

	if (_xap) {
		[_adjustData    setIntValue:(_xap->getPageScale())];
		[_adjustStepper setIntValue:(_xap->getPageScale())];

		_last_margin_unit = _xap->getMarginUnits();

		[_unitPopup selectItemAtIndex:[_unitPopup indexOfItemWithTag:_last_margin_unit]];

		[   _topMargin setFloatValue:(_xap->getMarginTop())];
		[_bottomMargin setFloatValue:(_xap->getMarginBottom())];
		[  _leftMargin setFloatValue:(_xap->getMarginLeft())];
		[ _rightMargin setFloatValue:(_xap->getMarginRight())];

		[_headerMargin setFloatValue:(_xap->getMarginHeader())];
		[_footerMargin setFloatValue:(_xap->getMarginFooter())];
	}
}

- (void)setXAPOwner:(AP_CocoaDialog_PageSetup *)owner
{
	_xap = owner;
}

- (NSView *)view
{
	return _view;
}

- (void)fetchData
{
	_xap->setPageScale([_adjustData intValue]);

	_xap->setMarginUnits(_last_margin_unit);

	_xap->setMarginTop   ([   _topMargin floatValue]);
	_xap->setMarginBottom([_bottomMargin floatValue]);
	_xap->setMarginLeft  ([  _leftMargin floatValue]);
	_xap->setMarginRight ([ _rightMargin floatValue]);

	_xap->setMarginHeader([_headerMargin floatValue]);
	_xap->setMarginFooter([_footerMargin floatValue]);
}

- (IBAction)adjustAction:(id)sender
{
	int percent = [sender intValue];

	percent = (percent < 1) ? 1 : ((percent > 1000) ? 1000 : percent);

	[_adjustData    setIntValue:percent];
	[_adjustStepper setIntValue:percent];
}

- (IBAction)adjustStepperAction:(id)sender
{
	[_adjustData setIntValue:[sender intValue]];
}

- (IBAction)unitAction:(id)sender
{
	UT_UNUSED(sender);
	UT_Dimension mu = (UT_Dimension) [[_unitPopup selectedItem] tag];

	float top    = [   _topMargin floatValue];
	float bottom = [_bottomMargin floatValue];
	float left   = [  _leftMargin floatValue];
	float right  = [ _rightMargin floatValue];

	float header = [_headerMargin floatValue];
	float footer = [_footerMargin floatValue];
	
	top    = (float) UT_convertDimensions((double) top,    _last_margin_unit, mu);
	bottom = (float) UT_convertDimensions((double) bottom, _last_margin_unit, mu);
	left   = (float) UT_convertDimensions((double) left,   _last_margin_unit, mu);
	right  = (float) UT_convertDimensions((double) right,  _last_margin_unit, mu);

	header = (float) UT_convertDimensions((double) header, _last_margin_unit, mu);
	footer = (float) UT_convertDimensions((double) footer, _last_margin_unit, mu);

	_last_margin_unit = mu;

	[   _topMargin setFloatValue:top   ];
	[_bottomMargin setFloatValue:bottom];
	[  _leftMargin setFloatValue:left  ];
	[ _rightMargin setFloatValue:right ];

	[_headerMargin setFloatValue:header];
	[_footerMargin setFloatValue:footer];
}

@end
