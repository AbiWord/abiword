/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
 * Copyright (C) 2003-2004 Hubert Figuiere
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

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_Frame.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_FormatTable.h"
#include "ap_CocoaDialog_FormatTable.h"
#include "ap_CocoaDialog_Columns.h"



/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_FormatTable::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_FormatTable * p = new AP_CocoaDialog_FormatTable(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_FormatTable::AP_CocoaDialog_FormatTable(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id dlgid)
	: AP_Dialog_FormatTable(pDlgFactory,dlgid),
	m_pPreviewWidget(NULL),
	m_dlg(nil)
{

}

AP_CocoaDialog_FormatTable::~AP_CocoaDialog_FormatTable(void)
{
}

void AP_CocoaDialog_FormatTable::runModeless(XAP_Frame * pFrame)
{
	m_dlg = [[AP_CocoaDialog_FormatTableController alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	NSWindow* window = [m_dlg window];

	// Populate the window's data items
	_populateWindowData();


	// make a new Cocoa GC
	DELETEP (m_pPreviewWidget);
	XAP_CocoaNSView * view = [m_dlg preview];
	GR_CocoaAllocInfo ai(view);
	m_pPreviewWidget = (GR_CocoaGraphics*)XAP_App::getApp()->newGraphics(ai);

	// Todo: we need a good widget to query with a probable
	// Todo: non-white (i.e. gray, or a similar bgcolor as our parent widget)
	// Todo: background. This should be fine
	m_pPreviewWidget->init3dColors();

	// let the widget materialize
	NSSize size = [view bounds].size;
	_createPreviewFromGC(m_pPreviewWidget,
						 static_cast<UT_uint32>(lrintf(size.width)),
						 static_cast<UT_uint32>(lrintf(size.width)));	
	
	m_pFormatTablePreview->draw();

	[window orderFront:m_dlg];	
	startUpdater();
}

void AP_CocoaDialog_FormatTable::setSensitivity(bool bSens)
{
	[m_dlg setSensitivity:bSens];
}

void AP_CocoaDialog_FormatTable::setBackgroundColorInGUI(UT_RGBColor clr)
{
	UT_ASSERT_NOT_REACHED(); // TODO
}

void AP_CocoaDialog_FormatTable::setBorderThicknessInGUI(UT_UTF8String & sThick)
{
	UT_ASSERT_NOT_REACHED();
}

void AP_CocoaDialog_FormatTable::event_Close(void)
{
	m_answer = AP_Dialog_FormatTable::a_CLOSE;
	destroy();
}

void AP_CocoaDialog_FormatTable::event_previewExposed(void)
{
	if(m_pFormatTablePreview) {
		m_pFormatTablePreview->draw();
	}
}


void AP_CocoaDialog_FormatTable::event_ApplyToChanged(void)
{
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
}

void AP_CocoaDialog_FormatTable::destroy(void)
{
	finalize();
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

void AP_CocoaDialog_FormatTable::activate(void)
{
	ConstructWindowName();
	NSWindow *window = [m_dlg window];
	[window setTitle:[NSString stringWithUTF8String:getWindowName()]];
	[window orderFront:m_dlg];
	setAllSensitivities();
}


void AP_CocoaDialog_FormatTable::notifyActiveFrame(XAP_Frame *pFrame)
{
	ConstructWindowName();
	NSWindow *window = [m_dlg window];
	[window setTitle:[NSString stringWithUTF8String:getWindowName()]];
	setAllSensitivities();
}

/*****************************************************************/
void AP_CocoaDialog_FormatTable::_populateWindowData(void)
{
   setAllSensitivities();
}

void AP_CocoaDialog_FormatTable::_storeWindowData(void)
{
}


@implementation AP_CocoaDialog_FormatTableController


- (id)initFromNib
{
	self = [super initWithWindowNibName:@"ap_CocoaDialog_FormatTable"];
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
	_xap = dynamic_cast<AP_CocoaDialog_FormatTable*>(owner);
}

- (void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		_xap->ConstructWindowName();
		[[self window] setTitle:[NSString stringWithUTF8String:_xap->getWindowName()]];
		LocalizeControl(_bgBox, pSS, AP_STRING_ID_DLG_FormatTable_Background);
		LocalizeControl(_bgColorLabel, pSS, AP_STRING_ID_DLG_FormatTable_Color);
		LocalizeControl(_borderBox, pSS, AP_STRING_ID_DLG_FormatTable_Borders);
		LocalizeControl(_borderColorLabel, pSS, AP_STRING_ID_DLG_FormatTable_Color);
	//	LocalizeControl(_mergeCellsBox, pSS, AP_STRING_ID_DLG_FormatTable_SetImageBackground);
		LocalizeControl(_setImageBtn, pSS, AP_STRING_ID_DLG_FormatTable_SelectImage);
		[_setImageBtn setImage:[NSImage imageNamed:@"tb_insert_graphic"]];
		LocalizeControl(_noImageBtn, pSS, AP_STRING_ID_DLG_FormatTable_NoImageBackground);
		[_noImageBtn setImage:[NSImage imageNamed:@"tb_remove_graphic"]];
		LocalizeControl(_previewBox, pSS, AP_STRING_ID_DLG_FormatTable_Preview);
		LocalizeControl(_applyToLabel, pSS, AP_STRING_ID_DLG_FormatTable_Apply_To);
		[_applyToPopup removeAllItems];
		AppendLocalizedMenuItem(_applyToPopup, pSS, AP_STRING_ID_DLG_FormatTable_Apply_To_Selection, 0);
		AppendLocalizedMenuItem(_applyToPopup, pSS, AP_STRING_ID_DLG_FormatTable_Apply_To_Row, 1);
		AppendLocalizedMenuItem(_applyToPopup, pSS, AP_STRING_ID_DLG_FormatTable_Apply_To_Column, 2);
		AppendLocalizedMenuItem(_applyToPopup, pSS, AP_STRING_ID_DLG_FormatTable_Apply_To_Table, 3);
		[_rightBorderBtn setImage:[NSImage imageNamed:@"tb_LineRight"]];
		[_topBorderBtn setImage:[NSImage imageNamed:@"tb_LineTop"]];
		[_leftBorderBtn setImage:[NSImage imageNamed:@"tb_LineLeft"]];
		[_bottomBorderBtn setImage:[NSImage imageNamed:@"tb_LineBottom"]];
	}
}

- (IBAction)applyAction:(id)sender
{
	_xap->applyChanges();
}

- (IBAction)bgColorAction:(id)sender
{
}

- (IBAction)borderColorAction:(id)sender
{
}

- (IBAction)bottomBorderAction:(id)sender
{
	_xap->toggleLineType(AP_Dialog_FormatTable::toggle_bottom, [sender state] == NSOnState);
	_xap->event_previewExposed();
}

- (IBAction)leftBorderAction:(id)sender
{
	_xap->toggleLineType(AP_Dialog_FormatTable::toggle_left, [sender state] == NSOnState);
	_xap->event_previewExposed();
}

- (IBAction)removeImageAction:(id)sender
{
	_xap->clearImage();
}

- (IBAction)rightBorderAction:(id)sender
{
	_xap->toggleLineType(AP_Dialog_FormatTable::toggle_right, [sender state] == NSOnState);
	_xap->event_previewExposed();
}

- (IBAction)selectImageAction:(id)sender
{
	_xap->askForGraphicPathName();
}

- (IBAction)topBorderAction:(id)sender
{
	_xap->toggleLineType(AP_Dialog_FormatTable::toggle_top, [sender state] == NSOnState);
	_xap->event_previewExposed();
}

- (IBAction)applyToAction:(id)sender
{
	_xap->event_ApplyToChanged();
}


- (XAP_CocoaNSView*)preview
{
	return _preview;
}


- (void)setSensitivity:(bool)bSens
{
	[_applyBtn setEnabled:bSens];
	[_borderColorWell setEnabled:bSens];
	[_bgColorWell setEnabled:bSens];
	[_bottomBorderBtn setEnabled:bSens];
	[_leftBorderBtn setEnabled:bSens];
	[_rightBorderBtn setEnabled:bSens];
	[_topBorderBtn setEnabled:bSens];
}


- (int)applyItemTag
{
	return [[_applyToPopup selectedItem] tag];
}

@end
