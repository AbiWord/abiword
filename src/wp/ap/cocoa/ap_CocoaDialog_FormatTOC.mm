/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2001-2002, 2004 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#import "xap_CocoaDialog_Utilities.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_FormatTOC.h"
#include "ap_CocoaDialog_FormatTOC.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_FormatTOC::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_FormatTOC * p = new AP_CocoaDialog_FormatTOC(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_FormatTOC::AP_CocoaDialog_FormatTOC(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_FormatTOC(pDlgFactory,dlgid)
{
}

AP_CocoaDialog_FormatTOC::~AP_CocoaDialog_FormatTOC(void)
{
}


void AP_CocoaDialog_FormatTOC::setTOCPropsInGUI(void)
{
	UT_ASSERT(0);
}

void AP_CocoaDialog_FormatTOC::setSensitivity(bool bSensitive)
{
	[m_dlg enableApply:(bSensitive?YES:NO)];
}

void AP_CocoaDialog_FormatTOC::destroy(void)
{
	UT_ASSERT(m_dlg);
	[m_dlg release];
	m_dlg = nil;
	finalize();
}

void AP_CocoaDialog_FormatTOC::activate(void)
{
	UT_ASSERT(m_dlg);
	[[m_dlg window] orderFront:m_dlg];	
}

void AP_CocoaDialog_FormatTOC::notifyActiveFrame(XAP_Frame *pFrame)
{
}

void AP_CocoaDialog_FormatTOC::runModeless(XAP_Frame * pFrame)
{
	m_dlg = [[AP_CocoaDialog_FormatTOC_Controller alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	NSWindow* window = [m_dlg window];

	_populateWindowData();
	[window orderFront:m_dlg];	
	startUpdater();
}

void AP_CocoaDialog_FormatTOC::_createLevelItems(NSPopUpButton* popup)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet ();
	[popup removeAllItems];

	AppendLocalizedMenuItem(popup, pSS, AP_STRING_ID_DLG_FormatTOC_Level1, 1);
	AppendLocalizedMenuItem(popup, pSS, AP_STRING_ID_DLG_FormatTOC_Level2, 2);
	AppendLocalizedMenuItem(popup, pSS, AP_STRING_ID_DLG_FormatTOC_Level3, 3);
	AppendLocalizedMenuItem(popup, pSS, AP_STRING_ID_DLG_FormatTOC_Level4, 4);
}


void AP_CocoaDialog_FormatTOC::_populateWindowData(void)
{
}

@implementation AP_CocoaDialog_FormatTOC_Controller

- (id)initFromNib
{
	self = [super initWithWindowNibName:@"ap_CocoaDialog_FormatTOC"];
	return self;
}

- (void)discardXAP
{
	_xap = NULL; 
}

- (void)dealloc
{
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_FormatTOC*>(owner);
}

- (void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_FormatTOC_Title);
		LocalizeControl(_applyBtn, pSS, XAP_STRING_ID_DLG_Apply);

		LocalizeControl([_tabView tabViewItemAtIndex:0], pSS, AP_STRING_ID_DLG_FormatTOC_General);
		LocalizeControl([_tabView tabViewItemAtIndex:1], pSS, AP_STRING_ID_DLG_FormatTOC_LayoutDetails);
		
		LocalizeControl(_hasHeadingBtn, pSS, AP_STRING_ID_DLG_FormatTOC_HasHeading);
		LocalizeControl(_headingTextLabel, pSS, AP_STRING_ID_DLG_FormatTOC_HeadingText);
		LocalizeControl(_headingStyleLabel, pSS, AP_STRING_ID_DLG_FormatTOC_HeadingStyle);
		LocalizeControl(_mainPropBox, pSS, AP_STRING_ID_DLG_FormatTOC_LevelDefs);
		LocalizeControl(_hasLabelBtn, pSS, AP_STRING_ID_DLG_FormatTOC_HasLabel);
		LocalizeControl(_fillStyleLabel, pSS, AP_STRING_ID_DLG_FormatTOC_FillStyle);
		LocalizeControl(_displayStyleLabel, pSS, AP_STRING_ID_DLG_FormatTOC_DispStyle);
		LocalizeControl(_labelDefBox, pSS, AP_STRING_ID_DLG_FormatTOC_DetailsTop);
		LocalizeControl(_startAtLabel, pSS, AP_STRING_ID_DLG_FormatTOC_StartAt);
		LocalizeControl(_textBeforeLabel, pSS, AP_STRING_ID_DLG_FormatTOC_TextBefore);
		LocalizeControl(_numberingTypeLabel, pSS, AP_STRING_ID_DLG_FormatTOC_NumberingType);
		LocalizeControl(_inheritLabelBtn, pSS, AP_STRING_ID_DLG_FormatTOC_InheritLabel);
		LocalizeControl(_tabsAndPageNumbBox, pSS, AP_STRING_ID_DLG_FormatTOC_DetailsTabPage);
		LocalizeControl(_tabLeadersLabel, pSS, AP_STRING_ID_DLG_FormatTOC_TabLeader);
		LocalizeControl(_pageNumberingLabel, pSS, AP_STRING_ID_DLG_FormatTOC_PageNumbering);
		LocalizeControl(_indentLabel, pSS, AP_STRING_ID_DLG_FormatTOC_Indent);

//	_createLabelTypeItems();
//	_createTABTypeItems();
		_xap->_createLevelItems(_mainLevelPopup);
		_xap->_createLevelItems(_layoutLevelPopup);

	}
}

- (IBAction)mainLevelAction:(id)sender
{
	_xap->setMainLevel([[sender selectedItem] tag]);
}

- (IBAction)detailLevelAction:(id)sender
{
	_xap->setDetailsLevel([[sender selectedItem] tag]);
}


- (IBAction)applyAction:(id)sender
{
	UT_DEBUGMSG(("Doing apply \n"));

// Heading Text

	UT_UTF8String sVal = [[_headingTextData stringValue] UTF8String];
	_xap->setTOCProperty("toc-heading",sVal.utf8_str());

// Text before and after

	UT_String sNum =  UT_String_sprintf("%d",_xap->getDetailsLevel());

	sVal = [[_textAfterData stringValue] UTF8String];
	UT_UTF8String sProp = "toc-label-after";
	sProp += sNum.c_str();
	_xap->setTOCProperty(sProp, sVal);

	sVal = [[_textBeforeData stringValue] UTF8String];
	sProp = "toc-label-before";
	sProp += sNum.c_str();
	_xap->setTOCProperty(sProp, sVal);
	
	_xap->Apply();
}

- (void)enableApply:(BOOL)enable
{
	[_applyBtn setEnabled:enable];
}

@end

