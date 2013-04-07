/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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

#include "ut_debugmsg.h"
#include "ap_CocoaPreview_Annotation.h"
#include "gr_CocoaCairoGraphics.h"
#include "xap_Frame.h"
#include "xap_CocoaFrameImpl.h"
#include "xap_CocoaFrame.h"
#include "xap_App.h"

AP_CocoaPreview_Annotation::AP_CocoaPreview_Annotation(XAP_DialogFactory * pDlgFactory,XAP_Dialog_Id dlgid) 
  : AP_Preview_Annotation(pDlgFactory,dlgid),
  m_gc(NULL),
  m_pPreviewWindow(nil),
  m_pDrawingArea(nil)
{
	UT_DEBUGMSG(("AP_CocoaPreview_Annotation: Preview annotation for Cocoa platform\n"));
}

AP_CocoaPreview_Annotation::~AP_CocoaPreview_Annotation(void)
{
  UT_DEBUGMSG(("Preview Annotation deleted %p \n",this));
  destroy();
}

void AP_CocoaPreview_Annotation::runModeless(XAP_Frame * pFrame)
{
	UT_DEBUGMSG(("Preview Annotation runModeless %p \n",this));
	setActiveFrame(pFrame);
	if(m_pPreviewWindow)
	{
		[m_pPreviewWindow release];
		m_pPreviewWindow = nil;
		m_pDrawingArea = nil;
		DELETEP(m_gc);
	}
	setSizeFromAnnotation();
	_constructWindow();
	const XAP_CocoaFrameImpl *impl = (const XAP_CocoaFrameImpl*)pFrame->getFrameImpl();
	[m_pPreviewWindow orderFront:impl->getTopLevelWindow()];
	
	// make a new Cocoa GC
	DELETEP(m_gc);
	
	XAP_App *pApp = XAP_App::getApp();
	GR_CocoaCairoAllocInfo ai(m_pDrawingArea);
	m_gc = (GR_CairoGraphics*) pApp->newGraphics(ai);
	
	NSSize size = [m_pDrawingArea frame].size;
	_createAnnotationPreviewFromGC(m_gc, size.width, size.height);
	m_gc->setZoomPercentage(100);
}

void AP_CocoaPreview_Annotation::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
	//UT_DEBUGMSG(("notifyActiveFrame: trying to activate... %p \n",this));
	//activate();
}

void AP_CocoaPreview_Annotation::activate(void)
{
	UT_ASSERT(m_pPreviewWindow);
	[m_pPreviewWindow orderFront:nil];
}

XAP_Dialog * AP_CocoaPreview_Annotation::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new AP_CocoaPreview_Annotation(pFactory,id);
}

void  AP_CocoaPreview_Annotation::_constructWindow(void)
{
	XAP_App::getApp()->rememberModelessId(getDialogId(), static_cast<XAP_Dialog_Modeless *>(this));
	UT_DEBUGMSG(("Contructing Window width %d height %d left %d top %d \n",m_width,m_height,m_left,m_top));

	// use the mouse position as origin
	NSPoint pt = [NSEvent mouseLocation];
	NSRect rect = NSMakeRect(pt.x, pt.y, m_width, m_height);
	
	m_pPreviewWindow = [[NSWindow alloc] initWithContentRect:rect styleMask:NSBorderlessWindowMask
	                    backing:NSBackingStoreRetained defer:NO];
	
	m_pDrawingArea = [[XAP_CocoaNSView alloc] initWith:NULL andFrame:[m_pPreviewWindow contentRectForFrameRect:rect]];
	
	[m_pPreviewWindow setContentView:m_pDrawingArea];
}

void  AP_CocoaPreview_Annotation::destroy(void)
{
	modeless_cleanup();
	if (m_pPreviewWindow != nil)
	{
		//_bringToTop();
		//sleep(4);
		//UT_DEBUGMSG(("sleep over\n"));
		
		[m_pPreviewWindow release];
		m_pPreviewWindow = nil;
		m_pDrawingArea = nil;
		DELETEP(m_gc);
	}
}



