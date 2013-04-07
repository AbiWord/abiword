/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
 * Copyright (C) 2001-2003 Hubert Figuiere
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
#include "xap_CocoaDialog_Utilities.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MarkRevisions.h"
#include "ap_CocoaDialog_MarkRevisions.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_MarkRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_MarkRevisions * p = new AP_CocoaDialog_MarkRevisions(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_MarkRevisions::AP_CocoaDialog_MarkRevisions(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_MarkRevisions(pDlgFactory,dlgid),
		m_dlg(nil)
{
}

AP_CocoaDialog_MarkRevisions::~AP_CocoaDialog_MarkRevisions(void)
{
}

void AP_CocoaDialog_MarkRevisions::event_OK ()
{
	m_answer = AP_Dialog_MarkRevisions::a_OK;
	setComment2 ([[m_dlg comment] UTF8String]);
	[NSApp stopModal];
}

void AP_CocoaDialog_MarkRevisions::event_Cancel ()
{
	m_answer = AP_Dialog_MarkRevisions::a_CANCEL ;
	[NSApp stopModal];
}

void AP_CocoaDialog_MarkRevisions::event_FocusToggled ()
{
	bool second_active = false;
	int toggled = [m_dlg toggled];
	
	if ((toggled == 2) || (getRadio1Label() == NULL)) {
		second_active = true;
	}
	[m_dlg setItems2Enabled:second_active];
}


void AP_CocoaDialog_MarkRevisions::runModal(XAP_Frame * /*pFrame*/)
{
	/* comment below should be re-read after analysing the UNIX code  -- Hub*/
	
	/*
	   This is the only function you need to implement, and the MarkRevisions
	   dialogue should look like this:

	   ----------------------------------------------------
	   | Title                                            |
       ----------------------------------------------------
	   |                                                  |
	   | O Radio1                                         |
	   |    Comment1 (a label)                            |
	   |                                                  |
	   | O Radio2                                         |
	   |    Comment2Label                                 |
	   |    Comment2 (an edit control)                    |
       |                                                  |
       |                                                  |
       |     OK_BUTTON              CANCEL_BUTTON         |
	   ----------------------------------------------------

	   Where: Title, Comment1 and Comment2Label are labels, Radio1-2
	   is are radio buttons, Comment2 is an Edit control.

	   Use getTitle(), getComment1(), getComment2Label(), getRadio1Label()
	   and getRadio2Label() to get the labels (the last two for the radio
	   buttons), note that you are responsible for freeing the
	   pointers returned by getLable1() and getComment1() using FREEP
	   (but not the rest!)

	   if getLabel1() returns NULL, hide the radio buttons and enable
	   the Edit box; otherwise the Edit box should be only enabled when
	   Radio2 is selected.

	   Use setComment2(const char * pszString) to store the contents of the Edit control
       when the dialogue closes; make sure that you freee pszString afterwards.


	*/
	m_dlg = [[AP_CocoaDialog_MarkRevisionsController alloc] initFromNib];
	
	[m_dlg setXAPOwner:this];

	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);

	event_FocusToggled();

	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}


@implementation AP_CocoaDialog_MarkRevisionsController


- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_MarkRevisions"];
}

-(void)discardXAP
{
	_xap = NULL; 
}

-(void)dealloc
{
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_MarkRevisions*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		char * str;
		char * str2;
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_MarkRevisions_Title);
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		str = _xap->getRadio1Label();
		if (str) {
			[_radio1 setTitle:[NSString stringWithUTF8String:str]];
			[_radio2 setTitle:[NSString stringWithUTF8String:_xap->getRadio2Label()]];
			str2 = _xap->getComment1();
			if (str2) {
				[_label1 setStringValue:[NSString stringWithUTF8String:str2]];
				FREEP(str2);	/* getComment1() is allocated */
			}
			FREEP(str);
		}
		else {
			[_radio1 removeFromSuperview];
			_radio1 = nil;
			[_radio2 removeFromSuperview];
			_radio2 = nil;
			[_label1 removeFromSuperview];
			_label1 = nil;
		}
		[_label2 setStringValue:[NSString stringWithUTF8String:_xap->getComment2Label()]];
	}
}


- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel();
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_OK();
}

- (IBAction)radio1Action:(id)sender
{
	UT_UNUSED(sender);
	[_radio2 setState:NSOffState];
	_xap->event_FocusToggled();
}

- (IBAction)radio2Action:(id)sender
{
	UT_UNUSED(sender);
	[_radio1 setState:NSOffState];
	_xap->event_FocusToggled();
}

- (NSString*)comment
{
	return [_commentData stringValue];
}

- (int)toggled
{
	if ([_radio1 state] == NSOnState) {
		return 1;
	}
	else {
		return 2;
	}
}

/*!
	Toggle enabling of bottom part of the dialog.
 */
- (void)setItems2Enabled:(bool)state
{
	[_label2 setEnabled:state];
	[_commentData setEnabled:state];
}


@end
