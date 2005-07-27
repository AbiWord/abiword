/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
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

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_WordCount.h"
#include "ap_CocoaDialog_WordCount.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_WordCount::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_WordCount * p = new AP_CocoaDialog_WordCount(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_WordCount::AP_CocoaDialog_WordCount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	AP_Dialog_WordCount(pDlgFactory,dlgid),
	m_dlg(nil)
{
	// 
}

AP_CocoaDialog_WordCount::~AP_CocoaDialog_WordCount(void)
{
	// 
}

/*****************************************************************/

void  AP_CocoaDialog_WordCount::activate(void)
{
	event_Update();

	[[m_dlg window] orderFront:m_dlg];
}

void AP_CocoaDialog_WordCount::runModeless(XAP_Frame * pFrame)
{
	m_dlg = [[AP_CocoaDialog_WordCountController alloc] initFromNib];

	[m_dlg setXAPOwner:this];

	NSWindow * window = [m_dlg window];

	/* Save dialog the ID number and pointer to the widget
	 */
	UT_sint32 sid = static_cast<UT_sint32>(getDialogId());
	m_pApp->rememberModelessId (sid, (XAP_Dialog_Modeless *) m_pDialog);

	/* Now construct the timer for auto-updating
	 */
	m_bAutoUpdate_happening_now  = false;
	m_bDestroy_says_stopupdating = false;

	m_pAutoUpdateWC = UT_Timer::static_constructor(autoupdateWC, this);
	m_pAutoUpdateWC->set(1000);

	activate();
}

void AP_CocoaDialog_WordCount::autoupdateWC(UT_Worker * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.

	AP_CocoaDialog_WordCount * pDialog = (AP_CocoaDialog_WordCount *) pTimer->getInstanceData();

	// Handshaking code

	if ((pDialog->m_bDestroy_says_stopupdating != true) && (pDialog->m_bAutoUpdate_happening_now != true))
	{
		pDialog->m_bAutoUpdate_happening_now = true;
		pDialog->event_Update();
		pDialog->m_bAutoUpdate_happening_now = false;
	}
}        

void AP_CocoaDialog_WordCount::event_Update(void)
{
	setCountFromActiveFrame();

	_updateWindowData();
}

void AP_CocoaDialog_WordCount::event_CloseWindow(void)
{
	m_answer = AP_Dialog_WordCount::a_CANCEL;	
}

void AP_CocoaDialog_WordCount::notifyActiveFrame(XAP_Frame * pFrame)
{
	event_Update();
}

void AP_CocoaDialog_WordCount::destroy(void)
{
	m_bDestroy_says_stopupdating = true;

	while (m_bAutoUpdate_happening_now == true) 
		;
	m_pAutoUpdateWC->stop();

	DELETEP(m_pAutoUpdateWC);

	m_answer = AP_Dialog_WordCount::a_CANCEL;	

	modeless_cleanup();

	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

void AP_CocoaDialog_WordCount::_updateWindowData(void)
{
	[m_dlg setCounts:&m_count];
}

void AP_CocoaDialog_WordCount::_populateWindowData(void)
{
	// 
}

@implementation AP_CocoaDialog_WordCountController

- (id)initFromNib
{
	if (self = [super initWithWindowNibName:@"ap_CocoaDialog_WordCount"])
		{
			_xap = 0;
		}
	return self;
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = static_cast<AP_CocoaDialog_WordCount *>(owner);
}

- (void)discardXAP
{
	_xap = 0;
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	LocalizeControl([self window],     pSS, AP_STRING_ID_DLG_WordCount_WordCountTitle);

	LocalizeControl(_wordCount,        pSS, AP_STRING_ID_DLG_WordCount_Words);
	LocalizeControl(_paraLabel,        pSS, AP_STRING_ID_DLG_WordCount_Paragraphs);
	LocalizeControl(_charSpaceLabel,   pSS, AP_STRING_ID_DLG_WordCount_Characters_Sp);
	LocalizeControl(_charNoSpaceLabel, pSS, AP_STRING_ID_DLG_WordCount_Characters_No);
	LocalizeControl(_linesLabel,       pSS, AP_STRING_ID_DLG_WordCount_Lines);
	LocalizeControl(_pageLabel,        pSS, AP_STRING_ID_DLG_WordCount_Pages);

	NSPanel * panel = (NSPanel *) [self window];

	[panel setBecomesKeyOnlyIfNeeded:YES];
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	_xap->event_CloseWindow();
}

- (void)setCounts:(FV_DocCount *)count
{
	[_wordCount        setIntValue:count->word ];
	[_paraCount        setIntValue:count->para ];
	[_charSpaceCount   setIntValue:count->ch_sp];
	[_charNoSpaceCount setIntValue:count->ch_no];
	[_linesCount       setIntValue:count->line ];
	[_pageCount        setIntValue:count->page ];
}

@end
