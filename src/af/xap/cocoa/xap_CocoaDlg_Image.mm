/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz
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
#import <Cocoa/Cocoa.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Image.h"
#include "xap_CocoaDlg_Image.h"

/*****************************************************************/

void XAP_CocoaDialog_Image::event_Ok ()
{
	setAnswer(XAP_Dialog_Image::a_OK);
	setTitle ([[m_dlg titleEntry] UTF8String]);
	setAlt ([[m_dlg altEntry] UTF8String]);
	[NSApp stopModal];
}

void XAP_CocoaDialog_Image::event_Cancel ()
{  
	setAnswer(XAP_Dialog_Image::a_Cancel);
	[NSApp stopModal];
}

void XAP_CocoaDialog_Image::aspectCheckbox()
{
	bool bAspect;
	
	if([m_dlg preserveRatio] && (m_dHeightWidth > 0.0001)) {
		bAspect = true;
	}
	else {
		bAspect = false;
	}
	setPreserveAspect(bAspect);
}

void XAP_CocoaDialog_Image::doHeightSpin(void)
{
	bool bIncrement = true;
	UT_sint32 val = [m_dlg heightNum];
	if (val == m_iHeight) {
		return;
	}
	if(val < m_iHeight) {
		bIncrement = false;
	}
	
	m_iHeight = val;
	incrementHeight(bIncrement);
	adjustWidthForAspect();
	[m_dlg setHeightEntry:[NSString stringWithUTF8String:getHeightString()]];
}


void XAP_CocoaDialog_Image::doWidthSpin(void)
{
	bool bIncrement = true;
	UT_sint32 val = [m_dlg widthNum];
	if (val == m_iWidth) {
		return;	
	}
	if(val < m_iWidth) {
		bIncrement = false;
	}
	m_iWidth = val;
	incrementWidth(bIncrement);
	adjustHeightForAspect();
	[m_dlg setWidthEntry:[NSString stringWithUTF8String:getWidthString()]];
}

void XAP_CocoaDialog_Image::doHeightEntry(void)
{
	const char * szHeight = [[m_dlg heightEntry] UTF8String];
	if(UT_determineDimension(szHeight,DIM_none) != DIM_none)
	{
		setHeight(szHeight);

		[m_dlg setHeightEntry:[NSString stringWithUTF8String:getHeightString()]];
	}
	adjustWidthForAspect();
}

void XAP_CocoaDialog_Image::setHeightEntry(void)
{
	[m_dlg setHeightEntry:[NSString stringWithUTF8String:getHeightString()]];
}

void XAP_CocoaDialog_Image::setWidthEntry(void)
{
	[m_dlg setWidthEntry:[NSString stringWithUTF8String:getWidthString()]];
}

void XAP_CocoaDialog_Image::doWidthEntry(void)
{
	const char * szWidth = [[m_dlg widthEntry] UTF8String];
	if(UT_determineDimension(szWidth,DIM_none) != DIM_none)
	{
		setWidth(szWidth);
		[m_dlg setWidthEntry:[NSString stringWithUTF8String:getWidthString()]];
	}
	adjustHeightForAspect();
}


void XAP_CocoaDialog_Image::adjustHeightForAspect(void)
{
	if(getPreserveAspect()) {
		setHeightEntry();
	}
}

void XAP_CocoaDialog_Image::adjustWidthForAspect(void)
{
	if(getPreserveAspect()) {
		setWidthEntry();
	}
}

/***********************************************************************/

XAP_Dialog * XAP_CocoaDialog_Image::static_constructor(XAP_DialogFactory * pFactory,
						      XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_Image * p = new XAP_CocoaDialog_Image(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_Image::XAP_CocoaDialog_Image(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id dlgid)
  : XAP_Dialog_Image(pDlgFactory,dlgid),
	m_iHeight(0),
	m_iWidth(0),
	m_dHeightWidth(0),
	m_dlg(nil)
{
}

XAP_CocoaDialog_Image::~XAP_CocoaDialog_Image(void)
{
}

void XAP_CocoaDialog_Image::runModal(XAP_Frame * pFrame)
{
	m_dlg = [[XAP_CocoaDialog_ImageController alloc] initFromNib];
	
	[m_dlg setXAPOwner:this];

	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);

	setHeightEntry();
	setWidthEntry();
	double height = UT_convertToInches(getHeightString());
	double width = UT_convertToInches(getWidthString());
	
	if((height > 0.0001) && (width > 0.0001)) {
		m_dHeightWidth = height/width;
	}
	else {
		m_dHeightWidth = 0.0;
		[m_dlg setPreserveRatio:NO];
	}	  

	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}



@implementation XAP_CocoaDialog_ImageController

- (id)initFromNib
{
	self = [super initWithWindowNibName:@"xap_CocoaDlg_Image"];
	return self;
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
	_xap = dynamic_cast<XAP_CocoaDialog_Image*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, XAP_STRING_ID_DLG_Image_Title);
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_heightLabel, pSS, XAP_STRING_ID_DLG_Image_Height);
		LocalizeControl(_widthLabel, pSS, XAP_STRING_ID_DLG_Image_Width);
		LocalizeControl(_titleLabel, pSS, XAP_STRING_ID_DLG_Image_LblTitle);
		LocalizeControl(_altLabel, pSS, XAP_STRING_ID_DLG_Image_LblAlt);
		LocalizeControl(_preserveAspectBtn, pSS, XAP_STRING_ID_DLG_Image_Aspect);
		[_preserveAspectBtn setState:(_xap->getPreserveAspect()?NSOnState:NSOffState)];
		[_titleData setStringValue:[NSString stringWithUTF8String:_xap->getTitle().utf8_str()]];
		[_altData setStringValue:[NSString stringWithUTF8String:_xap->getAlt().utf8_str()]];
		/* FIXME: we probably have smarter default values Unix code doesn't.*/
		[_heightNumStepper setIntValue:1];
		[_heightNumData setIntValue:1];
		[_widthNumStepper setIntValue:1];
		[_widthNumData setIntValue:1];
	}
}


- (IBAction)cancelAction:(id)sender
{
	_xap->event_Cancel();
}

- (IBAction)heightChanged:(id)sender
{
	_xap->doHeightEntry();
}

- (IBAction)heightNumChanged:(id)sender
{
	_xap->doHeightSpin();
}

- (IBAction)heightNumStepperChanged:(id)sender
{
	_xap->doHeightSpin();
}

- (IBAction)okAction:(id)sender
{
	_xap->event_Ok();
}

- (IBAction)preserveAction:(id)sender
{
	_xap->aspectCheckbox();
}

- (IBAction)widthChanged:(id)sender
{
	_xap->doWidthEntry();
}

- (IBAction)widthNumChanged:(id)sender
{
	_xap->doWidthSpin();
}

- (IBAction)widthNumStepperChanged:(id)sender
{
	_xap->doWidthSpin();
}


- (NSString*)titleEntry
{
	return [_titleData stringValue];
}

- (NSString*)altEntry
{
	return [_altData stringValue];
}


- (NSString*)widthEntry
{
	return [_widthData stringValue];
}

- (void)setWidthEntry:(NSString*)entry
{
	[_widthData setStringValue:entry];
}

- (int)widthNum
{
	return [_widthNumData intValue];
}


- (NSString*)heightEntry
{
	return [_heightData stringValue];
}

- (void)setHeightEntry:(NSString*)entry
{
	[_heightData setStringValue:entry];
}


- (int)heightNum
{
	return [_heightNumData intValue];
}

- (BOOL)preserveRatio
{
	return ([_preserveAspectBtn state] == NSOnState ? YES : NO);
}

- (void)setPreserveRatio:(BOOL)val
{
	[_preserveAspectBtn setState:(val?NSOnState:NSOffState)];
}

@end
