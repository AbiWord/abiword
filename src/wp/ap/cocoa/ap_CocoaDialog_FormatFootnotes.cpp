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

#import <Cocoa/Cocoa.h>

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_FormatFootnotes.h"
#include "ap_CocoaDialog_FormatFootnotes.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_FormatFootnotes::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_FormatFootnotes * p = new AP_CocoaDialog_FormatFootnotes(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_FormatFootnotes::AP_CocoaDialog_FormatFootnotes(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_FormatFootnotes(pDlgFactory,dlgid)
{
}

AP_CocoaDialog_FormatFootnotes::~AP_CocoaDialog_FormatFootnotes(void)
{
}

void AP_CocoaDialog_FormatFootnotes::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	m_dlg = [[AP_CocoaDialog_FormatFootnotes_Controller alloc] initFromNib];
	
	// used similarly to convert between text and numeric arguments
	[m_dlg setXAPOwner:this];
	setFrame(pFrame);
	setInitialValues();
	
	// build the dialog
	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);
	refreshVals();

	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

void AP_CocoaDialog_FormatFootnotes::event_OK(void)
{
	setAnswer(AP_Dialog_FormatFootnotes::a_OK);
	[NSApp stopModal];
}


void AP_CocoaDialog_FormatFootnotes::event_Cancel(void)
{
	setAnswer(AP_Dialog_FormatFootnotes::a_CANCEL);
	[NSApp stopModal];
}


void AP_CocoaDialog_FormatFootnotes::refreshVals(void)
{
	[m_dlg setFtNtInitialValue:getFootnoteVal()];
	[m_dlg setEndNtInitialValue:getEndnoteVal()];

	if(getRestartFootnoteOnSection()) {
		[m_dlg setFtNtRestart:1];
	}
	else if(getRestartFootnoteOnPage()) {
		[m_dlg setFtNtRestart:2];
	}
	else {
		[m_dlg setFtNtRestart:0];
	}

	if(getRestartEndnoteOnSection()) {
		[m_dlg setEndNtPlacement:0];
	}
	else if(getPlaceAtDocEnd()) {
		[m_dlg setEndNtPlacement:1];
	}

	[m_dlg setEndNtRestartOnSec:getRestartEndnoteOnSection()];

	[m_dlg setNtType:getFootnoteType() isFtNt:YES];
	[m_dlg setNtType:getEndnoteType() isFtNt:NO];
}


void AP_CocoaDialog_FormatFootnotes::event_MenuEndNtPlacementChange(NSPopUpButton* widget)
{
	NSMenuItem* item = [widget selectedItem];
	switch ([item tag]) {
	case 0:
		setPlaceAtSecEnd(false);
		setPlaceAtDocEnd(true);
		refreshVals();
		break;
	case 1:
		setPlaceAtSecEnd(true);
		setPlaceAtDocEnd(false);
		refreshVals();
		break;
	default:
		break;
	}
}


void AP_CocoaDialog_FormatFootnotes::event_MenuFtNtRestartChange(NSPopUpButton* widget)
{
	NSMenuItem* item = [widget selectedItem];
	switch ([item tag]) {
	case 2:
		setRestartFootnoteOnPage(true);
		setRestartFootnoteOnSection(false);
		refreshVals();
		break;
	case 1:
		setRestartFootnoteOnPage(false);
		setRestartFootnoteOnSection(true);
		refreshVals();
		break;
	case 0:
		setRestartFootnoteOnPage(false);
		setRestartFootnoteOnSection(false);
		refreshVals();
		break;
	default:
		break;
	}
}

void AP_CocoaDialog_FormatFootnotes::event_MenuStyleChange(NSPopUpButton* widget, bool bIsFootnote)
{
	FootnoteType iType = static_cast<FootnoteType>([widget indexOfSelectedItem]);
	if(bIsFootnote)	{
		setFootnoteType(iType);
	}
	else {
		setEndnoteType(iType);
	}
	refreshVals();
}

void AP_CocoaDialog_FormatFootnotes::event_FootInitialValueChange(NSTextField* widget)
{
	UT_sint32 val = [widget intValue];
	if (val == getFootnoteVal()) {
		return;
	}
	setFootnoteVal(val);
	refreshVals();
}


void AP_CocoaDialog_FormatFootnotes::event_EndInitialValueChange(NSTextField* widget)
{
	UT_sint32 val = [widget intValue];
	if (val == getEndnoteVal()) {
		return;
	}
	setEndnoteVal(val);
	refreshVals();
}


void AP_CocoaDialog_FormatFootnotes::event_EndRestartSection(NSButton* widget)
{
	setRestartEndnoteOnSection([widget state] == NSOnState);
}



@implementation AP_CocoaDialog_FormatFootnotes_Controller

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_FormatFootnotes"];
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
	_xap = dynamic_cast<AP_CocoaDialog_FormatFootnotes*>(owner);
}


static void _fillPopupWithFootnoteTypeDesc(NSPopUpButton* menu, const FootnoteTypeDesc *vec)
{
		
	[menu removeAllItems];
	const FootnoteTypeDesc *current;
	for (current = vec; current->n != _FOOTNOTE_TYPE_INVALID; current++) {
		[menu addItemWithTitle:[NSString stringWithUTF8String:current->label]];
	}
}



-(void)windowDidLoad
{
	if(_xap) {
		/* localize */
		const XAP_StringSet * pSS = _xap->getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_FormatFootnotes_Title);
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);	
		LocalizeControl(_ftNtBox, pSS, AP_STRING_ID_DLG_FormatFootnotes_Footnotes);	
		LocalizeControl(_ftNtStyleLabel, pSS, AP_STRING_ID_DLG_FormatFootnotes_FootStyle);	
		LocalizeControl(_ftNtRestartLabel, pSS, AP_STRING_ID_DLG_FormatFootnotes_FootnoteRestart);	
		LocalizeControl(_ftNtInitialValueLabel, pSS, AP_STRING_ID_DLG_FormatFootnotes_FootInitialVal);	
		LocalizeControl(_endNtBox, pSS, AP_STRING_ID_DLG_FormatFootnotes_Endnotes);	
		LocalizeControl(_endNtStyleLabel, pSS, AP_STRING_ID_DLG_FormatFootnotes_EndStyle);	
		LocalizeControl(_endNtPlacementLabel, pSS, AP_STRING_ID_DLG_FormatFootnotes_EndPlacement);	
		LocalizeControl(_endNtInitialValueLabel, pSS, AP_STRING_ID_DLG_FormatFootnotes_EndInitialVal);	
		LocalizeControl(_endNtRestartSectionBtn, pSS, AP_STRING_ID_DLG_FormatFootnotes_EndRestartSec);
	
		_fillPopupWithFootnoteTypeDesc(_ftNtStylePopup, AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList());
		_fillPopupWithFootnoteTypeDesc(_endNtStylePopup, AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList());

		// Footnotes number menu
		[_ftNtRestartPopup removeAllItems];
		AppendLocalizedMenuItem(_ftNtRestartPopup, pSS, AP_STRING_ID_DLG_FormatFootnotes_FootRestartNone, 0);	
		AppendLocalizedMenuItem(_ftNtRestartPopup, pSS, AP_STRING_ID_DLG_FormatFootnotes_FootRestartSec, 1);	
		AppendLocalizedMenuItem(_ftNtRestartPopup, pSS, AP_STRING_ID_DLG_FormatFootnotes_FootRestartPage, 2);	

		// Endnotes placement menu
		[_endNtPlacementPopup removeAllItems];
		AppendLocalizedMenuItem(_endNtPlacementPopup, pSS, AP_STRING_ID_DLG_FormatFootnotes_EndPlaceEndDoc, 0);	
		AppendLocalizedMenuItem(_endNtPlacementPopup, pSS, AP_STRING_ID_DLG_FormatFootnotes_EndPlaceEndSec, 1);	
	}
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel();
}

- (IBAction)initialEndNtValueAction:(id)sender
{
	_xap->event_EndInitialValueChange(sender);
}

- (IBAction)initialEndNtValueStepperAction:(id)sender
{
	_xap->event_EndInitialValueChange(sender);
}

- (IBAction)initialFtNtValueAction:(id)sender
{
	_xap->event_FootInitialValueChange(sender);
}

- (IBAction)initialFtNtValueStepperAction:(id)sender
{
	_xap->event_FootInitialValueChange(sender);
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_OK();
}

- (IBAction)styleMenuAction:(id)sender
{
	_xap->event_MenuStyleChange(sender, (sender == _ftNtStylePopup));
}


- (IBAction)ftNtRestartMenuAction:(id)sender
{
	_xap->event_MenuFtNtRestartChange(sender);
}


- (IBAction)endNtPlacementMenuAction:(id)sender
{
	_xap->event_MenuEndNtPlacementChange(sender);
}

- (IBAction)endNtRestartSecAction:(id)sender
{
	_xap->event_EndRestartSection(sender);
}


- (void)setFtNtInitialValue:(int)val
{
	[_ftNtInitialValue setIntValue:val];
	[_ftNtInitialValueStepper setIntValue:val];	
}

- (void)setEndNtInitialValue:(int)val
{
	[_endNtInitialValue setIntValue:val];
	[_endNtInitialValueStepper setIntValue:val];
}


- (void)setFtNtRestart:(int)val
{
	[_ftNtRestartPopup selectItemAtIndex:val];
}


- (void)setEndNtPlacement:(int)val
{
	[_endNtPlacementPopup selectItemAtIndex:val];
}


- (void)setEndNtRestartOnSec:(bool)val
{
	[_endNtRestartSectionBtn setState:(val?NSOnState:NSOffState)];
}


- (void)setNtType:(FootnoteType)type isFtNt:(BOOL)isFootnote
{
	if (isFootnote) {
		[_ftNtStylePopup selectItemAtIndex:(int)type];
	}
	else {
		[_endNtStylePopup selectItemAtIndex:(int)type];
	}
}

@end

