/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2001-2002, 2004, 2009 Hubert Figuiere
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
	[m_dlg setSensitivity:(bSensitive ? YES : NO)];
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

void AP_CocoaDialog_FormatTOC::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
	// 
}

void AP_CocoaDialog_FormatTOC::runModeless(XAP_Frame * /*pFrame*/)
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
	if (self = [super initWithWindowNibName:@"ap_CocoaDialog_FormatTOC"]) {

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

		const UT_GenericVector<const gchar *> * vecTABLeaders = _xap->getVecTABLeadersLabel();

		UT_sint32 nTypes = vecTABLeaders->getItemCount();

		[_tabLeadersData removeAllItems];

		for (UT_sint32 n = 0; n < nTypes; n++)
			{
				const gchar * tabLeader = vecTABLeaders->getNthItem(n);

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
	const FootnoteTypeDesc* vecTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();

	[popup removeAllItems];
	const FootnoteTypeDesc *current;
	for (current = vecTypeList; current->n != _FOOTNOTE_TYPE_INVALID ; current++)
	{
		[popup addItemWithTitle:[NSString stringWithUTF8String:current->label]];
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
		[    _hasHeadingBtn setState:(bHasHeading ? NSOnState : NSOffState)];

		[_headingTextData setEnabled:bHasHeading];
		[_headingStyleBtn setEnabled:bHasHeading];

		sVal = _xap->getTOCPropVal("toc-heading");
		[  _headingTextData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		sVal = _xap->getTOCPropVal("toc-heading-style");
		[ _headingStyleData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];
	}
	[self   syncMainLevelSettings];
	[self syncDetailLevelSettings];
}

- (void)syncMainLevelSettings
{
	if (_xap) {
		UT_UTF8String sVal;

		/* Main Properties
		 */
		sVal = _xap->getTOCPropVal("toc-has-label", _xap->getMainLevel());
		BOOL bHasLabel = (sVal == "1") ? YES : NO;
		[      _hasLabelBtn setState:(bHasLabel ? NSOnState : NSOffState)];

		[    _fillStyleData setEnabled:bHasLabel];
		[ _displayStyleData setEnabled:bHasLabel];

		sVal = _xap->getTOCPropVal("toc-source-style", _xap->getMainLevel());
		[    _fillStyleData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		sVal = _xap->getTOCPropVal("toc-dest-style", _xap->getMainLevel());
		[ _displayStyleData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];
	}
}

- (void)syncDetailLevelSettings
{
	if (_xap) {
		FV_View * pView = 0;

		if (XAP_Frame * pFrame = _xap->getActiveFrame())
			{
				pView = static_cast<FV_View *>(pFrame->getCurrentView());
			}
		if (!pView)
			{
				[self setSensitivity:NO];
				return;
			}

		UT_UTF8String sVal;

		/* Label Definitions
		 */
		sVal = _xap->getTOCPropVal("toc-label-start", _xap->getDetailsLevel());
		[      _startAtData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		sVal = _xap->getTOCPropVal("toc-label-before", _xap->getDetailsLevel());
		[   _textBeforeData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		sVal = _xap->getTOCPropVal("toc-label-type", _xap->getDetailsLevel());
		[_numberingTypeData selectItemAtIndex:((int) (pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str())))];

		sVal = _xap->getTOCPropVal("toc-label-after", _xap->getDetailsLevel());
		[    _textAfterData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];

		sVal = _xap->getTOCPropVal("toc-label-inherits", _xap->getDetailsLevel());
		BOOL bInherits = (sVal == "1") ? YES : NO;
		[ _inheritLabelBtn setState:(bInherits ? NSOnState : NSOffState)];

		/* Tabs & Page Nos
		 */
		sVal = _xap->getTOCPropVal("toc-tab-leader", _xap->getDetailsLevel());

		int iTabLeader = 1;

		if      (g_ascii_strcasecmp(sVal.utf8_str(), "none"     ) == 0)
			iTabLeader = 0;
		else if (g_ascii_strcasecmp(sVal.utf8_str(), "dot"      ) == 0)
			iTabLeader = 1;
		else if (g_ascii_strcasecmp(sVal.utf8_str(), "hyphen"   ) == 0)
			iTabLeader = 2;
		else if (g_ascii_strcasecmp(sVal.utf8_str(), "underline") == 0)
			iTabLeader = 3;

		[   _tabLeadersData selectItemAtIndex:iTabLeader];

		sVal = _xap->getTOCPropVal("toc-page-type", _xap->getDetailsLevel());
		[_pageNumberingData selectItemAtIndex:((int) (pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str())))];

		sVal = _xap->getTOCPropVal("toc-indent", _xap->getDetailsLevel());
		[      _indentData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];
	}
}

- (IBAction)headingStyleAction:(id)sender
{
	UT_UNUSED(sender);
	if (_xap) {
		UT_UTF8String sTOCProp = "toc-heading-style";

		UT_UTF8String sVal = _xap->getNewStyle(sTOCProp);

		[[self window] makeKeyAndOrderFront:self];

		[ _headingStyleData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];
	}
}

- (IBAction)fillStyleAction:(id)sender
{
	UT_UNUSED(sender);
	if (_xap) {
		UT_UTF8String sLevelNo = UT_UTF8String_sprintf("%d", _xap->getMainLevel());
		UT_UTF8String sTOCProp = "toc-source-style";
		sTOCProp += sLevelNo;

		UT_UTF8String sVal = _xap->getNewStyle(sTOCProp);

		[[self window] makeKeyAndOrderFront:self];

		[    _fillStyleData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];
	}
}

- (IBAction)displayStyleAction:(id)sender
{
	UT_UNUSED(sender);
	if (_xap) {
		UT_UTF8String sLevelNo = UT_UTF8String_sprintf("%d", _xap->getMainLevel());
		UT_UTF8String sTOCProp = "toc-dest-style";
		sTOCProp += sLevelNo;

		UT_UTF8String sVal = _xap->getNewStyle(sTOCProp);

		[[self window] makeKeyAndOrderFront:self];

		[ _displayStyleData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];
	}
}

- (IBAction)startAtStepperAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->incrementStartAt(_xap->getDetailsLevel(), ([_startAtStepper intValue] > 1));
	[_startAtStepper setIntValue:1];

	UT_UTF8String sVal = _xap->getTOCPropVal("toc-label-start", _xap->getDetailsLevel());
	[_startAtData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];
}

- (IBAction)startAtAction:(id)sender
{
	UT_UNUSED(sender);
	// TODO ??
}

- (IBAction)indentStepperAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->incrementIndent(_xap->getDetailsLevel(), ([_indentStepper intValue] > 1));
	[_indentStepper setIntValue:1];

	UT_UTF8String sVal = _xap->getTOCPropVal("toc-indent", _xap->getDetailsLevel());
	[_indentData setStringValue:[NSString stringWithUTF8String:(sVal.utf8_str())]];
}

- (IBAction)indentAction:(id)sender
{
	UT_UNUSED(sender);
	// TODO ??
}

- (IBAction)mainLevelAction:(id)sender
{
	[self saveMainLevelSettings];

	_xap->setMainLevel([[sender selectedItem] tag]);

	[self syncMainLevelSettings];
}

- (IBAction)detailLevelAction:(id)sender
{
	[self saveDetailLevelSettings];

	_xap->setDetailsLevel([[sender selectedItem] tag]);

	[self syncDetailLevelSettings];
}

- (void)saveMainLevelSettings
{
	if (_xap) {
		UT_UTF8String sLevelNo = UT_UTF8String_sprintf("%d", _xap->getMainLevel());
		UT_UTF8String sTOCProp;
		UT_UTF8String sVal;

		sTOCProp  = "toc-has-label";
		sTOCProp += sLevelNo;
		if ([_hasLabelBtn state] == NSOnState)
			sVal = "1";
		else
			sVal = "0";
		_xap->setTOCProperty(sTOCProp, sVal);

		sTOCProp  = "toc-source-style";
		sTOCProp += sLevelNo;
		sVal = [[    _fillStyleData stringValue] UTF8String];
		_xap->setTOCProperty(sTOCProp, sVal);

		sTOCProp  = "toc-dest-style";
		sTOCProp += sLevelNo;
		sVal = [[ _displayStyleData stringValue] UTF8String];
		_xap->setTOCProperty(sTOCProp, sVal);
	}
}

- (void)saveDetailLevelSettings
{
	if (_xap) {
		const FootnoteTypeDesc * vecPropList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();

		UT_UTF8String sLevelNo = UT_UTF8String_sprintf("%d", _xap->getDetailsLevel());
		UT_UTF8String sTOCProp;
		UT_UTF8String sVal;

		/* Label Definitions
		 */
		// sTOCProp  = "toc-label-start";
		// sTOCProp += sLevelNo;

		sTOCProp  = "toc-label-before";
		sTOCProp += sLevelNo;
		sVal = [[   _textBeforeData stringValue] UTF8String];
		_xap->setTOCProperty(sTOCProp, sVal);

		sTOCProp  = "toc-label-type";
		sTOCProp += sLevelNo;
		sVal = vecPropList[ [_numberingTypeData indexOfSelectedItem] ].prop;
		_xap->setTOCProperty(sTOCProp, sVal);

		sTOCProp  = "toc-label-after";
		sTOCProp += sLevelNo;
		sVal = [[    _textAfterData stringValue] UTF8String];
		_xap->setTOCProperty(sTOCProp, sVal);

		sTOCProp  = "toc-label-inherits";
		sTOCProp += sLevelNo;
		if ([ _inheritLabelBtn state] == NSOnState)
			sVal = "1";
		else
			sVal = "0";
		_xap->setTOCProperty(sTOCProp, sVal);

		/* Tabs & Page Nos
		 */
		sTOCProp  = "toc-tab-leader";
		sTOCProp += sLevelNo;
		switch ([_tabLeadersData indexOfSelectedItem])
			{
			case 0:
				sVal = "none";
				break;
			default:
			case 1:
				sVal = "dot";
				break;
			case 2:
				sVal = "hyphen";
				break;
			case 3:
				sVal = "underline";
				break;
			}
		_xap->setTOCProperty(sTOCProp, sVal);

		sTOCProp  = "toc-page-type";
		sTOCProp += sLevelNo;
		sVal = vecPropList[ [_pageNumberingData indexOfSelectedItem]].prop;
		_xap->setTOCProperty(sTOCProp, sVal);

		// sTOCProp  = "toc-indent";
		// sTOCProp += sLevelNo;
	}
}

- (IBAction)applyAction:(id)sender
{
	UT_UNUSED(sender);
	[self saveMainLevelSettings];
	[self saveDetailLevelSettings];

	UT_UTF8String sTOCProp;
	UT_UTF8String sVal;

	sTOCProp = "toc-has-heading";
	if ([_hasHeadingBtn state] == NSOnState)
		sVal = "1";
	else
		sVal = "0";
	_xap->setTOCProperty(sTOCProp, sVal);

	sTOCProp = "toc-heading";
	sVal = [[ _headingTextData stringValue] UTF8String];
	_xap->setTOCProperty(sTOCProp, sVal);
	
	sTOCProp = "toc-heading-style";
	sVal = [[_headingStyleData stringValue] UTF8String];
	_xap->setTOCProperty(sTOCProp, sVal);

	_xap->Apply();
}

- (void)setSensitivity:(BOOL)enable
{
	[   _mainLevelPopup setEnabled:enable];
	[ _layoutLevelPopup setEnabled:enable];

	[  _headingTextData setEnabled:enable];
	[  _headingStyleBtn setEnabled:enable];

	[    _fillStyleData setEnabled:enable];
	[ _displayStyleData setEnabled:enable];

	[  _headingStyleBtn setEnabled:enable];
	[     _fillStyleBtn setEnabled:enable];
	[  _displayStyleBtn setEnabled:enable];

	[    _hasHeadingBtn setEnabled:enable];
	[      _hasLabelBtn setEnabled:enable];

	[      _startAtData setEnabled:enable];
	[       _indentData setEnabled:enable];

	[   _startAtStepper setEnabled:enable];
	[    _indentStepper setEnabled:enable];

	[   _textBeforeData setEnabled:enable];
	[    _textAfterData setEnabled:enable];

	[  _inheritLabelBtn setEnabled:enable];

	[_numberingTypeData setEnabled:enable];
	[_pageNumberingData setEnabled:enable];

	[   _tabLeadersData setEnabled:enable];

	[_applyBtn setEnabled:enable];
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	if (_xap)
		_xap->destroy();
}

@end
