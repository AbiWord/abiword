/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
 * Copyright (C) 2003-2016 Hubert Figuiere
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "gr_CocoaGraphics.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_Frame.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaFrame.h"
#include "xap_CocoaToolbar_Icons.h"

#include "fl_TableLayout.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_FormatFrame.h"
#include "ap_CocoaDialog_FormatFrame.h"
#include "ap_CocoaDialog_Columns.h"


/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_FormatFrame::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_FormatFrame * p = new AP_CocoaDialog_FormatFrame(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_FormatFrame::AP_CocoaDialog_FormatFrame(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	AP_Dialog_FormatFrame(pDlgFactory,dlgid),
	m_pPreviewWidget(NULL),
	m_dlg(nil)
{
	// 
}

AP_CocoaDialog_FormatFrame::~AP_CocoaDialog_FormatFrame(void)
{
	// 
}

void AP_CocoaDialog_FormatFrame::runModeless(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[AP_CocoaDialog_FormatFrameController alloc] initFromNib];

	[m_dlg setXAPOwner:this];
	[m_dlg window];

	XAP_CocoaNSView* view = m_dlg.preview;
	UT_ASSERT([view isKindOfClass:[XAP_CocoaNSView class]]);

	NSSize size = view.bounds.size;

	UT_uint32 width  = static_cast<UT_uint32>(lrintf(size.width));
	UT_uint32 height = static_cast<UT_uint32>(lrintf(size.height));

	/* make a new Cocoa GC
	 */
	DELETEP(m_pPreviewWidget);
	GR_CocoaAllocInfo ai((XAP_CocoaNSView*)view);
	m_pPreviewWidget = (GR_CocoaGraphics *) m_pApp->newGraphics(ai);

	/* TODO: we need a good widget to query with a probable non-white (i.e. gray, or
	 *       a similar bgcolor as our parent widget) background. This should be fine.
	 */
	m_pPreviewWidget->init3dColors();

	/* let the widget materialize
	 */
	_createPreviewFromGC(m_pPreviewWidget, width, height);
	view.drawable = m_pFormatFramePreview;

	m_pFormatFramePreview->queueDraw();

	activate();
	startUpdater();
}

void AP_CocoaDialog_FormatFrame::setSensitivity(bool bSens)
{
	[m_dlg setSensitivity:bSens];
}

void AP_CocoaDialog_FormatFrame::setBorderThicknessInGUI(UT_UTF8String & /*sThick*/)
{
	UT_ASSERT_NOT_REACHED();
}

void AP_CocoaDialog_FormatFrame::event_Close(void)
{
	m_answer = AP_Dialog_FormatFrame::a_CLOSE;
	destroy();
}

void AP_CocoaDialog_FormatFrame::event_previewInvalidate(void)
{
	if(m_pFormatFramePreview) {
		m_pFormatFramePreview->queueDraw();
	}
}

void AP_CocoaDialog_FormatFrame::event_ApplyToChanged(void)
{
#if 0
	switch ([m_dlg applyItemTag])
	{
	case 0:
		setApplyFormatTo(FORMAT_TABLE_SELECTION);
		break;
	case 1:
		setApplyFormatTo(FORMAT_TABLE_ROW);
		break;
	case 2:
		setApplyFormatTo(FORMAT_TABLE_COLUMN);
		break;
	case 3:
		setApplyFormatTo(FORMAT_TABLE_TABLE);
		break;
	default:
		// should not happen
		break;
	}
#endif
}

void AP_CocoaDialog_FormatFrame::destroy(void)
{
	finalize();

	if (m_dlg) {
		[m_dlg close];
		[m_dlg release];
		m_dlg = nil;
	}
	DELETEP(m_pPreviewWidget);
}

void AP_CocoaDialog_FormatFrame::activate(void)
{
#if 0
	ConstructWindowName();

	NSWindow * window = [m_dlg window];

	[window setTitle:[NSString stringWithUTF8String:getWindowName()]];
#endif
	[[m_dlg window] makeKeyAndOrderFront:m_dlg];

	/* Populate the window's data items
	 */
	_populateWindowData();
}

void AP_CocoaDialog_FormatFrame::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
#if 0
	ConstructWindowName();

	NSWindow * window = [m_dlg window];

	[window setTitle:[NSString stringWithUTF8String:getWindowName()]];
#endif
	/* Populate the window's data items
	 */
	_populateWindowData();
}

void AP_CocoaDialog_FormatFrame::_populateWindowData(void)
{
	if (m_dlg) {
		float bcr = static_cast<float>(m_borderColor.m_red) / 255.0f;
		float bcg = static_cast<float>(m_borderColor.m_grn) / 255.0f;
		float bcb = static_cast<float>(m_borderColor.m_blu) / 255.0f;

		float bca = m_borderColor.m_bIsTransparent ? 0.0f : 1.0f;

		[m_dlg setBorderColor:[NSColor colorWithCalibratedRed:bcr green:bcg blue:bcb alpha:bca]];

		int bsr = getRightToggled()  ? NSControlStateValueOn : NSControlStateValueOff;
		int bsl = getLeftToggled()   ? NSControlStateValueOn : NSControlStateValueOff;
		int bst = getTopToggled()    ? NSControlStateValueOn : NSControlStateValueOff;
		int bsb = getBottomToggled() ? NSControlStateValueOn : NSControlStateValueOff;

		bcr = static_cast<float>(borderColorRight().m_red) / 255.0f;
		bcg = static_cast<float>(borderColorRight().m_grn) / 255.0f;
		bcb = static_cast<float>(borderColorRight().m_blu) / 255.0f;

		bca = borderColorRight().m_bIsTransparent ? 0.0f : 1.0f;

		NSString * thickness = 0;

		float value;

		thickness = [NSString stringWithUTF8String:(getBorderThicknessRight().utf8_str())];

		value = borderThicknessRight();

		[m_dlg  setRightBorderState:bsr borderColor:[NSColor colorWithCalibratedRed:bcr green:bcg blue:bcb alpha:bca] borderThickness:thickness stepperValue:value];

		bcr = static_cast<float>(borderColorLeft().m_red) / 255.0f;
		bcg = static_cast<float>(borderColorLeft().m_grn) / 255.0f;
		bcb = static_cast<float>(borderColorLeft().m_blu) / 255.0f;

		bca = borderColorLeft().m_bIsTransparent ? 0.0f : 1.0f;

		thickness = [NSString stringWithUTF8String:(getBorderThicknessLeft().utf8_str())];

		value = borderThicknessLeft();

		[m_dlg   setLeftBorderState:bsl borderColor:[NSColor colorWithCalibratedRed:bcr green:bcg blue:bcb alpha:bca] borderThickness:thickness stepperValue:value];

		bcr = static_cast<float>(borderColorTop().m_red) / 255.0f;
		bcg = static_cast<float>(borderColorTop().m_grn) / 255.0f;
		bcb = static_cast<float>(borderColorTop().m_blu) / 255.0f;

		bca = borderColorTop().m_bIsTransparent ? 0.0f : 1.0f;

		thickness = [NSString stringWithUTF8String:(getBorderThicknessTop().utf8_str())];

		value = borderThicknessTop();

		[m_dlg    setTopBorderState:bst borderColor:[NSColor colorWithCalibratedRed:bcr green:bcg blue:bcb alpha:bca] borderThickness:thickness stepperValue:value];

		bcr = static_cast<float>(borderColorBottom().m_red) / 255.0f;
		bcg = static_cast<float>(borderColorBottom().m_grn) / 255.0f;
		bcb = static_cast<float>(borderColorBottom().m_blu) / 255.0f;

		bca = borderColorBottom().m_bIsTransparent ? 0.0f : 1.0f;

		thickness = [NSString stringWithUTF8String:(getBorderThicknessBottom().utf8_str())];

		value = borderThicknessBottom();

		[m_dlg setBottomBorderState:bsb borderColor:[NSColor colorWithCalibratedRed:bcr green:bcg blue:bcb alpha:bca] borderThickness:thickness stepperValue:value];

		bcr = static_cast<float>(backgroundColor().m_red) / 255.0f;
		bcg = static_cast<float>(backgroundColor().m_grn) / 255.0f;
		bcb = static_cast<float>(backgroundColor().m_blu) / 255.0f;

		bca = backgroundColor().m_bIsTransparent ? 0.0f : 1.0f;

		[m_dlg setBackgroundColor:[NSColor colorWithCalibratedRed:bcr green:bcg blue:bcb alpha:bca]];

		[m_dlg setWrapState:(getWrapping() ? NSControlStateValueOn : NSControlStateValueOff)];

		[m_dlg setPositionState:((int) positionMode())];

		setAllSensitivities();
	}
}

void AP_CocoaDialog_FormatFrame::_storeWindowData(void)
{
	// 
}

/*****************************************************************/

@implementation AP_CocoaDialog_FormatFrameController

- (id)initFromNib
{
	if (self = [super initWithWindowNibName:@"ap_CocoaDialog_FormatFrame"]) {
		m_menuButtonTag = 0;
		m_activeMenuTag = 0;

		m_bEnabled = YES;

		_xap = 0;
	}
	return self;
}

- (void)dealloc
{
	// 
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = static_cast<AP_CocoaDialog_FormatFrame *>(owner);
}

- (void)discardXAP
{
	_xap = NULL; 
}

- (void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		LocalizeControl([self window],		pSS, AP_STRING_ID_DLG_FormatFrameTitle);

		LocalizeControl(_bgColorLabel,		pSS, AP_STRING_ID_DLG_FormatFrame_Color);
		LocalizeControl(_borderBox,			pSS, AP_STRING_ID_DLG_FormatFrame_Borders);
		LocalizeControl(_borderColorLabel,	pSS, AP_STRING_ID_DLG_FormatFrame_Color);
		LocalizeControl(_setImageBtn,		pSS, AP_STRING_ID_DLG_FormatFrame_SelectImage);

		LocalizeControl(_wrapSwitch,		pSS, AP_STRING_ID_DLG_FormatFrame_TextWrapping);

		[_setImageBtn setImage:[NSImage imageNamed:@"tb_insert_graphic"]];

		LocalizeControl(_noImageBtn,		pSS, AP_STRING_ID_DLG_FormatFrame_NoImageBackground);

		[_noImageBtn setImage:[NSImage imageNamed:@"tb_remove_graphic"]];

		LocalizeControl(_previewBox,		pSS, AP_STRING_ID_DLG_FormatFrame_Preview);

		[_positionPopUp removeAllItems];

		AppendLocalizedMenuItem(_positionPopUp, pSS, AP_STRING_ID_DLG_FormatFrame_SetToParagraph, (int) FL_FRAME_POSITIONED_TO_BLOCK);
		AppendLocalizedMenuItem(_positionPopUp, pSS, AP_STRING_ID_DLG_FormatFrame_SetToColumn,    (int) FL_FRAME_POSITIONED_TO_COLUMN);
		AppendLocalizedMenuItem(_positionPopUp, pSS, AP_STRING_ID_DLG_FormatFrame_SetToPage,      (int) FL_FRAME_POSITIONED_TO_PAGE);

		[[_positionPopUp menu] setAutoenablesItems:NO];

		[[_positionPopUp itemAtIndex:0] setState:NSControlStateValueOff];
		[[_positionPopUp itemAtIndex:1] setState:NSControlStateValueOff];
		[[_positionPopUp itemAtIndex:2] setState:NSControlStateValueOff];

		[_rightBorderBtn setImage:[NSImage imageNamed:@"tb_LineRight"]];
		[_topBorderBtn setImage:[NSImage imageNamed:@"tb_LineTop"]];
		[_leftBorderBtn setImage:[NSImage imageNamed:@"tb_LineLeft"]];
		[_bottomBorderBtn setImage:[NSImage imageNamed:@"tb_LineBottom"]];

		[_rightBorderBtn setMenu:_linestyleMenu withController:self];
		[_topBorderBtn setMenu:_linestyleMenu withController:self];
		[_leftBorderBtn setMenu:_linestyleMenu withController:self];
		[_bottomBorderBtn setMenu:_linestyleMenu withController:self];

		[[self window] makeFirstResponder:_borderNumberForm];
	}
}

- (IBAction)borderThicknessField:(id)sender
{
	NSStepper * stepper = 0;
	NSFormCell * field = 0;
	float thickness = 0;
	UT_UTF8String sThick;

	switch ([sender tag])
		{
		case 1:
			stepper   = _rightBorderStepper;
			field     = _rightBorderNumber;
			sThick    = [[field stringValue] UTF8String];
			_xap->setBorderThicknessRight(sThick);
			thickness = _xap->borderThicknessRight();
			[stepper setFloatValue:thickness];
			[field   setStringValue:[NSString stringWithUTF8String:(_xap->getBorderThicknessRight().utf8_str())]];
			break;

		case 2:
			stepper   = _bottomBorderStepper;
			field     = _bottomBorderNumber;
			sThick    = [[field stringValue] UTF8String];
			_xap->setBorderThicknessBottom(sThick);
			thickness = _xap->borderThicknessBottom();
			[stepper setFloatValue:thickness];
			[field   setStringValue:[NSString stringWithUTF8String:(_xap->getBorderThicknessBottom().utf8_str())]];
			break;

		case 3:
			stepper   = _leftBorderStepper;
			field     = _leftBorderNumber;
			sThick    = [[field stringValue] UTF8String];
			_xap->setBorderThicknessLeft(sThick);
			thickness = _xap->borderThicknessLeft();
			[stepper setFloatValue:thickness];
			[field   setStringValue:[NSString stringWithUTF8String:(_xap->getBorderThicknessLeft().utf8_str())]];
			break;

		case 4:
			stepper   = _topBorderStepper;
			field     = _topBorderNumber;
			sThick    = [[field stringValue] UTF8String];
			_xap->setBorderThicknessTop(sThick);
			thickness = _xap->borderThicknessTop();
			[stepper setFloatValue:thickness];
			[field   setStringValue:[NSString stringWithUTF8String:(_xap->getBorderThicknessTop().utf8_str())]];
			break;

		default:
			stepper   = _borderStepper;
			field     = _borderNumber;
			sThick    = [[field stringValue] UTF8String];

			_xap->setBorderThicknessRight(sThick);
			_xap->setBorderThicknessLeft(sThick);
			_xap->setBorderThicknessTop(sThick);
			_xap->setBorderThicknessBottom(sThick);

			thickness = _xap->borderThicknessRight();
			[stepper setFloatValue:thickness];
			[field   setStringValue:[NSString stringWithUTF8String:(_xap->getBorderThicknessRight().utf8_str())]];

			[ _rightBorderNumber setStringValue:[field stringValue]];
			[  _leftBorderNumber setStringValue:[field stringValue]];
			[   _topBorderNumber setStringValue:[field stringValue]];
			[_bottomBorderNumber setStringValue:[field stringValue]];

			[ _rightBorderStepper setFloatValue:[stepper floatValue]];
			[  _leftBorderStepper setFloatValue:[stepper floatValue]];
			[   _topBorderStepper setFloatValue:[stepper floatValue]];
			[_bottomBorderStepper setFloatValue:[stepper floatValue]];

			break;
		}
	_xap->event_previewInvalidate();
}

- (IBAction)borderThicknessStepper:(id)sender
{
	NSStepper * stepper = 0;
	NSFormCell * field = 0;
	float thickness = 0;

	switch ([sender tag])
		{
		case 1:
			stepper   = _rightBorderStepper;
			field     = _rightBorderNumber;
			thickness = [stepper floatValue];
			_xap->setBorderThicknessRight(thickness);
			thickness = _xap->borderThicknessRight();
			[stepper setFloatValue:thickness];
			[field   setStringValue:[NSString stringWithUTF8String:(_xap->getBorderThicknessRight().utf8_str())]];
			break;

		case 2:
			stepper   = _bottomBorderStepper;
			field     = _bottomBorderNumber;
			thickness = [stepper floatValue];
			_xap->setBorderThicknessBottom(thickness);
			thickness = _xap->borderThicknessBottom();
			[stepper setFloatValue:thickness];
			[field   setStringValue:[NSString stringWithUTF8String:(_xap->getBorderThicknessBottom().utf8_str())]];
			break;

		case 3:
			stepper   = _leftBorderStepper;
			field     = _leftBorderNumber;
			thickness = [stepper floatValue];
			_xap->setBorderThicknessLeft(thickness);
			thickness = _xap->borderThicknessLeft();
			[stepper setFloatValue:thickness];
			[field   setStringValue:[NSString stringWithUTF8String:(_xap->getBorderThicknessLeft().utf8_str())]];
			break;

		case 4:
			stepper   = _topBorderStepper;
			field     = _topBorderNumber;
			thickness = [stepper floatValue];
			_xap->setBorderThicknessTop(thickness);
			thickness = _xap->borderThicknessTop();
			[stepper setFloatValue:thickness];
			[field   setStringValue:[NSString stringWithUTF8String:(_xap->getBorderThicknessTop().utf8_str())]];
			break;

		default:
			stepper   = _borderStepper;
			field     = _borderNumber;
			thickness = [stepper floatValue];

			_xap->setBorderThicknessRight(thickness);
			_xap->setBorderThicknessLeft(thickness);
			_xap->setBorderThicknessTop(thickness);
			_xap->setBorderThicknessBottom(thickness);

			thickness = _xap->borderThicknessRight();
			[stepper setFloatValue:thickness];
			[field   setStringValue:[NSString stringWithUTF8String:(_xap->getBorderThicknessRight().utf8_str())]];

			[ _rightBorderNumber setStringValue:[field stringValue]];
			[  _leftBorderNumber setStringValue:[field stringValue]];
			[   _topBorderNumber setStringValue:[field stringValue]];
			[_bottomBorderNumber setStringValue:[field stringValue]];

			[ _rightBorderStepper setFloatValue:[stepper floatValue]];
			[  _leftBorderStepper setFloatValue:[stepper floatValue]];
			[   _topBorderStepper setFloatValue:[stepper floatValue]];
			[_bottomBorderStepper setFloatValue:[stepper floatValue]];

			break;
		}
	_xap->event_previewInvalidate();
}

- (IBAction)bgColorAction:(id)sender
{
	UT_UNUSED(sender);
	NSColor * color = [_bgColorWell color];

	CGFloat red;
	CGFloat green;
	CGFloat blue;
	CGFloat alpha;

	[color getRed:&red green:&green blue:&blue alpha:&alpha]; // TODO: is color necessarily RGBA? if not, could be a problem...

	int r = static_cast<int>(lrintf(red   * 255));	r = (r < 0) ? 0 : r;	r = (r > 255) ? 255 : r;
	int g = static_cast<int>(lrintf(green * 255));	g = (g < 0) ? 0 : g;	g = (g > 255) ? 255 : g;
	int b = static_cast<int>(lrintf(blue  * 255));	b = (b < 0) ? 0 : b;	b = (b > 255) ? 255 : b;

	_xap->setBGColor(UT_RGBColor(static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b)));
	_xap->event_previewInvalidate();
}

- (IBAction)borderColorAction:(id)sender
{
	NSColor * color = 0;

	switch ([sender tag])
		{
		case 1:
			color = [_rightBorderColorWell color];
			break;
		case 2:
			color = [_bottomBorderColorWell color];
			break;
		case 3:
			color = [_leftBorderColorWell color];
			break;
		case 4:
			color = [_topBorderColorWell color];
			break;
		default:
			color = [_borderColorWell color];
			[ _rightBorderColorWell setColor:color];
			[  _leftBorderColorWell setColor:color];
			[   _topBorderColorWell setColor:color];
			[_bottomBorderColorWell setColor:color];
			break;
		}

	CGFloat red;
	CGFloat green;
	CGFloat blue;
	CGFloat alpha;

	[color getRed:&red green:&green blue:&blue alpha:&alpha]; // TODO: is color necessarily RGBA? if not, could be a problem...

	int r = static_cast<int>(lrintf(red   * 255));	r = (r < 0) ? 0 : r;	r = (r > 255) ? 255 : r;
	int g = static_cast<int>(lrintf(green * 255));	g = (g < 0) ? 0 : g;	g = (g > 255) ? 255 : g;
	int b = static_cast<int>(lrintf(blue  * 255));	b = (b < 0) ? 0 : b;	b = (b > 255) ? 255 : b;

	UT_RGBColor rgb(static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b));

	switch ([sender tag])
		{
		case 1:
			_xap->setBorderColorRight(rgb);
			break;
		case 2:
			_xap->setBorderColorBottom(rgb);
			break;
		case 3:
			_xap->setBorderColorLeft(rgb);
			break;
		case 4:
			_xap->setBorderColorTop(rgb);
			break;
		default:
			_xap->setBorderColorAll(rgb);
			break;
		}
	_xap->event_previewInvalidate();
}

- (IBAction)rightBorderAction:(id)sender
{
	_xap->setBorderLineStyleRight (([sender state] == NSControlStateValueOn) ? LS_NORMAL : LS_OFF);
	_xap->event_previewInvalidate();
}

- (IBAction)leftBorderAction:(id)sender
{
	_xap->setBorderLineStyleLeft (([sender state] == NSControlStateValueOn) ? LS_NORMAL : LS_OFF);
	_xap->event_previewInvalidate();
}

- (IBAction)topBorderAction:(id)sender
{
	_xap->setBorderLineStyleTop (([sender state] == NSControlStateValueOn) ? LS_NORMAL : LS_OFF);
	_xap->event_previewInvalidate();
}

- (IBAction)bottomBorderAction:(id)sender
{
	_xap->setBorderLineStyleBottom (([sender state] == NSControlStateValueOn) ? LS_NORMAL : LS_OFF);
	_xap->event_previewInvalidate();
}

- (IBAction)borderLineStyleAction:(id)sender
{
	UT_sint32 tag = static_cast<UT_sint32>([sender tag]);

	NSWindow * window = [self window];

	// [window makeKeyAndOrderFront:self];

	switch (m_menuButtonTag)
		{
		case 1:
			_xap->setBorderLineStyleRight(tag);
			[window makeFirstResponder:_rightBorderNumberForm];
			break;
		case 2:
			_xap->setBorderLineStyleBottom(tag);
			[window makeFirstResponder:_bottomBorderNumberForm];
			break;
		case 3:
			_xap->setBorderLineStyleLeft(tag);
			[window makeFirstResponder:_leftBorderNumberForm];
			break;
		case 4:
			_xap->setBorderLineStyleTop(tag);
			[window makeFirstResponder:_topBorderNumberForm];
			break;

		default:
			// should not happen
			break;
		}
	_xap->event_previewInvalidate();
}

- (void)menuWillActivate:(NSMenu *)menu forButton:(XAP_CocoaToolbarButton *)button
{
	UT_UNUSED(menu);
	m_menuButtonTag = [button tag];

	switch (m_menuButtonTag)
		{
		case 1:
			m_activeMenuTag = static_cast<int>(_xap->borderLineStyleRight());
			break;
		case 2:
			m_activeMenuTag = static_cast<int>(_xap->borderLineStyleBottom());
			break;
		case 3:
			m_activeMenuTag = static_cast<int>(_xap->borderLineStyleLeft());
			break;
		case 4:
			m_activeMenuTag = static_cast<int>(_xap->borderLineStyleTop());
			break;

		default:
			m_activeMenuTag = -1;
			break;
		}
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
	[menuItem setState:(([menuItem tag] == m_activeMenuTag) ? NSControlStateValueOn : NSControlStateValueOff)];

	return YES;
}

- (IBAction)selectImageAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->askForGraphicPathName();
}

- (IBAction)removeImageAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->clearImage();
}

- (IBAction)applyAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->applyChanges();
}

- (IBAction)applyToAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_ApplyToChanged();
}

- (XAP_CocoaNSView *)preview
{
	return _preview;
}

- (void)setSensitivity:(bool)bSens
{
	BOOL bEnabled = bSens ? YES : NO;

	if (m_bEnabled == bEnabled)
		return;

	m_bEnabled = bEnabled;

	[_wrapSwitch    setEnabled:bEnabled];
	[_positionPopUp setEnabled:bEnabled];
	[_applyBtn      setEnabled:bEnabled];

	[_bgColorWell   setEnabled:bEnabled];
	[_setImageBtn   setEnabled:bEnabled];
	[_noImageBtn    setEnabled:bEnabled];

	[_borderColorWell setEnabled:bEnabled];
	[_borderStepper   setEnabled:bEnabled];
	[_borderNumber    setEnabled:bEnabled];
	
	[_bgColorLabel setEnabled:bEnabled];
	[_borderColorLabel setEnabled:bEnabled];

	[ _rightBorderColorWell	setEnabled:bEnabled];
	[  _leftBorderColorWell	setEnabled:bEnabled];
	[   _topBorderColorWell	setEnabled:bEnabled];
	[_bottomBorderColorWell	setEnabled:bEnabled];

	[ _borderStepper setEnabled:bEnabled];
	[ _rightBorderStepper setEnabled:bEnabled];
	[  _leftBorderStepper setEnabled:bEnabled];
	[   _topBorderStepper setEnabled:bEnabled];
	[_bottomBorderStepper setEnabled:bEnabled];

	[ _borderNumberForm setEnabled:bEnabled];
	[ _rightBorderNumberForm setEnabled:bEnabled];
	[  _leftBorderNumberForm setEnabled:bEnabled];
	[   _topBorderNumberForm setEnabled:bEnabled];
	[_bottomBorderNumberForm setEnabled:bEnabled];

	[ _rightBorderBtn setEnabled:bEnabled];
	[  _leftBorderBtn setEnabled:bEnabled];
	[   _topBorderBtn setEnabled:bEnabled];
	[_bottomBorderBtn setEnabled:bEnabled];

	if (bEnabled)
	{
		NSWindow * window = [self window];

		// [window makeKeyAndOrderFront:self];
		[window makeFirstResponder:_borderNumberForm];
	}
}

- (void)setWrapState:(int)state
{
	[_wrapSwitch setState:state];
}

- (IBAction)wrapAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->setWrapping([_wrapSwitch state] == NSControlStateValueOn);
}

- (void)setPositionState:(int)state
{
	switch ((FL_FrameFormatMode) state) {
	default:
	case FL_FRAME_POSITIONED_TO_BLOCK:
		[_positionPopUp selectItemAtIndex:0];
		break;

	case FL_FRAME_POSITIONED_TO_COLUMN:
		[_positionPopUp selectItemAtIndex:1];
		break;

	case FL_FRAME_POSITIONED_TO_PAGE:
		[_positionPopUp selectItemAtIndex:2];
		break;
	}
	[_positionPopUp synchronizeTitleAndSelectedItem];
}

- (IBAction)positionAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->setPositionMode((FL_FrameFormatMode) [[_positionPopUp selectedItem] tag]);
}

- (void)setBorderColor:(NSColor *)color
{
	[_borderColorWell setColor:color];
}

- (void)setBackgroundColor:(NSColor *)color
{
	[_bgColorWell setColor:color];
}

- (void)setRightBorderState:(int)state borderColor:(NSColor *)color borderThickness:(NSString *)thickness stepperValue:(float)value
{
	state = NSControlStateValueOff;
	[_rightBorderBtn       setState:state];
	[_rightBorderColorWell setColor:color];
	[_rightBorderNumber    setStringValue:thickness];
	[_rightBorderStepper   setFloatValue:value];
}

- (void)setLeftBorderState:(int)state borderColor:(NSColor *)color borderThickness:(NSString *)thickness stepperValue:(float)value
{
	state = NSControlStateValueOff;
	[_leftBorderBtn       setState:state];
	[_leftBorderColorWell setColor:color];
	[_leftBorderNumber    setStringValue:thickness];
	[_leftBorderStepper   setFloatValue:value];
}

- (void)setTopBorderState:(int)state borderColor:(NSColor *)color borderThickness:(NSString *)thickness stepperValue:(float)value
{
	state = NSControlStateValueOff;
	[_topBorderBtn       setState:state];
	[_topBorderColorWell setColor:color];
	[_topBorderNumber    setStringValue:thickness];
	[_topBorderStepper   setFloatValue:value];
}

- (void)setBottomBorderState:(int)state borderColor:(NSColor *)color borderThickness:(NSString *)thickness stepperValue:(float)value
{
	state = NSControlStateValueOff;
	[_bottomBorderBtn       setState:state];
	[_bottomBorderColorWell setColor:color];
	[_bottomBorderNumber    setStringValue:thickness];
	[_bottomBorderStepper   setFloatValue:value];
}

#if 0
- (int)applyItemTag
{
	return [[_applyToPopup selectedItem] tag];
}
#endif

@end
