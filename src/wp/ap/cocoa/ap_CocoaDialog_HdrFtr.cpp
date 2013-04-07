/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2003, 2009 Hubert Figuiere
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
#include <stdio.h>

// This header defines some functions for Cocoa dialogs,
// like centering them, measuring them, etc.
#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_CocoaDialog_HdrFtr.h"
#include "ut_debugmsg.h"



/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_HdrFtr::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_HdrFtr * p = new AP_CocoaDialog_HdrFtr(pFactory,dlgid);
	return (XAP_Dialog *) p;
}

AP_CocoaDialog_HdrFtr::AP_CocoaDialog_HdrFtr(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_HdrFtr(pDlgFactory,dlgid)
{
}

AP_CocoaDialog_HdrFtr::~AP_CocoaDialog_HdrFtr(void)
{
}

void AP_CocoaDialog_HdrFtr::runModal(XAP_Frame * /*pFrame*/)
{

	m_dlg = [[AP_CocoaDialog_HdrFtrController alloc] initFromNib];
	
	// used similarly to convert between text and numeric arguments
	[m_dlg setXAPOwner:this];

	// build the dialog
	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);

	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}


void AP_CocoaDialog_HdrFtr::eventOk (void)
{
	setAnswer (a_OK);
	[NSApp stopModal];
}

void AP_CocoaDialog_HdrFtr::eventCancel (void)
{
	setAnswer(a_CANCEL);
	[NSApp stopModal];
}

/*!
 * A check button has controlling a footer type has been changed.
 */
void AP_CocoaDialog_HdrFtr::CheckChanged(id checkbox)
{
	int state = [checkbox state];
	setValue(static_cast<HdrFtr_Control>([checkbox tag]), (state == NSOnState ? true : false), true);
}

/*!
 * Update the XP values of the spin button.
 */
void AP_CocoaDialog_HdrFtr::RestartSpinChanged(UT_sint32 RestartValue)
{
	setRestart(true, RestartValue, true);
}


@implementation AP_CocoaDialog_HdrFtrController

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_HdrFtr"];
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
	_xap = dynamic_cast<AP_CocoaDialog_HdrFtr*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_HdrFtr_Title);
		LocalizeControl(okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(headerBox, pSS, AP_STRING_ID_DLG_HdrFtr_HeaderFrame);
		LocalizeControl(headerFacingBtn, pSS, AP_STRING_ID_DLG_HdrFtr_HeaderEven);
		[headerFacingBtn setTag:AP_Dialog_HdrFtr::HdrEven];
		LocalizeControl(headerFirstBtn, pSS, AP_STRING_ID_DLG_HdrFtr_HeaderFirst);
		[headerFirstBtn setTag:AP_Dialog_HdrFtr::HdrFirst];
		LocalizeControl(headerLastBtn, pSS, AP_STRING_ID_DLG_HdrFtr_HeaderLast);
		[headerLastBtn setTag:AP_Dialog_HdrFtr::HdrLast];
		LocalizeControl(footerBox, pSS, AP_STRING_ID_DLG_HdrFtr_FooterFrame);
		LocalizeControl(footerFacingBtn, pSS, AP_STRING_ID_DLG_HdrFtr_FooterEven);
		[footerFacingBtn setTag:AP_Dialog_HdrFtr::FtrEven];
		LocalizeControl(footerFirstBtn, pSS, AP_STRING_ID_DLG_HdrFtr_FooterFirst);
		[footerFirstBtn setTag:AP_Dialog_HdrFtr::FtrFirst];
		LocalizeControl(footerLastBtn, pSS, AP_STRING_ID_DLG_HdrFtr_FooterLast);
		[footerLastBtn setTag:AP_Dialog_HdrFtr::FtrLast];
		LocalizeControl(pageNumberBox, pSS, AP_STRING_ID_DLG_HdrFtr_PageNumberProperties);
		LocalizeControl(restartPgNumberBtn, pSS, AP_STRING_ID_DLG_HdrFtr_RestartCheck);
		LocalizeControl(restartAtLabel, pSS, AP_STRING_ID_DLG_HdrFtr_RestartNumbers);
		
		
		[restartStepper setFloatValue:_xap->getRestartValue()];
		if(_xap->isRestart()) {
			[restartStepper setEnabled:YES];
			[restartAtLabel setEnabled:YES];
			[restartAtData setEnabled:YES];
			[restartPgNumberBtn setState:NSOnState];
		}
		else {
			[restartStepper setEnabled:NO];
			[restartAtLabel setEnabled:NO];
			[restartAtData setEnabled:NO];
		}
		UT_sint32 j = static_cast<UT_sint32>(AP_Dialog_HdrFtr::HdrEven);
		for(j = static_cast<UT_sint32>(AP_Dialog_HdrFtr::HdrEven) ; j<= static_cast<UT_sint32>(AP_Dialog_HdrFtr::FtrLast); j++)	{
			bool value = _xap->getValue( static_cast<AP_Dialog_HdrFtr::HdrFtr_Control>(j));
			[[[[self window] contentView] viewWithTag:j] setState:(value ? NSOnState : NSOffState)];
		}
	}
}


- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->eventCancel();
}

- (IBAction)btnAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->CheckChanged(sender);
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->eventOk();
}

- (IBAction)restartAction:(id)sender
{
	[restartStepper setIntValue:[sender intValue]];
	_xap->RestartSpinChanged([sender intValue]);
}

- (IBAction)restartBtnAction:(id)sender
{
	UT_UNUSED(sender);
	UT_sint32 RestartValue = [restartAtData intValue];
	if([restartPgNumberBtn state] == NSOnState)
	{
		[restartAtLabel setEnabled:YES];
		[restartAtData setEnabled:YES];
		[restartStepper setEnabled:YES];
		_xap->setRestart(true, RestartValue, true);
	}
	else
	{
		[restartAtLabel setEnabled:NO];
		[restartAtData setEnabled:NO];
		[restartStepper setEnabled:NO];
		_xap->setRestart(false, RestartValue, true);
	}
}

- (IBAction)restartStepperAction:(id)sender
{	
	[restartAtData setIntValue:[sender intValue]];
	_xap->RestartSpinChanged([sender intValue]);
}

@end
