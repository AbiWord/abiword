/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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
#include <stdio.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_types.h"
#include "ut_string.h"

#include "xap_Preview.h"
#include "ap_Preview_Abi.h"

#include "ap_Strings.h"

/************************************************************************/

// Two REALLY useful macros from ap_Preview_Paragraph to scale to and 
// from inches and pixels

#define DIMENSION_INCH_SCALE_FACTOR	36

#define PIXELS_TO_INCH(p) (double) ((double) p / (double) DIMENSION_INCH_SCALE_FACTOR)

/*!
 * This class gives the full power of the abi formatter to any Graphics Context
 * It creates a new blank document and view set to preview mode. You can
 * access these via the getDoc() and getView() methods to load text and manipulate
 * via props, attributes and style changes. It will faithfully layout text into
 * any graphics context you give it.
\param gc the graphics context to draw into
\param iWidth the width of the gc in pixels
\param iHeight the height of the gc in pixels
\param pFrame a pointer to the Frame that was ultimately responsible for
       creating this class
*/ 
/************************************************************************/

AP_Preview_Abi::AP_Preview_Abi(GR_Graphics * gc, UT_uint32 iWidth, 
									 UT_uint32 iHeight, XAP_Frame * pFrame,PreViewMode previewMode )
	: XAP_Preview(gc)
{
//
// set the size of the window in pixels
//
	setWindowSize(iWidth,iHeight);
//
// code to create an empty blank document to draw into
//
	m_pFrame = pFrame;
//
// Get the width of the current doc so we can scale the graphics context
// zoom
//
	double curWidth = static_cast<PD_Document *>(pFrame->getCurrentDoc())->m_docPageSize.Width(fp_PageSize::inch);
	double curHeight = static_cast<PD_Document *>(pFrame->getCurrentDoc())->m_docPageSize.Height(fp_PageSize::inch);

	m_pApp = pFrame->getApp();
//
// Make a new document
//
	m_pDocument = new PD_Document(m_pApp);
	m_pDocument->newDocument();
//
// Next set the different modes
//
	UT_uint32 iZoom;
	double width,height,previewWidth,tmp;
	switch(previewMode)
	{
//
// For this we set the zoom to fit the page width inside the preview
//
	case PREVIEW_ZOOMED:
		m_pDocument->m_docPageSize.Set(curWidth,curHeight,fp_PageSize::inch);
		previewWidth = ((double) iWidth)/((double) gc->getResolution());
		tmp = 100.0 * previewWidth/curWidth;
		iZoom = (UT_uint32) tmp;
		gc->setZoomPercentage(iZoom);
		break;
//
// In this case we set the page size to fit inside the gc window
//
	case PREVIEW_ADJUSTED_PAGE:
		width = (double) iWidth/((double) gc->getResolution());
		height = (double) iHeight/((double) gc->getResolution()) ;
		m_pDocument->m_docPageSize.Set(width,height,fp_PageSize::inch);
		break;
//
// In this case we set the page size to the current pagesize and just
// clip the display inside the gc window
//
	case PREVIEW_CLIPPED:
		m_pDocument->m_docPageSize.Set(curWidth,curHeight,fp_PageSize::inch);
		break;
//
// For this we set the zoom to fit the page width inside the preview and add
// a scrollbar
//
	case PREVIEW_ZOOMED_SCROLL:
		m_pDocument->m_docPageSize.Set(curWidth,curHeight,fp_PageSize::inch);
		previewWidth = (double) iWidth/((double) gc->getResolution());
		tmp = 100.0 * previewWidth/curWidth;
		iZoom = (UT_uint32) tmp;
		gc->setZoomPercentage(iZoom);

		break;
//
// In this case we set the page size to fit inside the gc window and add a scroll
// bar
//
	case PREVIEW_ADJUSTED_PAGE_SCROLL:
		width = (double) iWidth/((double) gc->getResolution());
		height = (double) iHeight/((double) gc->getResolution()) ;
		m_pDocument->m_docPageSize.Set(width,height,fp_PageSize::inch);
		break;
//
// In this case we set the page size to the current pagesize and just
// clip the display inside the gc window and add a scroll bar.
//
	case PREVIEW_CLIPPED_SCROLL:
		m_pDocument->m_docPageSize.Set(curWidth,curHeight,fp_PageSize::inch);
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	m_pDocLayout = new FL_DocLayout(m_pDocument,gc);
//
// The "true" tells the view that this is a preview so don't need to update
// View listeners for this things like rulers or scroll bars.
//
	m_pView = new FV_View(getApp(),m_pFrame,m_pDocLayout);
	m_pView->setWindowSize(iWidth,iHeight);
	m_pView->setViewMode(VIEW_PREVIEW);
	m_pView->setPreviewMode(previewMode);
//
// OK Now set the scrollbar stuff if needed.
//
	if(previewMode >=  PREVIEW_ZOOMED_SCROLL && previewMode <=  PREVIEW_CLIPPED_SCROLL)
	{
        // FIXME!! FIXME!! This won't work until we put platform specific code
        // beneath this class to handle the scroll bar generation and to connect
        // signals to the scroll bars. Don't use the *_SCROLL modes yet!! 
		//
		// Scroll bar stuff.
		// 
		// The ScrollObj object receives send {Vertical, Horizontal} scroll 
		// events from both the scroll-related edit methods and from the UI 
        // callbacks.
        //
		// AV_ScrollObj * pScrollObj = NULL;
		//
		// The pScrollbarViewListener receives change notifications as 
		// the document changes. This ViewListener is responsible for 
		// recalibrating the scrollbars as pages are added/removed from the
		// document.
		//
		// We don't need listeners for the rulers or the toolbars or 
		// the frame itself here
		// otherwise lots of this code was stolen from ap_UnixFrame.cpp
		//
		//  ap_Scrollbar_ViewListener * pScrollbarViewListener = NULL;
//  		AV_ListenerId lid;
//  		AV_ListenerId lidScrollbarViewListener;
//  		pScrollObj = new AV_ScrollObj(this,_scrollFuncX,_scrollFuncY);
//  		ENSUREP(pScrollObj);
//  		pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
//  		ENSUREP(pScrollbarViewListener);
//  		m_pView->addListener(static_cast<AV_Listener *>(pScrollbarViewListener),
//  							 &lidScrollbarViewListener);
//		m_pView->addScrollListener(pScrollObj);
	}
}

AP_Preview_Abi::~AP_Preview_Abi()
{
	DELETEP(m_pView);
	DELETEP(m_pDocLayout);
	UNREFP(m_pDocument);
}

void AP_Preview_Abi::draw(void)
{
	getView()->updateScreen(false);
}

/*!
 * Do all the manipulations you like via this view pointer
 */
FV_View * AP_Preview_Abi::getView(void) const
{
	return m_pView;
}

PD_Document * AP_Preview_Abi::getDoc(void) const
{
	return m_pDocument;
}

