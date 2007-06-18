/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002-2003 Hubert Figuiere.
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

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_CocoaDialog_Break.h"
#include "ap_CocoaFrame.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_Break::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_Break * p = new AP_CocoaDialog_Break(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_Break::AP_CocoaDialog_Break(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_Break(pDlgFactory,dlgid),
	  m_dlg(nil),
	  m_breakType(AP_Dialog_Break::b_PAGE)
{
}

AP_CocoaDialog_Break::~AP_CocoaDialog_Break(void)
{
}

/*****************************************************************/

void AP_CocoaDialog_Break::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	m_dlg = [[AP_CocoaDialog_BreakController alloc] initFromNib];	// autoreleased
	[m_dlg setXAPOwner:this];
	NSWindow *win = [m_dlg window];		// force the window to be loaded.

	// Populate the window's data items
	_populateWindowData();
	
	[NSApp runModalForWindow:win];

	_storeWindowData();
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}


void AP_CocoaDialog_Break::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.
}

void AP_CocoaDialog_Break::_storeWindowData(void)
{
	m_break = m_breakType;
}


@implementation AP_CocoaDialog_BreakController

- (id)initFromNib
{
	if (self = [super initWithWindowNibName:@"ap_CocoaDialog_Break"])
	{
		m_xap = 0;
	}
	return self;
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	m_xap = dynamic_cast<AP_CocoaDialog_Break*>(owner);
	UT_ASSERT(m_xap);
}

- (void)discardXAP
{
	m_xap = nil;
}

- (void)windowDidLoad
{
	// we get all our strings from the application string set
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Break_BreakTitle);
	LocalizeControl(m_insertGrp, pSS, AP_STRING_ID_DLG_Break_Insert);
	LocalizeControl(m_pgBrkBtn, pSS, AP_STRING_ID_DLG_Break_PageBreak);
	LocalizeControl(m_sectionBrkBtn, pSS, AP_STRING_ID_DLG_Break_SectionBreaks);
	LocalizeControl(m_nxtPgBtn, pSS, AP_STRING_ID_DLG_Break_NextPage);
	LocalizeControl(m_continuousBtn, pSS, AP_STRING_ID_DLG_Break_Continuous);
	LocalizeControl(m_evenPgBtn, pSS, AP_STRING_ID_DLG_Break_EvenPage);
	LocalizeControl(m_oddPgBtn, pSS, AP_STRING_ID_DLG_Break_OddPage);
	
	[self _updateButtonsState];
}

- (IBAction)cancelAction:(id)sender
{
	UT_ASSERT(m_xap);
	m_xap->_setAnswer(AP_Dialog_Break::a_CANCEL);
	[NSApp stopModal];
}

- (IBAction)okAction:(id)sender
{
	UT_ASSERT(m_xap);

	AP_Dialog_Break::breakType type = AP_Dialog_Break::b_PAGE;

	switch ([m_insertRadioBtns selectedRow]) {
	case 0:
		type = AP_Dialog_Break::b_PAGE;
		break;
	case 1:
		type = AP_Dialog_Break::b_COLUMN;
		break;
	case 2:
		{
			int y = [m_sectionBreakBtns selectedRow];

			if ([m_sectionBreakBtns selectedColumn]) {
				if (y == 0) {
					type = AP_Dialog_Break::b_EVENPAGE;
				}
				else {
					type = AP_Dialog_Break::b_ODDPAGE;
				}
			}
			else {
				if (y == 0) {
					type = AP_Dialog_Break::b_NEXTPAGE;
				}
				else {
					type = AP_Dialog_Break::b_CONTINUOUS;
				}
			}
			break;
		}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	m_xap->_setBreakType(type);
	m_xap->_setAnswer(AP_Dialog_Break::a_OK);

	[NSApp stopModal];
}

- (IBAction)insertAction:(id)sender;
{
	[self _updateButtonsState];
}

- (void)_updateButtonsState
{
	if ([m_insertRadioBtns selectedRow] == 2) {
		[m_sectionBreakBtns setEnabled:YES];	
	}
	else {
		[m_sectionBreakBtns setEnabled:NO];
	}
}

@end

