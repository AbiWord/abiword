/* AbiSource Application Framework
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Cocoa dialogs,
// like centering them, measuring them, etc.
#include "xap_CocoaDialog_Utilities.h"

#include "gr_CocoaGraphics.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Zoom.h"
#include "xap_CocoaDlg_Zoom.h"
#include "xav_View.h"

/*****************************************************************/

XAP_Dialog * XAP_CocoaDialog_Zoom::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_Zoom * p = new XAP_CocoaDialog_Zoom(pFactory, dlgid);
	return p;
}

XAP_CocoaDialog_Zoom::XAP_CocoaDialog_Zoom(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id dlgid)
	: XAP_Dialog_Zoom(pDlgFactory, dlgid),
		m_pGR(NULL)
{
}

XAP_CocoaDialog_Zoom::~XAP_CocoaDialog_Zoom(void)
{
	DELETEP(m_pGR);
}

/*****************************************************************/

void XAP_CocoaDialog_Zoom::runModal(XAP_Frame * pFrame)
{
	NSWindow* window;
	m_pFrame = pFrame;
	UT_ASSERT(m_pFrame);
	
	m_dlg = [[XAP_CocoaDlg_ZoomController alloc] initFromNib];
	[m_dlg setXAPOwner:this];
	window = [m_dlg window];
	
	// make a new Cocoa GC
	XAP_CocoaNSView* view = [m_dlg preview];
	NSSize size = [view bounds].size;
	//m_pGR = new GR_CocoaGraphics(view, m_pApp);
	GR_CocoaAllocInfo ai(view, m_pApp);
	m_pGR = (GR_CocoaGraphics*)XAP_App::getApp()->newGraphics(ai);

	_createPreviewFromGC(m_pGR, lrintf(size.width), lrintf(size.height));
	_populateWindowData();

	[NSApp runModalForWindow:window];
	
	_storeWindowData();
	
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
	m_pFrame = NULL;
}

void XAP_CocoaDialog_Zoom::event_OK(void)
{
	m_answer = XAP_Dialog_Zoom::a_OK;
	[NSApp stopModal];
}

void XAP_CocoaDialog_Zoom::event_Cancel(void)
{
	m_answer = XAP_Dialog_Zoom::a_CANCEL;
	[NSApp stopModal];
}

void XAP_CocoaDialog_Zoom::event_Radio200Clicked(void)
{
	[m_dlg _enablePercentSpin:NO];
	_updatePreviewZoomPercent(200);
}

void XAP_CocoaDialog_Zoom::event_Radio100Clicked(void)
{
	[m_dlg _enablePercentSpin:NO];
	_updatePreviewZoomPercent(100);
}

void XAP_CocoaDialog_Zoom::event_Radio75Clicked(void)
{
	[m_dlg _enablePercentSpin:NO];
	_updatePreviewZoomPercent(75);
}

void XAP_CocoaDialog_Zoom::event_RadioPageWidthClicked(void)
{
	[m_dlg _enablePercentSpin:NO];
    _updatePreviewZoomPercent(m_pFrame->getCurrentView()->calculateZoomPercentForPageWidth());
}

void XAP_CocoaDialog_Zoom::event_RadioWholePageClicked(void)
{
	[m_dlg _enablePercentSpin:NO];
	_updatePreviewZoomPercent(m_pFrame->getCurrentView()->calculateZoomPercentForWholePage());
}

void XAP_CocoaDialog_Zoom::event_RadioPercentClicked(void)
{
	[m_dlg _enablePercentSpin:YES];
	// call event_SpinPercentChanged() to do the fetch and update work
	event_SpinPercentChanged();
}

void XAP_CocoaDialog_Zoom::event_SpinPercentChanged(void)
{
	_updatePreviewZoomPercent([m_dlg percentValue]);
}


/*****************************************************************/

void XAP_CocoaDialog_Zoom::_populateWindowData(void)
{
	// The callbacks for these radio buttons aren't always
	// called when the dialog is being constructed, so we have to
	// set the widget's value, then manually enable/disable
	// the spin button.
	
	// enable the right button
	[m_dlg _enablePercentSpin:NO];
	XAP_Frame::tZoomType zoomType = getZoomType();
	switch(zoomType)
	{
	case XAP_Frame::z_200:
		_updatePreviewZoomPercent(200);
		break;
	case XAP_Frame::z_100:
		_updatePreviewZoomPercent(100);		
		break;
	case XAP_Frame::z_75:
		_updatePreviewZoomPercent(75);
		break;
	case XAP_Frame::z_PAGEWIDTH:
		break;
	case XAP_Frame::z_WHOLEPAGE:
		break;
	case XAP_Frame::z_PERCENT:
		[m_dlg _enablePercentSpin:YES];
		_updatePreviewZoomPercent(getZoomPercent());
		break;
	default:
		// if they haven't set anything yet, default to the 100% radio item
		zoomType = XAP_Frame::z_100;
	}
	[[m_dlg zoomMatrix] selectCellWithTag:(int)zoomType];
	[m_dlg setPercentValue:getZoomPercent()];
}

void XAP_CocoaDialog_Zoom::_storeWindowData(void)
{
	m_zoomType = (XAP_Frame::tZoomType)[[[m_dlg zoomMatrix] selectedCell] tag];
	// store away percentage; the base class decides if it's important when
	// the caller requests the percent
	m_zoomPercent = [m_dlg percentValue];
}


//****************************************************************

@implementation XAP_CocoaDlg_ZoomController

- (id) initFromNib
{
	self = [super initWithWindowNibName:@"xap_CocoaDlg_Zoom"];
	return self;
}


- (void)setXAPOwner:(XAP_Dialog*)owner
{
	_xap = dynamic_cast<XAP_CocoaDialog_Zoom*>(owner);
	UT_ASSERT(_xap);
}

- (void)discardXAP
{
	_xap = nil;
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	LocalizeControl ([self window], pSS, XAP_STRING_ID_DLG_Zoom_ZoomTitle);
	LocalizeControl (_zoomBox, pSS, XAP_STRING_ID_DLG_Zoom_RadioFrameCaption);
	LocalizeControl (_zoom200Btn, pSS, XAP_STRING_ID_DLG_Zoom_200);
	[_zoom200Btn setTag:(int)XAP_Frame::z_200];
	LocalizeControl (_zoom100Btn, pSS,	XAP_STRING_ID_DLG_Zoom_100);
	[_zoom100Btn setTag:(int)XAP_Frame::z_100];
	LocalizeControl (_zoom75Btn, pSS, XAP_STRING_ID_DLG_Zoom_75);
	[_zoom75Btn setTag:(int)XAP_Frame::z_75];
	LocalizeControl (_pageWidthBtn, pSS, XAP_STRING_ID_DLG_Zoom_PageWidth);
	[_pageWidthBtn setTag:(int)XAP_Frame::z_PAGEWIDTH];
	LocalizeControl (_wholePageBtn, pSS, XAP_STRING_ID_DLG_Zoom_WholePage);
	[_wholePageBtn setTag:(int)XAP_Frame::z_WHOLEPAGE];
	LocalizeControl (_percentBtn, pSS, XAP_STRING_ID_DLG_Zoom_Percent);
	[_percentBtn setTag:(int)XAP_Frame::z_PERCENT];
	LocalizeControl (_previewBox, pSS, XAP_STRING_ID_DLG_Zoom_PreviewFrame);
	LocalizeControl (_okBtn, pSS, XAP_STRING_ID_DLG_OK);
	LocalizeControl (_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
}


- (IBAction)cancelAction:(id)sender
{
	_xap->event_Cancel();
}

- (IBAction)okAction:(id)sender
{
	_xap->event_OK();
}

- (IBAction)stepperAction:(id)sender
{
	[_percentField setIntValue:[sender intValue]];
}

- (IBAction)zoom100Action:(id)sender
{
	_xap->event_Radio100Clicked();
}


- (IBAction)zoom200Action:(id)sender
{
	_xap->event_Radio200Clicked();
}


- (IBAction)zoom75Action:(id)sender
{
	_xap->event_Radio75Clicked();
}


- (IBAction)zoomPageWidthAction:(id)sender
{
	_xap->event_RadioPageWidthClicked();
}


- (IBAction)zoomWholePageAction:(id)sender
{
	_xap->event_RadioWholePageClicked();
}


- (IBAction)zoomPercentAction:(id)sender
{
	_xap->event_RadioPercentClicked();
}

- (IBAction)zoomChangedAction:(id)sender
{
	_xap->event_SpinPercentChanged();
}

- (XAP_CocoaNSView*)preview
{
	return _preview;
}

- (NSMatrix*)zoomMatrix
{
	return _zoomMatrix;
}

- (void)setPercentValue:(int)value
{
	[_percentField setIntValue:value];
	[_percentStepper setIntValue:value];
}


- (int)percentValue
{
	int percent = [_percentField intValue];
	percent = (percent < 1) ? 1 : ((percent > 1000) ? 1000 : percent);
	[self setPercentValue:percent];
	return percent;
}

- (void)_enablePercentSpin:(BOOL)enable
{
	[_percentStepper setEnabled:enable];
	[_percentField setEnabled:enable];
}

@end

