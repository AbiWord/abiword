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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <string.h>

#include "ut_assert.h"
#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "ap_Strings.h"
#include "xap_CocoaDialog_Utilities.h"

#include "ap_CocoaDialog_PageSetup.h"

#include "ut_debugmsg.h"

/*********************************************************************************/

// some static variables
static fp_PageSize::Predefined last_page_size = fp_PageSize::psCustom;

/*********************************************************************************/

void AP_CocoaDialog_PageSetup::_validate(AP_CocoaDialog_PageSetup_Controller* ctrl)
{
	fp_PageSize fp (last_page_size);

	if(fp.Width(DIM_IN) < 1.0 || fp.Height(DIM_IN) < 1.0)
	{
		setAnswer(a_CANCEL);
		return;
	}

	[m_ctrl fetchData];

	// The window will only close (on an OK click) if the margins
	// fit inside the paper size.
	if ( validatePageSettings() ) {
		setAnswer (a_OK);
	}
	else {
		// "The margins selected are too large to fit on the page."
		m_pFrame->showMessageBox(AP_STRING_ID_DLG_PageSetup_ErrBigMargins, 
								 XAP_Dialog_MessageBox::b_O,
								 XAP_Dialog_MessageBox::a_OK);
	}
}


/*********************************************************************************/

XAP_Dialog *
AP_CocoaDialog_PageSetup::static_constructor(XAP_DialogFactory * pFactory,
					    XAP_Dialog_Id dlgid)
{
    AP_CocoaDialog_PageSetup * p = new AP_CocoaDialog_PageSetup(pFactory,dlgid);
    return p;
}

AP_CocoaDialog_PageSetup::AP_CocoaDialog_PageSetup (XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id dlgid) 
  : AP_Dialog_PageSetup (pDlgFactory, dlgid)
{
  // nada
}

AP_CocoaDialog_PageSetup::~AP_CocoaDialog_PageSetup (void)
{
  // nada
}

void AP_CocoaDialog_PageSetup::runModal (XAP_Frame *pFrame)
{
	NSPageLayout* pageLayout = [NSPageLayout pageLayout];
	NSPrintInfo*	printInfo = [[NSPrintInfo alloc] init];
	m_pFrame = pFrame;

	m_ctrl = [[AP_CocoaDialog_PageSetup_Controller alloc] init];
	if (![NSBundle loadNibNamed:@"ap_CocoaDialog_PageSetup" owner:m_ctrl]) {
		NSLog(@"Couldn't load nib ap_CocoaDialog_PageSetup");
	}
	[m_ctrl setXAPOwner:this];
	[pageLayout setAccessoryView:[m_ctrl view]];
	int retval = [pageLayout runModalWithPrintInfo:printInfo];
	switch (retval) {
	case NSOKButton:
		setAnswer (a_OK);
		
		break;
	case NSCancelButton:
		setAnswer (a_CANCEL);
		break;
	default:
		break;
	}
	
	[m_ctrl release];
	[printInfo release];
	m_pFrame = NULL;
}


#pragma mark ------------------------------------------------------

#ifdef _
#undef _
#endif
#define _(a, x) a##_STRING_ID_##x

@implementation AP_CocoaDialog_PageSetup_Controller

- (void)awakeFromNib
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	LocalizeControl (_adjustLabel, pSS, _(AP, DLG_PageSetup_Adjust));
	LocalizeControl (_marginBox, pSS, _(AP, DLG_PageSetup_Page));
	LocalizeControl (_percentLabel, pSS, _(AP, DLG_PageSetup_Percent));
	LocalizeControl (_scaleBox, pSS, _(AP, DLG_PageSetup_Scale));
	LocalizeControl (_unitLabel, pSS, _(AP, DLG_PageSetup_Units));

	[_unitPopup removeAllItems];
	AppendLocalizedMenuItem(_unitPopup, pSS, _(XAP, DLG_Unit_inch), DIM_IN);
	AppendLocalizedMenuItem(_unitPopup, pSS, _(XAP, DLG_Unit_cm), DIM_CM);
	AppendLocalizedMenuItem(_unitPopup, pSS, _(XAP, DLG_Unit_mm), DIM_MM);

	[_icon setImage:[NSImage imageNamed:@"margin"]];
}

- (void)setXAPOwner:(AP_CocoaDialog_PageSetup*)owner
{
	_xap = owner;
	[_adjustData setIntValue:_xap->getPageScale()];
	_last_margin_unit = _xap->getMarginUnits();
	[_unitPopup selectItemAtIndex:[_unitPopup indexOfItemWithTag:_last_margin_unit]];

	[_bottomMargin setFloatValue:_xap->getMarginBottom()];
	[_footerMargin setFloatValue:_xap->getMarginFooter()];
	[_headerMargin setFloatValue:_xap->getMarginHeader()];
	[_leftMargin setFloatValue:_xap->getMarginLeft()];
	[_rightMargin setFloatValue:_xap->getMarginRight()];
	[_topMargin setFloatValue:_xap->getMarginTop()];
}

- (NSView*)view
{
	return _view;
}

- (void)fetchData
{
//	_xap->setPageSize (fp);
	_xap->setMarginUnits(_last_margin_unit);
//	_xap->setPageUnits(last_page_unit);
//	_xap->setPageOrientation(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioPagePortrait)) ? PORTRAIT : LANDSCAPE);
	_xap->setPageScale([_adjustData intValue]);

	_xap->setMarginTop([_topMargin floatValue]);
	_xap->setMarginBottom([_bottomMargin floatValue]);
	_xap->setMarginLeft([_leftMargin floatValue]);
	_xap->setMarginRight([_rightMargin floatValue]);
	_xap->setMarginHeader([_headerMargin floatValue]);
	_xap->setMarginFooter([_footerMargin floatValue]);
}


- (IBAction)adjustAction:(id)sender
{
	[_adjustStepper setIntValue:[sender intValue]];
}

- (IBAction)adjustStepperAction:(id)sender
{
	[_adjustData setIntValue:[sender intValue]];
}

- (IBAction)unitAction:(id)sender
{
	UT_Dimension mu = (UT_Dimension) [[_unitPopup selectedItem] tag];

	float top, bottom, left, right, header, footer;
	
	top    = [_topMargin floatValue];
	bottom = [_bottomMargin floatValue];
	left   = [_leftMargin floatValue];
	right  = [_rightMargin floatValue];
	header = [_headerMargin floatValue];
	footer = [_footerMargin floatValue];
	
	UT_convertDimensions (top,    _last_margin_unit, mu);
	UT_convertDimensions (bottom, _last_margin_unit, mu);
	UT_convertDimensions (left,   _last_margin_unit, mu);
	UT_convertDimensions (right,  _last_margin_unit, mu);
	UT_convertDimensions (header, _last_margin_unit, mu);
	UT_convertDimensions (footer, _last_margin_unit, mu);
	
	_last_margin_unit = mu;
	
	[_topMargin setFloatValue:top];
	[_bottomMargin setFloatValue:bottom];
	[_leftMargin setFloatValue:left];
	[_rightMargin setFloatValue:right];
	[_headerMargin setFloatValue:header];
	[_footerMargin setFloatValue:footer];
}

@end
