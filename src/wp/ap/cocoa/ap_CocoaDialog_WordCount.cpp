/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2003-2005 Hubert Figuiere
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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_timer.h"

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

void AP_CocoaDialog_WordCount::runModeless(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[AP_CocoaDialog_WordCountController alloc] initFromNib];

	[m_dlg setXAPOwner:this];

	[m_dlg window]; // just to make sure the nib is loaded.
	localizeDialog();

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

	updateDialogData();
}

void AP_CocoaDialog_WordCount::event_CloseWindow(void)
{
	m_answer = AP_Dialog_WordCount::a_CANCEL;	
}

void AP_CocoaDialog_WordCount::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
	event_Update();
}

void AP_CocoaDialog_WordCount::destroy(void)
{
	m_bDestroy_says_stopupdating = true;

	m_pAutoUpdateWC->stop();

	DELETEP(m_pAutoUpdateWC);

	m_answer = AP_Dialog_WordCount::a_CANCEL;	

	modeless_cleanup();

	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}


@implementation AP_CocoaDialog_WordCountController

- (id)initFromNib
{
	if (self = [super initWithWindowNibName:@"ap_CocoaDialog_WordCount"])
	{
        _xap = NULL;
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
	NSPanel * panel = (NSPanel *) [self window];

	[panel setBecomesKeyOnlyIfNeeded:YES];
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	_xap->event_CloseWindow();
}


- (XAP_CocoaWidget *)getWidget:(int)wid
{
	switch(wid) {
	case AP_Dialog_WordCount::DIALOG_WID:
		return new XAP_CocoaWidget([self window]);
	case AP_Dialog_WordCount::CLOSE_BTN_WID:
		return new XAP_CocoaWidget(NULL);
	case AP_Dialog_WordCount::TITLE_LBL_WID:
		return new XAP_CocoaWidget(_titleLabel);
	case AP_Dialog_WordCount::PAGES_LBL_WID:
		return new XAP_CocoaWidget(_pageLabel);
	case AP_Dialog_WordCount::PAGES_VAL_WID:
		return new XAP_CocoaWidget(_pageCount);
	case AP_Dialog_WordCount::LINES_LBL_WID:
		return new XAP_CocoaWidget(_linesLabel);
	case AP_Dialog_WordCount::LINES_VAL_WID:
		return new XAP_CocoaWidget(_linesCount);
	case AP_Dialog_WordCount::CHARNSP_LBL_WID:
		return new XAP_CocoaWidget(_charNoSpaceLabel);
	case AP_Dialog_WordCount::CHARNSP_VAL_WID:
		return new XAP_CocoaWidget(_charNoSpaceCount);
	case AP_Dialog_WordCount::CHARSP_LBL_WID:
		return new XAP_CocoaWidget(_charSpaceLabel);
	case AP_Dialog_WordCount::CHARSP_VAL_WID:
		return new XAP_CocoaWidget(_charSpaceCount);
	case AP_Dialog_WordCount::PARA_LBL_WID:
		return new XAP_CocoaWidget(_paraLabel);
	case AP_Dialog_WordCount::PARA_VAL_WID:
		return new XAP_CocoaWidget(_paraCount);
	case AP_Dialog_WordCount::WORDS_LBL_WID:
		return new XAP_CocoaWidget(_wordLabel);
	case AP_Dialog_WordCount::WORDS_VAL_WID:
		return new XAP_CocoaWidget(_wordCount);
	case AP_Dialog_WordCount::WORDSNF_LBL_WID:
		return new XAP_CocoaWidget(_wordNoFNLabel);
	case AP_Dialog_WordCount::WORDSNF_VAL_WID:
		return new XAP_CocoaWidget(_wordNoFNCount);
	}
	return NULL;
}


@end
