/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
	setDescription ([[m_dlg altEntry] UTF8String]);
	setWrapping([m_dlg textWrap]);
	setPositionTo([m_dlg imagePlacement]);
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

void XAP_CocoaDialog_Image::doHeightSpin(bool bIncrement)
{
	incrementHeight(bIncrement);
	adjustWidthForAspect();
	[m_dlg setHeightEntry:[NSString stringWithUTF8String:getHeightString()]];
}

void XAP_CocoaDialog_Image::doHeightEntry(void)
{
	const char * szHeight = [[m_dlg heightEntry] UTF8String];

	if(UT_determineDimension(szHeight, DIM_none) != DIM_none) {
		setHeight(szHeight);
	}
	adjustWidthForAspect();

	[m_dlg setHeightEntry:[NSString stringWithUTF8String:getHeightString()]];
}

void XAP_CocoaDialog_Image::setHeightEntry(void)
{
	[m_dlg setHeightEntry:[NSString stringWithUTF8String:getHeightString()]];
}

void XAP_CocoaDialog_Image::doWidthSpin(bool bIncrement)
{
	incrementWidth(bIncrement);
	adjustHeightForAspect();
	[m_dlg setWidthEntry:[NSString stringWithUTF8String:getWidthString()]];
}

void XAP_CocoaDialog_Image::setWidthEntry(void)
{
	[m_dlg setWidthEntry:[NSString stringWithUTF8String:getWidthString()]];
}

void XAP_CocoaDialog_Image::doWidthEntry(void)
{
	const char * szWidth = [[m_dlg widthEntry] UTF8String];

	if(UT_determineDimension(szWidth, DIM_none) != DIM_none) {
		setWidth(szWidth);
	}
	adjustHeightForAspect();

	[m_dlg setWidthEntry:[NSString stringWithUTF8String:getWidthString()]];
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

		LocalizeControl([self window],		pSS, XAP_STRING_ID_DLG_Image_Title);
		LocalizeControl(_okBtn,				pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn,			pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_titleCell,			pSS, XAP_STRING_ID_DLG_Image_LblTitle);
		LocalizeControl(_descriptionCell,	pSS, XAP_STRING_ID_DLG_Image_LblDescription);
		LocalizeControl(_widthCell,			pSS, XAP_STRING_ID_DLG_Image_Width);
		LocalizeControl(_heightCell,		pSS, XAP_STRING_ID_DLG_Image_Height);
		LocalizeControl(_preserveAspectBtn,	pSS, XAP_STRING_ID_DLG_Image_Aspect);
		LocalizeControl(_textWrapLabel,		pSS, XAP_STRING_ID_DLG_Image_TextWrapping);
		LocalizeControl(_textWrapInline,	pSS, XAP_STRING_ID_DLG_Image_InLine);
		LocalizeControl(_textWrapRight,		pSS, XAP_STRING_ID_DLG_Image_WrappedRight);
		LocalizeControl(_textWrapLeft,		pSS, XAP_STRING_ID_DLG_Image_WrappedLeft);
		LocalizeControl(_textWrapBoth,		pSS, XAP_STRING_ID_DLG_Image_WrappedBoth);
		LocalizeControl(_imagePlaceLabel,	pSS, XAP_STRING_ID_DLG_Image_Placement);
		LocalizeControl(_imagePlaceNearest,	pSS, XAP_STRING_ID_DLG_Image_PlaceParagraph);
		LocalizeControl(_imagePlaceColumn,	pSS, XAP_STRING_ID_DLG_Image_PlaceColumn);
		LocalizeControl(_imagePlacePage,	pSS, XAP_STRING_ID_DLG_Image_PlacePage);

		[_titleCell       setStringValue:[NSString stringWithUTF8String:_xap->getTitle().utf8_str()]];
		[_descriptionCell setStringValue:[NSString stringWithUTF8String:_xap->getDescription().utf8_str()]];

		[_preserveAspectBtn setState:(_xap->getPreserveAspect() ? NSOnState : NSOffState)];

		[ _widthNumStepper setIntValue:1];
		[_heightNumStepper setIntValue:1];

		[self setTextWrap:(_xap->getWrapping()) isEnabled:YES];

		[self setImagePlacement:(_xap->getPositionTo()) isEnabled:((_xap->getWrapping() == WRAP_INLINE) ? NO : YES)];
	}
}

- (IBAction)okAction:(id)sender
{
	_xap->event_Ok();
}

- (IBAction)cancelAction:(id)sender
{
	_xap->event_Cancel();
}


- (IBAction)widthChanged:(id)sender
{
	_xap->doWidthEntry();
}

- (IBAction)widthNumStepperChanged:(id)sender
{
	bool bIncrement = ([_widthNumStepper intValue] > 1);
	[_widthNumStepper setIntValue:1];
	_xap->doWidthSpin(bIncrement);
}

- (IBAction)heightChanged:(id)sender
{
	_xap->doHeightEntry();
}

- (IBAction)heightNumStepperChanged:(id)sender
{
	bool bIncrement = ([_heightNumStepper intValue] > 1);
	[_heightNumStepper setIntValue:1];
	_xap->doHeightSpin(bIncrement);
}

- (IBAction)preserveAction:(id)sender
{
	_xap->aspectCheckbox();
}


- (NSString*)titleEntry
{
	return [_titleCell stringValue];
}

- (NSString*)altEntry
{
	return [_descriptionCell stringValue];
}


- (NSString*)widthEntry
{
	return [_widthCell stringValue];
}

- (void)setWidthEntry:(NSString*)entry
{
	[_widthCell setStringValue:entry];
}

- (NSString*)heightEntry
{
	return [_heightCell stringValue];
}

- (void)setHeightEntry:(NSString*)entry
{
	[_heightCell setStringValue:entry];
}

- (BOOL)preserveRatio
{
	return ([_preserveAspectBtn state] == NSOnState ? YES : NO);
}

- (void)setPreserveRatio:(BOOL)val
{
	[_preserveAspectBtn setState:(val?NSOnState:NSOffState)];
}

- (IBAction)wrapAction:(id)sender
{
	BOOL bEnabled = (([self textWrap] == WRAP_INLINE) ? NO : YES);

	[_imagePlaceLabel  setEnabled:bEnabled];
	[_imagePlaceMatrix setEnabled:bEnabled];
}

- (void)setTextWrap:(WRAPPING_TYPE)textWrap isEnabled:(BOOL)enabled
{
	switch (textWrap)
		{
		case WRAP_INLINE:
			[_textWrapMatrix selectCellAtRow:0 column:0];
			break;
		case WRAP_TEXTRIGHT:
			[_textWrapMatrix selectCellAtRow:1 column:0];
			break;
		case WRAP_TEXTLEFT:
			[_textWrapMatrix selectCellAtRow:2 column:0];
			break;
		case WRAP_TEXTBOTH:
			[_textWrapMatrix selectCellAtRow:3 column:0];
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
	[_textWrapLabel  setEnabled:enabled];
	[_textWrapMatrix setEnabled:enabled];
}

- (WRAPPING_TYPE)textWrap
{
	WRAPPING_TYPE type = WRAP_INLINE;

	switch ([_textWrapMatrix selectedRow])
		{
		case 0:
			type = WRAP_INLINE;
			break;
		case 1:
			type = WRAP_TEXTRIGHT;
			break;
		case 2:
			type = WRAP_TEXTLEFT;
			break;
		case 3:
			type = WRAP_TEXTBOTH;
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
	return type;
}

- (void)setImagePlacement:(POSITION_TO)imagePlacement isEnabled:(BOOL)enabled
{
	switch (imagePlacement)
		{
		case POSITION_TO_PARAGRAPH:
			[_imagePlaceMatrix selectCellAtRow:0 column:0];
			break;
		case POSITION_TO_COLUMN:
			[_imagePlaceMatrix selectCellAtRow:1 column:0];
			break;
		case POSITION_TO_PAGE:
			[_imagePlaceMatrix selectCellAtRow:2 column:0];
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
	[_imagePlaceLabel  setEnabled:enabled];
	[_imagePlaceMatrix setEnabled:enabled];
}

- (POSITION_TO)imagePlacement
{
	POSITION_TO type = POSITION_TO_PARAGRAPH;

	switch ([_imagePlaceMatrix selectedRow])
		{
		case 0:
			type = POSITION_TO_PARAGRAPH;
			break;
		case 1:
			type = POSITION_TO_COLUMN;
			break;
		case 2:
			type = POSITION_TO_PAGE;
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
	return type;
}

@end
