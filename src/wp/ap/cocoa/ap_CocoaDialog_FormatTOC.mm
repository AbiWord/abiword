/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
#include "ap_Dialog_FormatFootnotes.h"
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
	if (m_dlg)
		[m_dlg sync];
}

void AP_CocoaDialog_FormatTOC::setSensitivity(bool bSensitive)
{
	[m_dlg enableApply:(bSensitive?YES:NO)];
}

void AP_CocoaDialog_FormatTOC::destroy(void)
{
	if (m_dlg)
		{
			[m_dlg release];
			m_dlg = nil;
		}
	finalize();
}

void AP_CocoaDialog_FormatTOC::activate(void)
{
	if (m_dlg)
		{
			[[m_dlg window] orderFront:m_dlg];
			[m_dlg sync];
		}
}

void AP_CocoaDialog_FormatTOC::notifyActiveFrame(XAP_Frame * pFrame)
{
	// 
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

void AP_CocoaDialog_FormatTOC::_populateWindowData(void)
{
	// 
}

@implementation AP_CocoaDialog_FormatTOC_Controller

- (id)initFromNib
{
	if (self = [super initWithWindowNibName:@"ap_CocoaDialog_FormatTOC"])
		{
			// 
		}
	return self;
}

- (void)dealloc
{
	// 
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = static_cast<AP_CocoaDialog_FormatTOC *>(owner);
}

- (void)discardXAP
{
	_xap = NULL; 
}

- (void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		LocalizeControl([self window],			pSS, AP_STRING_ID_DLG_FormatTOC_Title);

		LocalizeControl(_applyBtn,				pSS, XAP_STRING_ID_DLG_Apply);

		LocalizeControl([_tabView tabViewItemAtIndex:0], pSS, AP_STRING_ID_DLG_FormatTOC_General);
		LocalizeControl([_tabView tabViewItemAtIndex:1], pSS, AP_STRING_ID_DLG_FormatTOC_LayoutDetails);
		
		LocalizeControl(_hasHeadingBtn,			pSS, AP_STRING_ID_DLG_FormatTOC_HasHeading);
		LocalizeControl(_headingTextLabel,		pSS, AP_STRING_ID_DLG_FormatTOC_HeadingText);
		LocalizeControl(_headingStyleLabel,		pSS, AP_STRING_ID_DLG_FormatTOC_HeadingStyle);
		LocalizeControl(_hasLabelBtn,			pSS, AP_STRING_ID_DLG_FormatTOC_HasLabel);
		LocalizeControl(_fillStyleLabel,		pSS, AP_STRING_ID_DLG_FormatTOC_FillStyle);
		LocalizeControl(_displayStyleLabel,		pSS, AP_STRING_ID_DLG_FormatTOC_DispStyle);
		LocalizeControl(_startAtLabel,			pSS, AP_STRING_ID_DLG_FormatTOC_StartAt);
		LocalizeControl(_textBeforeLabel,		pSS, AP_STRING_ID_DLG_FormatTOC_TextBefore);
		LocalizeControl(_numberingTypeLabel,	pSS, AP_STRING_ID_DLG_FormatTOC_NumberingType);
		LocalizeControl(_inheritLabelBtn,		pSS, AP_STRING_ID_DLG_FormatTOC_InheritLabel);
		LocalizeControl(_tabLeadersLabel,		pSS, AP_STRING_ID_DLG_FormatTOC_TabLeader);
		LocalizeControl(_pageNumberingLabel,	pSS, AP_STRING_ID_DLG_FormatTOC_PageNumbering);
		LocalizeControl(_indentLabel,			pSS, AP_STRING_ID_DLG_FormatTOC_Indent);
		LocalizeControl(_defineMainLabel,		pSS, AP_STRING_ID_DLG_FormatTOC_LevelDefs);
		LocalizeControl(_labelDefinitionsLabel,	pSS, AP_STRING_ID_DLG_FormatTOC_DetailsTop);
		LocalizeControl(_tabsPageNoLabel,		pSS, AP_STRING_ID_DLG_FormatTOC_DetailsTabPage);

		LocalizeControl(_headingStyleBtn,		pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);
		LocalizeControl(_fillStyleBtn,			pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);
		LocalizeControl(_displayStyleBtn,		pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);

		[self createLevelItems:  _mainLevelPopup];
		[self createLevelItems:_layoutLevelPopup];

		[self createNumberingItems:_numberingTypeData];
		[self createNumberingItems:_pageNumberingData];

		const UT_GenericVector<const XML_Char *> * vecTABLeaders = _xap->getVecTABLeadersLabel();

		UT_sint32 nTypes = vecTABLeaders->getItemCount();

		[_tabLeadersData removeAllItems];

		for (UT_sint32 n = 0; n < nTypes; n++)
			{
				const XML_Char * tabLeader = vecTABLeaders->getNthItem(n);

				[_tabLeadersData addItemWithTitle:[NSString stringWithUTF8String:((const char *) tabLeader)]];
			}

		[self sync];
	}
}

- (void)createLevelItems:(NSPopUpButton *)popup
{
	if (_xap) {
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		[popup removeAllItems];

		AppendLocalizedMenuItem(popup, pSS, AP_STRING_ID_DLG_FormatTOC_Level1, 1);
		AppendLocalizedMenuItem(popup, pSS, AP_STRING_ID_DLG_FormatTOC_Level2, 2);
		AppendLocalizedMenuItem(popup, pSS, AP_STRING_ID_DLG_FormatTOC_Level3, 3);
		AppendLocalizedMenuItem(popup, pSS, AP_STRING_ID_DLG_FormatTOC_Level4, 4);
	}
}

- (void)createNumberingItems:(NSPopUpButton *)popup
{
	const UT_GenericVector<const XML_Char *> * vecTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();

	UT_sint32 nTypes = vecTypeList->getItemCount();

	[popup removeAllItems];

	for (UT_sint32 n = 0; n < nTypes; n++)
		{
			const XML_Char * typeList = vecTypeList->getNthItem(n);

			[popup addItemWithTitle:[NSString stringWithUTF8String:((const char *) typeList)]];
		}
}

- (void)sync
{
	if (_xap) {
		UT_UTF8String sVal;

		/* Heading
		 */
		sVal = _xap->getTOCPropVal("toc-has-heading");
		BOOL bHasHeading = (sVal == "1") ? YES : NO;
		[_hasHeadingBtn setState:(bHasHeading ? NSOnState : NSOffState)];

		[_headingTextData setEnabled:bHasHeading];
		[_headingStyleBtn setEnabled:bHasHeading];

		sVal = _xap->getTOCPropVal("toc-heading");
		[ _headingTextData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		sVal = _xap->getTOCPropVal("toc-heading-style");
		[_headingStyleData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		/* Main Properties
		 */
		sVal = _xap->getTOCPropVal("toc-has-label", _xap->getMainLevel());
		BOOL bHasLabel = (sVal == "1") ? YES : NO;
		[_hasLabelBtn setState:(bHasLabel ? NSOnState : NSOffState)];

		[   _fillStyleData setEnabled:bHasLabel];
		[_displayStyleData setEnabled:bHasLabel];

		sVal = _xap->getTOCPropVal("toc-source-style", _xap->getMainLevel());
		[   _fillStyleData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		sVal = _xap->getTOCPropVal("toc-dest-style", _xap->getMainLevel());
		[_displayStyleData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		/* Label Definitions
		 */
		sVal = _xap->getTOCPropVal("toc-label-start", _xap->getDetailsLevel());
		[     _startAtData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		sVal = _xap->getTOCPropVal("toc-label-before", _xap->getDetailsLevel());
		[  _textBeforeData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		// TODO: numbering type

		sVal = _xap->getTOCPropVal("toc-label-after", _xap->getDetailsLevel());
		[   _textAfterData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		sVal = _xap->getTOCPropVal("toc-label-inherits", _xap->getDetailsLevel());
		BOOL bInherits = (sVal == "1") ? YES : NO;
		[_inheritLabelBtn setState:(bInherits ? NSOnState : NSOffState)];

		/* Tabs & Page Nos
		 */
		// TODO: Tab leader

		// TODO: Page No.

		sVal = _xap->getTOCPropVal("toc-indent", _xap->getDetailsLevel());
		[      _indentData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];
	}
}

- (IBAction)headingStyleAction:(id)sender
{
	// TODO
}

- (IBAction)fillStyleAction:(id)sender
{
	// TODO
}

- (IBAction)displayStyleAction:(id)sender
{
	// TODO
}

- (IBAction)startAtStepperAction:(id)sender
{
	_xap->incrementIndent(_xap->getDetailsLevel(), ([_startAtStepper intValue] > 1));
	[_startAtStepper setIntValue:1];

	UT_UTF8String sVal = _xap->getTOCPropVal("toc-label-start", _xap->getDetailsLevel());
	[_startAtData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];
}

- (IBAction)startAtAction:(id)sender
{
	// TODO
}

- (IBAction)indentStepperAction:(id)sender
{
	_xap->incrementIndent(_xap->getDetailsLevel(), ([_indentStepper intValue] > 1));
	[_indentStepper setIntValue:1];

	UT_UTF8String sVal = _xap->getTOCPropVal("toc-indent", _xap->getDetailsLevel());
	[_indentData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];
}

- (IBAction)indentAction:(id)sender
{
	// TODO
}

- (IBAction)mainLevelAction:(id)sender
{
	_xap->setMainLevel([[sender selectedItem] tag]);
	[self sync];
}

- (IBAction)detailLevelAction:(id)sender
{
	_xap->setDetailsLevel([[sender selectedItem] tag]);
	[self sync];
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

- (void)windowWillClose:(NSNotification *)aNotification
{
	if (_xap)
		_xap->destroy();
}

@end
