/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Hubert Figuiere.
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
	: AP_Dialog_Break(pDlgFactory,dlgid)
{
}

AP_CocoaDialog_Break::~AP_CocoaDialog_Break(void)
{
}

/*****************************************************************/

void AP_CocoaDialog_Break::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	m_dlg = [AP_CocoaDialog_BreakController loadFromNib];	// autoreleased
	[m_dlg setXAPOwner:this];
	NSWindow *win = [m_dlg window];		// force the window to be loaded.

	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	AP_CocoaFrame * pCocoaFrame = static_cast<AP_CocoaFrame *>(pFrame);
	UT_ASSERT(pCocoaFrame);
	
	[NSApp runModalForWindow:win];

	_storeWindowData();
}


void AP_CocoaDialog_Break::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.
}

void AP_CocoaDialog_Break::_storeWindowData(void)
{
	m_break = _getActiveRadioItem();
}


AP_Dialog_Break::breakType AP_CocoaDialog_Break::_getActiveRadioItem(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
	return AP_Dialog_Break::b_PAGE;
}


@implementation AP_CocoaDialog_BreakController

+ (AP_CocoaDialog_BreakController *)loadFromNib
{
	AP_CocoaDialog_BreakController * dlg = [[AP_CocoaDialog_BreakController alloc] initWithWindowNibName:@"ap_CocoaDialog_Break"];
	return [dlg autorelease];
}


- (void)setXAPOwner:(AP_CocoaDialog_Break *)owner
{
	UT_ASSERT (owner);
	m_xap = owner;
}

- (void)windowDidLoad
{
#if 0
	// translate the whole dialog.
	XAP_CocoaFrame *pFrame = m_xap->_getFrame ();
	// we get all our strings from the application string set
	const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
	[[self window] setTitle:[NSString stringWithCString:pSS->getValue(AP_STRING_ID_DLG_Break_BreakTitle)]];
	[m_insertGrp setTitle:[NSString stringWithCString:pSS->getValue(AP_STRING_ID_DLG_Break_Insert)]];
#endif
}

- (IBAction)cancelAction:(id)sender
{
	UT_ASSERT (m_xap);
	m_xap->_setAnswer(AP_Dialog_Break::a_CANCEL);
	[NSApp stopModal];
}

- (IBAction)okAction:(id)sender
{
	UT_ASSERT (m_xap);
	m_xap->_setAnswer(AP_Dialog_Break::a_OK);
	[NSApp stopModal];
}

- (IBAction)sectionBrkAction:(id)sender;
{
}

- (IBAction)insertAction:(id)sender;
{
	if ([m_sectionBrkBtn compare:sender] == NSOrderedSame) {
		// enable all the section break buttons
	}
	else {
		// disable all the section break buttons
	}	
}


@end

