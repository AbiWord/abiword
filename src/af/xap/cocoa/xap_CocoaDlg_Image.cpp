/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz
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



@interface XAP_CocoaDialog_ImageController : NSWindowController <XAP_CocoaDialogProtocol>
{
@public
	IBOutlet NSTabView *        _mainTab;
	IBOutlet NSBox *            _nameBox;
	IBOutlet NSFormCell *		_titleCell;
	IBOutlet NSFormCell *		_descriptionCell;

	IBOutlet NSBox *            _imageSizeBox;
	IBOutlet NSFormCell *		_widthCell;
	IBOutlet NSFormCell *		_heightCell;

	IBOutlet NSStepper *		_widthNumStepper;
	IBOutlet NSStepper *		_heightNumStepper;

	IBOutlet NSButton *			_preserveAspectBtn;

	IBOutlet NSBox *		_textWrapLabel;
	IBOutlet NSMatrix *			_textWrapMatrix;
	IBOutlet NSButtonCell *		_textWrapInline;
	IBOutlet NSButtonCell *		_textWrapFloat;
	IBOutlet NSButtonCell *		_textWrapRight;
	IBOutlet NSButtonCell *		_textWrapLeft;
	IBOutlet NSButtonCell *		_textWrapBoth;

	IBOutlet NSBox *		_imagePlaceLabel;
	IBOutlet NSMatrix *			_imagePlaceMatrix;
	IBOutlet NSButtonCell *		_imagePlaceNearest;
	IBOutlet NSButtonCell *		_imagePlaceColumn;
	IBOutlet NSButtonCell *		_imagePlacePage;

	IBOutlet NSBox *            _typeTextWrapBox;
	IBOutlet NSMatrix *         _typeTextWrapMatrix;
	IBOutlet NSButtonCell *     _typeTextWrapSquare;
	IBOutlet NSButtonCell *     _typeTextWrapTight;

	IBOutlet NSButton *			_cancelBtn;
	IBOutlet NSButton *			_okBtn;

	XAP_CocoaDialog_Image *		_xap;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)heightChanged:(id)sender;
- (IBAction)heightNumStepperChanged:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)preserveAction:(id)sender;
- (IBAction)widthChanged:(id)sender;
- (IBAction)widthNumStepperChanged:(id)sender;
- (IBAction)wrapAction:(id)sender;

- (NSString*)titleEntry;
- (NSString*)altEntry;
- (NSString*)widthEntry;
- (void)setWidthEntry:(NSString*)entry;
- (NSString*)heightEntry;
- (void)setHeightEntry:(NSString*)entry;
- (BOOL)preserveRatio;
- (void)setPreserveRatio:(BOOL)val;

- (WRAPPING_TYPE)textWrap;
- (POSITION_TO)imagePlacement;
- (bool)tightWrap;
@end

/*****************************************************************/

void XAP_CocoaDialog_Image::event_Ok ()
{
	setAnswer(XAP_Dialog_Image::a_OK);
	setTitle ([[m_dlg titleEntry] UTF8String]);
	setDescription ([[m_dlg altEntry] UTF8String]);
	setWrapping([m_dlg textWrap]);
	setPositionTo([m_dlg imagePlacement]);
	setTightWrap([m_dlg tightWrap]);
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


void XAP_CocoaDialog_Image::wrappingChanged(void)
{
	if([m_dlg->_textWrapMatrix selectedCell] == m_dlg->_textWrapInline)
	{
		[m_dlg->_imagePlaceMatrix selectCell:m_dlg->_imagePlaceNearest];
		[m_dlg->_imagePlaceMatrix setEnabled:NO];
		[m_dlg->_typeTextWrapMatrix setEnabled:NO];
		return;
	}
	[m_dlg->_imagePlaceMatrix setEnabled:YES];
	[m_dlg->_textWrapMatrix setEnabled:YES];
	[m_dlg->_typeTextWrapMatrix setEnabled:YES];
}

void XAP_CocoaDialog_Image::setWrappingGUI()
{
	if(isInHdrFtr() || (getWrapping() == WRAP_INLINE))
	{
		[m_dlg->_textWrapInline setState:NSOnState];
		[m_dlg->_typeTextWrapMatrix setEnabled:NO];
	}
	else if(getWrapping() == WRAP_TEXTRIGHT)
	{
		[m_dlg->_textWrapRight setState:NSOnState];
		[m_dlg->_typeTextWrapMatrix setEnabled:YES];
	}
	else if(getWrapping() == WRAP_NONE)
	{
		[m_dlg->_textWrapFloat setState:NSOnState];
		[m_dlg->_typeTextWrapMatrix setEnabled:NO];
	}
	else if(getWrapping() == WRAP_TEXTLEFT)
	{
		[m_dlg->_textWrapLeft setState:NSOnState];
		[m_dlg->_typeTextWrapMatrix setEnabled:YES];
	}
	else if(getWrapping() == WRAP_TEXTBOTH)
	{
		[m_dlg->_textWrapBoth setState:NSOnState];
		[m_dlg->_typeTextWrapMatrix setEnabled:YES];
	}
	if(isInHdrFtr())
	{
		[m_dlg->_textWrapRight setEnabled:NO];
		[m_dlg->_textWrapLeft setEnabled:NO];
		[m_dlg->_textWrapBoth setEnabled:NO];
		[m_dlg->_typeTextWrapMatrix setEnabled:NO];
	}
	else if(isTightWrap())
	{
		[m_dlg->_typeTextWrapTight setState:NSOnState];
	}
	else if(!isTightWrap())
	{
		[m_dlg->_typeTextWrapSquare setState:NSOnState];
	}
}


void XAP_CocoaDialog_Image::setPositionToGUI()
{
	if(!isInHdrFtr())
	{
		if(getPositionTo() == POSITION_TO_PARAGRAPH)
		{
			[m_dlg->_imagePlaceNearest setState:NSOnState];
		}
		else if(getPositionTo() == POSITION_TO_COLUMN)
		{
			[m_dlg->_imagePlaceColumn setState:NSOnState];
		}
		else if(getPositionTo() == POSITION_TO_PAGE)
		{
			[m_dlg->_imagePlacePage setState:NSOnState];
		}
	}
	else
	{
		[m_dlg->_imagePlaceMatrix setEnabled:NO];
		[m_dlg->_typeTextWrapMatrix setEnabled:NO];
		[m_dlg->_textWrapMatrix setEnabled:NO];
	}
}

void XAP_CocoaDialog_Image::runModal(XAP_Frame * /*pFrame*/)
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

	setWrappingGUI();
	setPositionToGUI();
	wrappingChanged();

	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}



@implementation XAP_CocoaDialog_ImageController

- (id)initFromNib
{
	if(![super initWithWindowNibName:@"xap_CocoaDlg_Image"]) {
		return nil;
	}
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

		LocalizeControl([_mainTab tabViewItemAtIndex:0], pSS, XAP_STRING_ID_DLG_Image_DescTabLabel);
		LocalizeControl([_mainTab tabViewItemAtIndex:1], pSS, XAP_STRING_ID_DLG_Image_WrapTabLabel);

		LocalizeControl(_nameBox,			pSS, XAP_STRING_ID_DLG_Image_ImageDesc);
		LocalizeControl(_titleCell,			pSS, XAP_STRING_ID_DLG_Image_LblTitle);
		LocalizeControl(_descriptionCell,	pSS, XAP_STRING_ID_DLG_Image_LblDescription);
		LocalizeControl(_imageSizeBox,		pSS, XAP_STRING_ID_DLG_Image_ImageSize);
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
		
		LocalizeControl(_typeTextWrapBox,		pSS, XAP_STRING_ID_DLG_Image_WrapType);
		LocalizeControl(_typeTextWrapSquare,	pSS, XAP_STRING_ID_DLG_Image_SquareWrap);
		LocalizeControl(_typeTextWrapTight,		pSS, XAP_STRING_ID_DLG_Image_TightWrap);
		
		[_titleCell       setStringValue:[NSString stringWithUTF8String:_xap->getTitle().utf8_str()]];
		[_descriptionCell setStringValue:[NSString stringWithUTF8String:_xap->getDescription().utf8_str()]];

		[_preserveAspectBtn setState:(_xap->getPreserveAspect() ? NSOnState : NSOffState)];

		[ _widthNumStepper setIntValue:1];
		[_heightNumStepper setIntValue:1];
	}
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Ok();
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel();
}


- (IBAction)widthChanged:(id)sender
{
	UT_UNUSED(sender);
	_xap->doWidthEntry();
}

- (IBAction)widthNumStepperChanged:(id)sender
{
	UT_UNUSED(sender);
	bool bIncrement = ([_widthNumStepper intValue] > 1);
	[_widthNumStepper setIntValue:1];
	_xap->doWidthSpin(bIncrement);
}

- (IBAction)heightChanged:(id)sender
{
	UT_UNUSED(sender);
	_xap->doHeightEntry();
}

- (IBAction)heightNumStepperChanged:(id)sender
{
	UT_UNUSED(sender);
	bool bIncrement = ([_heightNumStepper intValue] > 1);
	[_heightNumStepper setIntValue:1];
	_xap->doHeightSpin(bIncrement);
}

- (IBAction)preserveAction:(id)sender
{
	UT_UNUSED(sender);
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
	UT_UNUSED(sender);
	_xap->wrappingChanged();
}


- (WRAPPING_TYPE)textWrap
{
	WRAPPING_TYPE type = WRAP_INLINE;

	NSCell *ctrl = [_textWrapMatrix selectedCell];

	if(ctrl == _textWrapInline) {
		type = WRAP_INLINE;
	}
	else if(ctrl == _textWrapFloat) {
		type = WRAP_NONE;
	}
	else if(ctrl == _textWrapRight) {
		type = WRAP_TEXTRIGHT;
	}
	else if(ctrl == _textWrapLeft) {
		type = WRAP_TEXTLEFT;
	}
	else if(ctrl == _textWrapBoth) {
	   type = WRAP_TEXTBOTH;
	}
	return type;
}


- (POSITION_TO)imagePlacement
{
	POSITION_TO type = POSITION_TO_PARAGRAPH;
	NSCell *ctrl = [_imagePlaceMatrix selectedCell];

	if(ctrl == _imagePlaceNearest) {
		type = POSITION_TO_PARAGRAPH;
	}
	else if(ctrl == _imagePlaceColumn) {
		type = POSITION_TO_COLUMN;
	}
	else if(ctrl == _imagePlacePage) {
		type = POSITION_TO_PAGE;
	}
	return type;
}

- (bool)tightWrap
{
	NSCell *ctrl = [_typeTextWrapMatrix selectedCell];
	return (ctrl == _typeTextWrapTight);
}

@end
