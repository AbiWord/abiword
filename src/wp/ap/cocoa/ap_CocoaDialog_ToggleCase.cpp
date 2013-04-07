/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
 * Copyright (C) 2003 Mark Pazolli
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
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Cocoa dialogs,
// like centering them, measuring them, etc.
#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_ToggleCase.h"
#include "ap_CocoaDialog_ToggleCase.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_ToggleCase::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_ToggleCase * p = new AP_CocoaDialog_ToggleCase(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_ToggleCase::AP_CocoaDialog_ToggleCase(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_ToggleCase(pDlgFactory,dlgid)
{
}

AP_CocoaDialog_ToggleCase::~AP_CocoaDialog_ToggleCase(void)
{
}

void AP_CocoaDialog_ToggleCase::runModal(XAP_Frame * /*pFrame*/)
{
	NSWindow* window;

	m_dlg = [[AP_CocoaDialog_ToggleCaseController alloc] initFromNib];
	[m_dlg setXAPOwner:this];
	window = [m_dlg window];
	
	[NSApp runModalForWindow:window];
	
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

@implementation AP_CocoaDialog_ToggleCaseController

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_ToggleCase"];
}

- (void)awakeFromNib
{

	switch (_xap->getCase()) {
	case CASE_SENTENCE:
		[_caseMatrix selectCellWithTag:0];
		break;
	case CASE_LOWER:
		[_caseMatrix selectCellWithTag:1];
		break;
	case CASE_UPPER:
		[_caseMatrix selectCellWithTag:2];
		break;
	case CASE_FIRST_CAPITAL:
		[_caseMatrix selectCellWithTag:3];
		break;
	case CASE_TOGGLE:
		[_caseMatrix selectCellWithTag:4];
		break;
	default:
		[_caseMatrix selectCellWithTag:0];
		break;
	}
}

- (void)dealloc
{
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_ToggleCase*>(owner);
	UT_ASSERT(_xap);
}

- (void)discardXAP
{
	_xap = NULL;
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	LocalizeControl([self window],	pSS, AP_STRING_ID_DLG_ToggleCase_Title);

	LocalizeControl (_okBtn,		pSS, XAP_STRING_ID_DLG_OK);
	LocalizeControl (_cancelBtn,	pSS, XAP_STRING_ID_DLG_Cancel);

	LocalizeControl(_sentenceBtn,	pSS, AP_STRING_ID_DLG_ToggleCase_SentenceCase);
	LocalizeControl(_lowerBtn,		pSS, AP_STRING_ID_DLG_ToggleCase_LowerCase);
	LocalizeControl(_upperBtn,		pSS, AP_STRING_ID_DLG_ToggleCase_UpperCase);
//	LocalizeControl(_titleBtn,		pSS, AP_STRING_ID_DLG_ToggleCase_TitleCase);
	LocalizeControl(_initialBtn,	pSS, AP_STRING_ID_DLG_ToggleCase_FirstUpperCase);
	LocalizeControl(_toggleBtn,		pSS, AP_STRING_ID_DLG_ToggleCase_ToggleCase);
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	switch ([[_caseMatrix selectedCell] tag]) {
	case 0:
		_xap->setCase(CASE_SENTENCE);
		break;
	case 1:
		_xap->setCase(CASE_LOWER);
		break;
	case 2:
		_xap->setCase(CASE_UPPER);
		break;
	case 3:
		_xap->setCase(CASE_FIRST_CAPITAL);
		break;
	case 4:
		_xap->setCase(CASE_TOGGLE);
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	_xap->setAnswer(_xap->a_OK);
    [NSApp stopModal];
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->setAnswer(_xap->a_CANCEL);
    [NSApp stopModal];
}

@end
