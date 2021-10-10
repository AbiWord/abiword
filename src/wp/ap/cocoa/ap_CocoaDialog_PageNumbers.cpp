/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ut_types.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "gr_CocoaGraphics.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"
#include "ap_CocoaDialog_PageNumbers.h"


XAP_Dialog * AP_CocoaDialog_PageNumbers::static_constructor(XAP_DialogFactory * pFactory,
                                                         XAP_Dialog_Id dlgid)
{
    AP_CocoaDialog_PageNumbers * p = new AP_CocoaDialog_PageNumbers(pFactory,dlgid);
    return p;
}

AP_CocoaDialog_PageNumbers::AP_CocoaDialog_PageNumbers(XAP_DialogFactory * pDlgFactory,
                                                 XAP_Dialog_Id dlgid)
    : AP_Dialog_PageNumbers(pDlgFactory,dlgid),
		m_pG(NULL),
		m_dlg(nil)
{
  m_recentControl = m_control;
  m_recentAlign   = m_align;
}

AP_CocoaDialog_PageNumbers::~AP_CocoaDialog_PageNumbers(void)
{
  DELETEP (m_pG);
}

void AP_CocoaDialog_PageNumbers::event_OK(void)
{
	m_answer = AP_Dialog_PageNumbers::a_OK;

	// set the align and control data
	m_align   = m_recentAlign;
	m_control = m_recentControl;

	[NSApp stopModal];
}

void AP_CocoaDialog_PageNumbers::event_Cancel(void)
{
	m_answer = AP_Dialog_PageNumbers::a_CANCEL;
	[NSApp stopModal];
}


void AP_CocoaDialog_PageNumbers::event_previewInvalidate(void)
{
	if(m_preview) {
		m_preview->queueDraw();
	}
}

void AP_CocoaDialog_PageNumbers::event_AlignChanged(AP_Dialog_PageNumbers::tAlign   align)
{
	m_recentAlign = align;
	_updatePreview(m_recentAlign, m_recentControl);
}

void AP_CocoaDialog_PageNumbers::event_HdrFtrChanged(AP_Dialog_PageNumbers::tControl control)
{
	m_recentControl = control;
	_updatePreview(m_recentAlign, m_recentControl);
}

void AP_CocoaDialog_PageNumbers::runModal(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[AP_CocoaDialog_PageNumbersController alloc] initFromNib];
	
	// used similarly to convert between text and numeric arguments
	[m_dlg setXAPOwner:this];

	// build the dialog
	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);
	DELETEP (m_pG);
	
	// make a new Cocoa GC
	XAP_CocoaNSView* view = m_dlg.preview;
	NSSize size = view.frame.size;
	GR_CocoaAllocInfo ai(view);
	m_pG = (GR_CocoaGraphics*)XAP_App::getApp()->newGraphics(ai);

	// let the widget materialize
	_createPreviewFromGC(m_pG, (UT_uint32) lrintf(size.width), (UT_uint32) lrintf(size.height));
	view.drawable = m_preview;
	
	// hack in a quick draw here
	_updatePreview(m_recentAlign, m_recentControl);
	event_previewInvalidate();

	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
	DELETEP(m_pG);
}

@implementation AP_CocoaDialog_PageNumbersController

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_PageNumbers"];
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
	_xap = dynamic_cast<AP_CocoaDialog_PageNumbers*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		LocalizeControl([self window],	pSS, AP_STRING_ID_DLG_PageNumbers_Title);

		LocalizeControl(_okBtn,			pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn,		pSS, XAP_STRING_ID_DLG_Cancel);

		LocalizeControl(_positionBox,	pSS, AP_STRING_ID_DLG_PageNumbers_Position);

		LocalizeControl(_headerBtn,		pSS, AP_STRING_ID_DLG_PageNumbers_Header);
		LocalizeControl(_footerBtn,		pSS, AP_STRING_ID_DLG_PageNumbers_Footer);

		LocalizeControl(_alignmentBox,	pSS, AP_STRING_ID_DLG_PageNumbers_Alignment);

		LocalizeControl(_leftBtn,		pSS, AP_STRING_ID_DLG_PageNumbers_Left);
		LocalizeControl(_centerBtn,		pSS, AP_STRING_ID_DLG_PageNumbers_Center);
		LocalizeControl(_rightBtn,		pSS, AP_STRING_ID_DLG_PageNumbers_Right);

		LocalizeControl(_previewBox,	pSS, AP_STRING_ID_DLG_PageNumbers_Preview);

		[_headerBtn setTag:AP_Dialog_PageNumbers::id_HDR];
		[_footerBtn setTag:AP_Dialog_PageNumbers::id_FTR];

		[  _leftBtn setTag:AP_Dialog_PageNumbers::id_LALIGN];
		[_centerBtn setTag:AP_Dialog_PageNumbers::id_CALIGN];
		[ _rightBtn setTag:AP_Dialog_PageNumbers::id_RALIGN];

		[_positionMatrix  selectCellWithTag:((int) ((_xap->isHeader()) ? AP_Dialog_PageNumbers::id_HDR : AP_Dialog_PageNumbers::id_FTR))];
		[_alignmentMatrix selectCellWithTag:((int) _xap->getAlignment())];
	}
}

- (IBAction)alignmentAction:(id)sender
{
	_xap->event_AlignChanged(static_cast<AP_Dialog_PageNumbers::tAlign>([[sender selectedCell] tag]));
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

- (IBAction)positionAction:(id)sender
{
	_xap->event_HdrFtrChanged(static_cast<AP_Dialog_PageNumbers::tControl>([[sender selectedCell] tag]));
}

- (XAP_CocoaNSView*)preview
{
	return _preview;
}

@end
