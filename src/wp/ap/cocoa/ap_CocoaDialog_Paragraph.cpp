/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2003 Hubert Figuiere
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
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "gr_CocoaCairoGraphics.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Dialog_Id.h"

#include "ap_Strings.h"

#include "ap_Preview_Paragraph.h"
#include "ap_CocoaDialog_Paragraph.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_Paragraph::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_Paragraph * p = new AP_CocoaDialog_Paragraph(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_Paragraph::AP_CocoaDialog_Paragraph(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id dlgid)
	: AP_Dialog_Paragraph(pDlgFactory,dlgid),
		m_pGraphics(NULL),
		m_dlg(nil)
{
}

AP_CocoaDialog_Paragraph::~AP_CocoaDialog_Paragraph(void)
{
	DELETEP(m_pGraphics);
}

/*****************************************************************/

void AP_CocoaDialog_Paragraph::runModal(XAP_Frame * pFrame)
{
	NSWindow* window;
	m_pFrame = pFrame;
	
	m_dlg = [[AP_CocoaDialog_ParagraphController alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	window = [m_dlg window];
	_populateWindowData();
 	_createGC([m_dlg preview]);
	event_PreviewAreaExposed();

	// sync all controls once to get started
	// HACK: the first arg gets ignored
	_syncControls(id_MENU_ALIGNMENT, true);

	[NSApp runModalForWindow:window];
	
	
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

/* callbacks to create / release the GR_Graphics when the window is loaded / released */
void	AP_CocoaDialog_Paragraph::_createGC(XAP_CocoaNSView* owner)
{
	NSSize  size;
	GR_CocoaCairoAllocInfo ai(owner);
	m_pGraphics = (GR_CocoaCairoGraphics*)XAP_App::getApp()->newGraphics(ai);

	size = [owner bounds].size;
	_createPreviewFromGC(m_pGraphics, lrintf(size.width), lrintf(size.height));
}

void AP_CocoaDialog_Paragraph::_deleteGC(void)
{
	DELETEP(m_pGraphics);
	m_pGraphics = NULL;
}
/*****************************************************************/

void AP_CocoaDialog_Paragraph::event_OK(void)
{
	m_answer = AP_Dialog_Paragraph::a_OK;
	[NSApp stopModal];
}

void AP_CocoaDialog_Paragraph::event_Cancel(void)
{
	m_answer = AP_Dialog_Paragraph::a_CANCEL;
	[NSApp stopModal];
}

void AP_CocoaDialog_Paragraph::event_Tabs(void)
{
	m_answer = AP_Dialog_Paragraph::a_TABS;
	[NSApp stopModal];
}

void AP_CocoaDialog_Paragraph::event_MenuChanged(id sender)
{
	tControl idc = (tControl)-1;
	
	NSMenu* menu = [sender menu];
	if (menu == [[m_dlg specialPopup] menu]) {
		idc = (tControl)[[m_dlg specialPopup] tag];
	}
	else if (menu == [[m_dlg lineSpacingPopup] menu]) {
		idc = (tControl)[[m_dlg lineSpacingPopup] tag];
	}
	else if (menu == [[m_dlg alignmentPopup] menu]) {
		idc = (tControl)[[m_dlg alignmentPopup] tag];
	}
	
	int value = [[sender selectedItem] tag];

	_setMenuItemValue(idc, value);
}

void AP_CocoaDialog_Paragraph::event_EditChanged(id sender)
{
	tControl idc = (tControl)-1;
	
	if ([sender isKindOfClass:[NSForm class]]) {
		int idx = [sender indexOfSelectedItem];
		UT_ASSERT (idx != -1);
		idc = (tControl)[[sender cellAtRow:idx column:0] tag];
	}
	else {
		idc = (tControl)[sender tag];
	}
	
	// this function will massage the contents for proper
	// formatting for spinbuttons that need it.  for example,
	// line spacing can't be negative.
	_setSpinItemValue(idc, (const gchar *)
						[[sender stringValue] UTF8String]);

	// to ensure the massaged value is reflected back up
	// to the screen, we repaint from the member variable
	_syncControls(idc);
}

void AP_CocoaDialog_Paragraph::event_CheckToggled(id sender)
{

	tControl idc = (tControl) [sender tag];

	int state = [sender state];

	tCheckState cs = check_FALSE;

	switch (state) {
	case NSOnState:
		cs = check_TRUE;
		break;
	case NSOffState:
		cs = check_FALSE;
		break;
	case NSMixedState:
		cs = check_INDETERMINATE;
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
	_setCheckItemValue(idc, cs);
}


void AP_CocoaDialog_Paragraph::event_PreviewAreaExposed(void)
{
	if (m_paragraphPreview)
		m_paragraphPreview->draw();
}


void AP_CocoaDialog_Paragraph::_populateWindowData(void)
{

	// alignment option menu
	[(NSPopUpButton*)_getWidget(id_MENU_ALIGNMENT) selectItemAtIndex:_getMenuItemValue(id_MENU_ALIGNMENT)];

	// indent and paragraph margins
	[_getWidget(id_SPIN_LEFT_INDENT) setStringValue:[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_LEFT_INDENT)]];
	[_getWidget(id_SPIN_RIGHT_INDENT) setStringValue:[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_RIGHT_INDENT)]];
	[_getWidget(id_SPIN_SPECIAL_INDENT) setStringValue:[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_SPECIAL_INDENT)]];
	[(NSPopUpButton*)_getWidget(id_MENU_SPECIAL_INDENT) selectItemAtIndex:_getMenuItemValue(id_MENU_SPECIAL_INDENT)];
	// spacing
	[_getWidget(id_SPIN_BEFORE_SPACING) setStringValue:[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_BEFORE_SPACING)]];
	[_getWidget(id_SPIN_AFTER_SPACING) setStringValue:[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_AFTER_SPACING)]];
	[_getWidget(id_SPIN_SPECIAL_SPACING) setStringValue:[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_SPECIAL_SPACING)]];
	[(NSPopUpButton*)_getWidget(id_MENU_SPECIAL_SPACING) selectItemAtIndex:_getMenuItemValue(id_MENU_SPECIAL_SPACING)];

	// set the check boxes
	[_getWidget(id_CHECK_WIDOW_ORPHAN) setState:_tCheckStateToNS(_getCheckItemValue(id_CHECK_WIDOW_ORPHAN))];
	[_getWidget(id_CHECK_KEEP_LINES) setState:_tCheckStateToNS(_getCheckItemValue(id_CHECK_KEEP_LINES))];
	[_getWidget(id_CHECK_PAGE_BREAK) setState:_tCheckStateToNS(_getCheckItemValue(id_CHECK_PAGE_BREAK))];
	[_getWidget(id_CHECK_SUPPRESS) setState:_tCheckStateToNS(_getCheckItemValue(id_CHECK_SUPPRESS))];
	[_getWidget(id_CHECK_NO_HYPHENATE) setState:_tCheckStateToNS(_getCheckItemValue(id_CHECK_NO_HYPHENATE))];
	[_getWidget(id_CHECK_KEEP_NEXT) setState:_tCheckStateToNS(_getCheckItemValue(id_CHECK_KEEP_NEXT))];
	[_getWidget(id_CHECK_DOMDIRECTION) setState:_tCheckStateToNS(_getCheckItemValue(id_CHECK_DOMDIRECTION))];
}
void AP_CocoaDialog_Paragraph::_syncControls(tControl changed, bool bAll /* = false */)
{
	// let parent sync any member variables first
	AP_Dialog_Paragraph::_syncControls(changed, bAll);

	// sync the display

	// 1.  link the "hanging indent by" combo and spinner
	if (bAll || (changed == id_SPIN_SPECIAL_INDENT))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_INDENT) == indent_FIRSTLINE)
		{
			[_getWidget(id_MENU_SPECIAL_INDENT) selectItemAtIndex:
										_getMenuItemValue(id_MENU_SPECIAL_INDENT)];
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_INDENT))
	{	
		NSTextField* control = nil;
		switch(_getMenuItemValue(id_MENU_SPECIAL_INDENT))
		{
		case indent_NONE:
			// clear the spin control
			{
				control = _getWidget(id_SPIN_SPECIAL_INDENT);
				UT_ASSERT (control);
				[control setStringValue:@""];
				[control setEnabled:NO];
			}
			break;

		default:
			{
				control = _getWidget(id_SPIN_SPECIAL_INDENT);
			// set the spin control
				UT_ASSERT (control);
				[control setStringValue:
						[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_SPECIAL_INDENT)]];
				[control setEnabled:YES];
			}
			break;
		}
	}

	// 2.  link the "line spacing at" combo and spinner

	if (bAll || (changed == id_SPIN_SPECIAL_SPACING))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_SPACING) == spacing_MULTIPLE)
		{
			[_getWidget(id_MENU_SPECIAL_SPACING) selectItemAtIndex:
										_getMenuItemValue(id_MENU_SPECIAL_SPACING)];
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_SPACING))
	{
		NSTextField* control = nil;
		switch(_getMenuItemValue(id_MENU_SPECIAL_SPACING))
		{
		case spacing_SINGLE:
		case spacing_ONEANDHALF:
		case spacing_DOUBLE:
			// clear the spin control
			control = _getWidget(id_SPIN_SPECIAL_SPACING);
			[control setStringValue:@""];
			[control setEnabled:NO];
			break;

		default:
			// set the spin control
			control = _getWidget(id_SPIN_SPECIAL_SPACING);
			[control setStringValue:
					[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_SPECIAL_SPACING)]];
			[control setEnabled:YES];
			break;
		}
	}

	// 3.  move results of _doSpin() back to screen

	if (!bAll)
	{
		// spin controls only sync when spun
		switch (changed)
		{
		case id_SPIN_LEFT_INDENT:
			[_getWidget(id_SPIN_LEFT_INDENT) setStringValue:
					[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_LEFT_INDENT)]];
		case id_SPIN_RIGHT_INDENT:
			[_getWidget(id_SPIN_RIGHT_INDENT) setStringValue:
					[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_RIGHT_INDENT)]];
		case id_SPIN_SPECIAL_INDENT:
			[_getWidget(id_SPIN_SPECIAL_INDENT) setStringValue:
					[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_SPECIAL_INDENT)]];
		case id_SPIN_BEFORE_SPACING:
			[_getWidget(id_SPIN_BEFORE_SPACING) setStringValue:
					[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_BEFORE_SPACING)]];
		case id_SPIN_AFTER_SPACING:
			[_getWidget(id_SPIN_AFTER_SPACING) setStringValue:
					[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_AFTER_SPACING)]];
		case id_SPIN_SPECIAL_SPACING:
			[_getWidget(id_SPIN_SPECIAL_SPACING) setStringValue:
					[NSString stringWithUTF8String:_getSpinItemValue(id_SPIN_SPECIAL_SPACING)]];
		default:
			break;
		}
	}
}

int AP_CocoaDialog_Paragraph::_tCheckStateToNS(AP_CocoaDialog_Paragraph::tCheckState x) 
{
	switch (x) {
	case check_FALSE:
		return NSOffState;
	case check_TRUE:
		return NSOnState;
	case check_INDETERMINATE:
		return NSMixedState;
	}
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return 0;
}


@implementation AP_CocoaDialog_ParagraphController

- (id) initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_Paragraph"];
}

- (void)setXAPOwner:(XAP_Dialog*)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_Paragraph*>(owner);
	UT_ASSERT(_xap);
}

- (void)discardXAP
{
	if (_xap) {
		_xap->_deleteGC();
		_xap = NULL;
	}
}

- (void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Para_ParaTitle);
		LocalizeControl(_tabsBtn, pSS, AP_STRING_ID_DLG_Para_ButtonTabs);
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl([_mainTab tabViewItemAtIndex:0], pSS, AP_STRING_ID_DLG_Para_TabLabelIndentsAndSpacing);
		LocalizeControl(_alignmentLabel, pSS, AP_STRING_ID_DLG_Para_LabelAlignment);
		/* menu items */
		[_alignmentPopup setTag:(int)AP_Dialog_Paragraph::id_MENU_ALIGNMENT];
		[_alignmentPopup removeAllItems];
		[_alignmentPopup addItemWithTitle:@" "];
		AppendLocalizedMenuItem(_alignmentPopup, pSS, AP_STRING_ID_DLG_Para_AlignLeft, AP_Dialog_Paragraph::align_LEFT);
		AppendLocalizedMenuItem(_alignmentPopup, pSS, AP_STRING_ID_DLG_Para_AlignCentered, AP_Dialog_Paragraph::align_CENTERED );
		AppendLocalizedMenuItem(_alignmentPopup, pSS, AP_STRING_ID_DLG_Para_AlignRight, AP_Dialog_Paragraph::align_RIGHT);
		AppendLocalizedMenuItem(_alignmentPopup, pSS, AP_STRING_ID_DLG_Para_AlignJustified, AP_Dialog_Paragraph::align_JUSTIFIED);
		LocalizeControl(_rtlDominantBtn, pSS, AP_STRING_ID_DLG_Para_DomDirection);
		[_rtlDominantBtn setTag:(int)AP_Dialog_Paragraph::id_CHECK_DOMDIRECTION];
		LocalizeControl(_indentationBox, pSS, AP_STRING_ID_DLG_Para_LabelIndentation);
		LocalizeControl(_indentLeftFormCell, pSS, AP_STRING_ID_DLG_Para_LabelLeft);
		[_indentLeftFormCell setTag:(int)AP_Dialog_Paragraph::id_SPIN_LEFT_INDENT];
		LocalizeControl(_indentRightFormCell, pSS, AP_STRING_ID_DLG_Para_LabelRight);
		[_indentRightFormCell setTag:(int)AP_Dialog_Paragraph::id_SPIN_RIGHT_INDENT];
		LocalizeControl(_specialLabel, pSS, AP_STRING_ID_DLG_Para_LabelSpecial);

		/*menu items*/
		[_specialPopup setTag:(int)AP_Dialog_Paragraph::id_MENU_SPECIAL_INDENT];
		[_specialPopup removeAllItems];
		[_specialPopup addItemWithTitle:@" "];
		AppendLocalizedMenuItem(_specialPopup, pSS, AP_STRING_ID_DLG_Para_SpecialNone, AP_Dialog_Paragraph::indent_NONE);
		AppendLocalizedMenuItem(_specialPopup, pSS, AP_STRING_ID_DLG_Para_SpecialFirstLine, AP_Dialog_Paragraph::indent_FIRSTLINE);
		AppendLocalizedMenuItem(_specialPopup, pSS, AP_STRING_ID_DLG_Para_SpecialHanging, AP_Dialog_Paragraph::indent_HANGING);
		LocalizeControl(_byLabel, pSS, AP_STRING_ID_DLG_Para_LabelBy);
		[_byData setTag:(int)AP_Dialog_Paragraph::id_SPIN_SPECIAL_INDENT];
		LocalizeControl(_spacingBox, pSS, AP_STRING_ID_DLG_Para_LabelSpacing);
		LocalizeControl(_spacingBeforeFormCell, pSS, AP_STRING_ID_DLG_Para_LabelBefore);
		[_spacingBeforeFormCell setTag:(int)AP_Dialog_Paragraph::id_SPIN_BEFORE_SPACING];
		LocalizeControl(_spacingAfterFormCell, pSS, AP_STRING_ID_DLG_Para_LabelAfter);
		[_spacingAfterFormCell setTag:(int)AP_Dialog_Paragraph::id_SPIN_AFTER_SPACING];
		LocalizeControl(_lineSpacingLabel, pSS, AP_STRING_ID_DLG_Para_LabelLineSpacing);
		/*menu items*/
		[_lineSpacingPopup setTag:(int)AP_Dialog_Paragraph::id_MENU_SPECIAL_SPACING];
		[_lineSpacingPopup removeAllItems];
		[_lineSpacingPopup addItemWithTitle:@" "];
		AppendLocalizedMenuItem(_lineSpacingPopup, pSS, AP_STRING_ID_DLG_Para_SpacingSingle, AP_Dialog_Paragraph::spacing_SINGLE);
		AppendLocalizedMenuItem(_lineSpacingPopup, pSS, AP_STRING_ID_DLG_Para_SpacingHalf, AP_Dialog_Paragraph::spacing_ONEANDHALF);
		AppendLocalizedMenuItem(_lineSpacingPopup, pSS, AP_STRING_ID_DLG_Para_SpacingDouble, AP_Dialog_Paragraph::spacing_DOUBLE);
		AppendLocalizedMenuItem(_lineSpacingPopup, pSS, AP_STRING_ID_DLG_Para_SpacingAtLeast, AP_Dialog_Paragraph::spacing_ATLEAST);
		AppendLocalizedMenuItem(_lineSpacingPopup, pSS, AP_STRING_ID_DLG_Para_SpacingExactly, AP_Dialog_Paragraph::spacing_EXACTLY);
		AppendLocalizedMenuItem(_lineSpacingPopup, pSS, AP_STRING_ID_DLG_Para_SpacingMultiple, AP_Dialog_Paragraph::spacing_MULTIPLE);
		LocalizeControl(_atLabel, pSS, AP_STRING_ID_DLG_Para_LabelAt);
		[_atData setTag:(int)AP_Dialog_Paragraph::id_SPIN_SPECIAL_SPACING];
		
		LocalizeControl([_mainTab tabViewItemAtIndex:1], pSS, AP_STRING_ID_DLG_Para_TabLabelLineAndPageBreaks);
		LocalizeControl(_paginationBox, pSS, AP_STRING_ID_DLG_Para_LabelPagination);
		LocalizeControl(_widowOrphanBtn, pSS, AP_STRING_ID_DLG_Para_PushWidowOrphanControl);
		[_widowOrphanBtn setTag:(int)AP_Dialog_Paragraph::id_CHECK_WIDOW_ORPHAN];
		LocalizeControl(_keepNextBtn, pSS, AP_STRING_ID_DLG_Para_PushKeepWithNext);
		[_keepNextBtn setTag:(int)AP_Dialog_Paragraph::id_CHECK_KEEP_NEXT];
		LocalizeControl(_keepsLinesBtn, pSS, AP_STRING_ID_DLG_Para_PushKeepLinesTogether);
		[_keepsLinesBtn setTag:(int)AP_Dialog_Paragraph::id_CHECK_KEEP_LINES];
		LocalizeControl(_pageBreakBtn, pSS, AP_STRING_ID_DLG_Para_PushPageBreakBefore);
		[_pageBreakBtn setTag:(int)AP_Dialog_Paragraph::id_CHECK_PAGE_BREAK];
		LocalizeControl(_suppressLineNumBtn, pSS, AP_STRING_ID_DLG_Para_PushSuppressLineNumbers);
		[_suppressLineNumBtn setTag:(int)AP_Dialog_Paragraph::id_CHECK_SUPPRESS];
		LocalizeControl(_dontHyphenBtn, pSS, AP_STRING_ID_DLG_Para_PushNoHyphenate);
		[_dontHyphenBtn setTag:(int)AP_Dialog_Paragraph::id_CHECK_NO_HYPHENATE];
		LocalizeControl(_previewBox, pSS, AP_STRING_ID_DLG_Para_LabelPreview);
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

- (IBAction)tabAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Tabs();
}

- (IBAction)checkBoxAction:(id)sender
{
	_xap->event_CheckToggled(sender);
}

- (IBAction)menuAction:(id)sender
{
	_xap->event_MenuChanged(sender);
}

- (IBAction)editAction:(id)sender
{
	_xap->event_EditChanged(sender);
}

/*
- (void)controlTextDidChange:(NSNotification *)aNotification
{
	id obj = [aNotification object];
	
}
*/

- (XAP_CocoaNSView*)preview
{
	return _preview;
}

- (id)_getWidget:(AP_Dialog_Paragraph::tControl) widget
{
	id obj = nil;
	switch(widget) {
	case AP_Dialog_Paragraph::id_MENU_ALIGNMENT:
		return _alignmentPopup;
	case AP_Dialog_Paragraph::id_SPIN_LEFT_INDENT:
		return _indentLeftFormCell;
	case AP_Dialog_Paragraph::id_SPIN_RIGHT_INDENT:
		return _indentRightFormCell;
	case AP_Dialog_Paragraph::id_MENU_SPECIAL_INDENT:
		return _specialPopup;
	case AP_Dialog_Paragraph::id_SPIN_SPECIAL_INDENT:
		return _byData;
	case AP_Dialog_Paragraph::id_SPIN_BEFORE_SPACING:
		return _spacingBeforeFormCell;
	case AP_Dialog_Paragraph::id_SPIN_AFTER_SPACING:
		return _spacingAfterFormCell;
	case AP_Dialog_Paragraph::id_MENU_SPECIAL_SPACING:
		return _lineSpacingPopup;
	case AP_Dialog_Paragraph::id_SPIN_SPECIAL_SPACING:
		return _atData;
	case AP_Dialog_Paragraph::id_CHECK_WIDOW_ORPHAN:
		return _widowOrphanBtn;
	case AP_Dialog_Paragraph::id_CHECK_KEEP_LINES:
		return _keepsLinesBtn;
	case AP_Dialog_Paragraph::id_CHECK_PAGE_BREAK:
		return _pageBreakBtn;
	case AP_Dialog_Paragraph::id_CHECK_SUPPRESS:
		return _suppressLineNumBtn;
	case AP_Dialog_Paragraph::id_CHECK_NO_HYPHENATE:
		return _dontHyphenBtn;
	case AP_Dialog_Paragraph::id_CHECK_KEEP_NEXT:
		return _keepNextBtn;
	case AP_Dialog_Paragraph::id_CHECK_DOMDIRECTION:
		return _rtlDominantBtn;
	}
	UT_ASSERT(obj);
	return obj;
}

- (NSPopUpButton*)specialPopup
{
	return _specialPopup;
}

- (NSPopUpButton*)lineSpacingPopup
{
	return _lineSpacingPopup;
}

- (NSPopUpButton*)alignmentPopup
{
	return _alignmentPopup;
}

@end
