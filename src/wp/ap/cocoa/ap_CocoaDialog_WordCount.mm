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

#import "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_WordCount.h"
#include "ap_CocoaDialog_WordCount.h"

/*****************************************************************/


/*****************************************************************/

static UT_sint32 i_WordCountunix_first_time = 0;

XAP_Dialog * AP_CocoaDialog_WordCount::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_WordCount * p = new AP_CocoaDialog_WordCount(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_WordCount::AP_CocoaDialog_WordCount(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id dlgid)
	: AP_Dialog_WordCount(pDlgFactory,dlgid),
		m_dlg(nil)
{
}

AP_CocoaDialog_WordCount::~AP_CocoaDialog_WordCount(void)
{
	[m_dlg release];
	m_dlg = nil;
}

/*****************************************************************/



void  AP_CocoaDialog_WordCount::activate(void)
{
	NSWindow* window = [m_dlg window];
	ConstructWindowName();
	[window setTitle:[NSString stringWithUTF8String:m_WindowName]];
	setCountFromActiveFrame ();
	_updateWindowData ();
	[window orderFront:m_dlg];
}


void AP_CocoaDialog_WordCount::runModeless(XAP_Frame * pFrame)
{
	NSWindow* window;

	UT_sint32 sid = static_cast<UT_sint32>(getDialogId());

	// This magic command displays the frame that characters will be
	// inserted into.
	m_dlg = [[AP_CocoaDialog_WordCountController alloc] initFromNib];
	[m_dlg setXAPOwner:this];
	
	// Save dialog the ID number and pointer to the widget
	m_pApp->rememberModelessId (sid, (XAP_Dialog_Modeless *)m_pDialog);
	window = [m_dlg window];


	ConstructWindowName();
	[window setTitle:[NSString stringWithUTF8String:m_WindowName]];
	[window orderFront:m_dlg];

	// Now construct the timer for auto-updating
	GR_Graphics * pG = NULL;
	m_pAutoUpdateWC = UT_Timer::static_constructor(autoupdateWC, this, pG);

	if (i_WordCountunix_first_time == 0) {	
		//  Set it update evey second to start with
		m_Update_rate = 1000;
		m_bAutoWC = true;
		i_WordCountunix_first_time = 1;
	}

	setUpdateCounter();
}

void    AP_CocoaDialog_WordCount::setUpdateCounter(void)
{
	m_bDestroy_says_stopupdating = false;
	m_bAutoUpdate_happening_now = false;

	float f_Update_rate = ((float)m_Update_rate) / 1000.0;
	[m_dlg setSeconds:f_Update_rate];
	if (m_bAutoWC == true)	{
		m_pAutoUpdateWC->stop();
		m_pAutoUpdateWC->set(m_Update_rate);
	}
	[m_dlg _syncControls];
}
         
void    AP_CocoaDialog_WordCount::autoupdateWC(UT_Worker * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.

	AP_CocoaDialog_WordCount * pDialog =  (AP_CocoaDialog_WordCount *)pTimer->getInstanceData();

	// Handshaking code

	if (pDialog->m_bDestroy_says_stopupdating != true)
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


void AP_CocoaDialog_WordCount::event_Checkbox(bool enabled)
{
	if (enabled)
	{
		m_pAutoUpdateWC->stop();
		// This actually does gtk_timer_add...
		m_pAutoUpdateWC->set(m_Update_rate);
		m_bAutoWC = true;
	}
	else
	{
		m_pAutoUpdateWC->stop();
		m_bAutoWC = false;
	}
	[m_dlg _syncControls];
}

void AP_CocoaDialog_WordCount::event_Spin(void)
{
	m_Update_rate = lrintf([m_dlg seconds] * 1000);

	// We need this because calling adds a new timer to the gtk list!
	// So we have to stop the timer to remove it from the gtk list before
	// changing the speed of the timer.

	m_pAutoUpdateWC->stop();

	// This actually does gtk_timer_add...

	m_pAutoUpdateWC->set(m_Update_rate);
}

void AP_CocoaDialog_WordCount::event_CloseWindow(void)
{
	m_answer = AP_Dialog_WordCount::a_CANCEL;	
}

void AP_CocoaDialog_WordCount::notifyActiveFrame(XAP_Frame *pFrame)
{
	UT_ASSERT(m_dlg);
	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:m_WindowName]];
	event_Update();
}

void AP_CocoaDialog_WordCount::destroy(void)
{
	m_bDestroy_says_stopupdating = true;
	while (m_bAutoUpdate_happening_now == true) 
		;
	m_pAutoUpdateWC->stop();
	m_answer = AP_Dialog_WordCount::a_CANCEL;	
	modeless_cleanup();
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
	DELETEP(m_pAutoUpdateWC);
}

/*****************************************************************/
void AP_CocoaDialog_WordCount::_updateWindowData(void)
{
	[m_dlg setCounts:&m_count];

//	gtk_frame_set_label (GTK_FRAME(m_pTableframe), getActiveFrame ()->getTitle (60));
}


void AP_CocoaDialog_WordCount::_populateWindowData(void)
{

}


@implementation AP_CocoaDialog_WordCountController

- (id)initFromNib
{
	self = [super initWithWindowNibName:@"ap_CocoaDialog_WordCount"];
	return self;
}

- (void)setXAPOwner:(XAP_Dialog*)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_WordCount*>(owner);
	UT_ASSERT(_xap);
}

- (void)discardXAP
{
	_xap = nil;
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	LocalizeControl(_refreshBtn, pSS, XAP_STRING_ID_DLG_Update);
	LocalizeControl(_autoUpdateBtn, pSS, AP_STRING_ID_DLG_WordCount_Auto_Update);
	LocalizeControl(_secondsLabel, pSS, AP_STRING_ID_DLG_WordCount_Update_Rate);
	LocalizeControl(_wordCount, pSS, AP_STRING_ID_DLG_WordCount_Words);
	LocalizeControl(_paraLabel, pSS, AP_STRING_ID_DLG_WordCount_Paragraphs);
	LocalizeControl(_charSpaceLabel, pSS, AP_STRING_ID_DLG_WordCount_Characters_Sp);
	LocalizeControl(_charNoSpaceLabel, pSS, AP_STRING_ID_DLG_WordCount_Characters_No);
	LocalizeControl(_linesLabel, pSS, AP_STRING_ID_DLG_WordCount_Lines);
	LocalizeControl(_pageLabel, pSS, AP_STRING_ID_DLG_WordCount_Pages);
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	_xap->event_CloseWindow();
}


- (IBAction)autoUpdateAction:(id)sender
{
	_xap->event_Checkbox([sender state] == NSOnState);
}

- (void)_syncControls
{
	if ([_autoUpdateBtn state] == NSOnState) {
		[_secondsData setEnabled:YES];
		[_secondsLabel setEnabled:YES];
		[_stepper setEnabled:YES];
	}
	else {
		[_secondsData setEnabled:NO];
		[_secondsLabel setEnabled:NO];
		[_stepper setEnabled:NO];
	}
}

- (IBAction)refreshAction:(id)sender
{
	_xap->event_Update();
}

- (IBAction)secondsUpdated:(id)sender
{
	[_stepper setFloatValue:[sender floatValue]];
	_xap->event_Spin();
}

- (IBAction)stepperAction:(id)sender
{
	[_secondsData setFloatValue:[sender floatValue]];
	_xap->event_Spin();
}


- (void)setSeconds:(float)sec
{
	[_secondsData setFloatValue:sec];
	[_stepper setFloatValue:sec];	
}

- (float)seconds
{
	return [_secondsData floatValue];
}

- (void)setCounts:(FV_DocCount*)count
{
	[_wordCount setIntValue:count->word];
	[_paraCount setIntValue:count->para];
	[_charSpaceCount setIntValue:count->ch_sp];
	[_charNoSpaceCount setIntValue:count->ch_no];
	[_linesCount setIntValue:count->line];
	[_pageCount setIntValue:count->page];
}

@end

