/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Cocoa dialogs,
// like centering them, measuring them, etc.
#include "xap_CocoaDialog_Utilities.h"

#include "gr_CocoaCairoGraphics.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Zoom.h"
#include "xap_CocoaDlg_Zoom.h"
#include "xav_View.h"

/*****************************************************************/

XAP_Dialog * XAP_CocoaDialog_Zoom::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_Zoom * p = new XAP_CocoaDialog_Zoom(pFactory, dlgid);
	return p;
}

XAP_CocoaDialog_Zoom::XAP_CocoaDialog_Zoom(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	XAP_Dialog_Zoom(pDlgFactory, dlgid)
{
	// 
}

XAP_CocoaDialog_Zoom::~XAP_CocoaDialog_Zoom(void)
{
}

/*****************************************************************/

void XAP_CocoaDialog_Zoom::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	if (!pFrame)
		return;

	m_pFrame = pFrame;

	m_dlg = [[XAP_CocoaDlg_ZoomController alloc] initFromNib];

	[m_dlg setXAPOwner:this];

	NSWindow * window = [m_dlg window];
	
	_populateWindowData();

	[NSApp runModalForWindow:window];
	
	_storeWindowData();
	
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;

	m_pFrame = NULL;
}


void XAP_CocoaDialog_Zoom::event_Close(void)
{
	m_answer = XAP_Dialog_Zoom::a_CANCEL;
	[NSApp stopModal];
}

void XAP_CocoaDialog_Zoom::event_Radio200Clicked(void)
{
	setZoomType(XAP_Frame::z_200);

	[m_dlg setPercentValue:((int) getZoomPercent())];
	[m_dlg _enablePercentSpin:NO];

	_updatePreviewZoomPercent(getZoomPercent());
}

void XAP_CocoaDialog_Zoom::event_Radio100Clicked(void)
{
	setZoomType(XAP_Frame::z_100);

	[m_dlg setPercentValue:((int) getZoomPercent())];
	[m_dlg _enablePercentSpin:NO];

	_updatePreviewZoomPercent(getZoomPercent());
}

void XAP_CocoaDialog_Zoom::event_Radio75Clicked(void)
{
	setZoomType(XAP_Frame::z_75);

	[m_dlg setPercentValue:((int) getZoomPercent())];
	[m_dlg _enablePercentSpin:NO];

	_updatePreviewZoomPercent(getZoomPercent());
}

void XAP_CocoaDialog_Zoom::event_RadioPageWidthClicked(void)
{
	setZoomType(XAP_Frame::z_PAGEWIDTH);

	[m_dlg setPercentValue:((int) getZoomPercent())];
	[m_dlg _enablePercentSpin:NO];

	_updatePreviewZoomPercent(getZoomPercent());
}

void XAP_CocoaDialog_Zoom::event_RadioWholePageClicked(void)
{
	setZoomType(XAP_Frame::z_WHOLEPAGE);

	[m_dlg setPercentValue:((int) getZoomPercent())];
	[m_dlg _enablePercentSpin:NO];

	_updatePreviewZoomPercent(getZoomPercent());
}

void XAP_CocoaDialog_Zoom::event_RadioPercentClicked(void)
{
	setZoomType(XAP_Frame::z_PERCENT);

	[m_dlg _enablePercentSpin:YES];
}

void XAP_CocoaDialog_Zoom::event_SpinPercentChanged(void)
{
	setZoomPercent((UT_uint32) [m_dlg percentValue]);

	[m_dlg setPercentValue:((int) getZoomPercent())];

	_updatePreviewZoomPercent(getZoomPercent());
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
	}
	[[m_dlg zoomMatrix] selectCellWithTag:(int)zoomType];
	[m_dlg setPercentValue:((int) getZoomPercent())];
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
	return [super initWithWindowNibName:@"xap_CocoaDlg_Zoom"];
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
	
	LocalizeControl ([self window],	pSS, XAP_STRING_ID_DLG_Zoom_ZoomTitle);

	LocalizeControl (_closeBtn,	pSS, XAP_STRING_ID_DLG_Close);

	LocalizeControl (_zoomBox,		pSS, XAP_STRING_ID_DLG_Zoom_RadioFrameCaption);

	LocalizeControl (_zoom200Btn,	pSS, XAP_STRING_ID_DLG_Zoom_200);
	LocalizeControl (_zoom100Btn,	pSS, XAP_STRING_ID_DLG_Zoom_100);
	LocalizeControl (_zoom75Btn,	pSS, XAP_STRING_ID_DLG_Zoom_75);
	LocalizeControl (_pageWidthBtn,	pSS, XAP_STRING_ID_DLG_Zoom_PageWidth);
	LocalizeControl (_wholePageBtn,	pSS, XAP_STRING_ID_DLG_Zoom_WholePage);
	LocalizeControl (_percentBtn,	pSS, XAP_STRING_ID_DLG_Zoom_Percent);

	[_zoom200Btn	setTag:(int)XAP_Frame::z_200];
	[_zoom100Btn	setTag:(int)XAP_Frame::z_100];
	[_zoom75Btn		setTag:(int)XAP_Frame::z_75];
	[_pageWidthBtn	setTag:(int)XAP_Frame::z_PAGEWIDTH];
	[_wholePageBtn	setTag:(int)XAP_Frame::z_WHOLEPAGE];
	[_percentBtn	setTag:(int)XAP_Frame::z_PERCENT];

	[_percentStepper setMinValue:((double) XAP_DLG_ZOOM_MINIMUM_ZOOM)];
	[_percentStepper setMaxValue:((double) XAP_DLG_ZOOM_MAXIMUM_ZOOM)];
}

- (IBAction)closeAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Close();
}


- (IBAction)zoom200Action:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Radio200Clicked();
}

- (IBAction)zoom100Action:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Radio100Clicked();
}

- (IBAction)zoom75Action:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Radio75Clicked();
}

- (IBAction)zoomPageWidthAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_RadioPageWidthClicked();
}

- (IBAction)zoomWholePageAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_RadioWholePageClicked();
}

- (IBAction)zoomPercentAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_RadioPercentClicked();
}

- (IBAction)stepperAction:(id)sender
{
	[_percentField setIntValue:[sender intValue]];

	_xap->event_SpinPercentChanged();
}

- (IBAction)zoomChangedAction:(id)sender
{
	UT_UNUSED(sender);
	int percent = [_percentField intValue];
	percent = (percent < XAP_DLG_ZOOM_MINIMUM_ZOOM) ? XAP_DLG_ZOOM_MINIMUM_ZOOM :
			 ((percent > XAP_DLG_ZOOM_MAXIMUM_ZOOM) ? XAP_DLG_ZOOM_MAXIMUM_ZOOM : percent);
	[self setPercentValue:percent];

	_xap->event_SpinPercentChanged();
}

- (NSMatrix*)zoomMatrix
{
	return _zoomMatrix;
}

- (void)setPercentValue:(int)value
{
	[_percentField   setIntValue:value];
	[_percentStepper setIntValue:value];
}

- (int)percentValue
{
	return [_percentField intValue];
}

- (void)_enablePercentSpin:(BOOL)enable
{
	[_percentStepper setEnabled:enable];
	[_percentField   setEnabled:enable];
}

@end

