/* AbiSource Application Framework
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

#include <windows.h>
#include <stdio.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "xap_Win32PreviewWidget.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#define GWL(hwnd)		(XAP_Win32PreviewWidget *)GetWindowLong((hwnd), GWL_USERDATA)
#define SWL(hwnd, f)	(XAP_Win32PreviewWidget *)SetWindowLong((hwnd), GWL_USERDATA,(LONG)(f))

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

ATOM		XAP_Win32PreviewWidget::m_atomPreviewWidgetClass = NULL;
UT_uint32	XAP_Win32PreviewWidget::m_iInstanceCount = 0;
char		XAP_Win32PreviewWidget::m_bufClassName[100];

XAP_Win32PreviewWidget::XAP_Win32PreviewWidget(XAP_Win32App * pWin32App, HWND hwndParent, UINT style)
{
	m_hwndPreview = NULL;
	m_pWin32App = pWin32App;
	m_pGraphics = NULL;
	m_pPreview = NULL;
	
	if(!m_atomPreviewWidgetClass)
		{
		sprintf(m_bufClassName,"PreviewWidget");

		WNDCLASS wndclass;
		wndclass.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | style;
		wndclass.lpfnWndProc   = _wndProc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = m_pWin32App->getInstance();
		wndclass.hIcon         = NULL;
		wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
		wndclass.lpszMenuName  = NULL;
		wndclass.lpszClassName = (LPCTSTR)m_bufClassName;

		m_atomPreviewWidgetClass = RegisterClass(&wndclass);
		UT_ASSERT(m_atomPreviewWidgetClass);
		if (!m_atomPreviewWidgetClass)
			return;
		}

	RECT rParent;
	GetClientRect(hwndParent,&rParent);
	
	m_hwndPreview = CreateWindow((LPCTSTR)m_atomPreviewWidgetClass,NULL,
								 WS_CHILD|WS_VISIBLE,
								 0,0,(rParent.right-rParent.left),(rParent.bottom-rParent.top),
								 hwndParent,NULL,m_pWin32App->getInstance(),NULL);
	UT_ASSERT(m_hwndPreview);

	// bind window back to this object.  note that this will happen
	// after WM_Create (and many other messages) have gone thru, but
	// before anything we care about.
	
	SWL(m_hwndPreview,this);

	// create a GR_Graphics for this window and HDC
	
	m_pGraphics = new GR_Win32Graphics(GetDC(m_hwndPreview),m_hwndPreview, pWin32App->getApp());
	UT_ASSERT(m_pGraphics);

	m_iInstanceCount++;
}

XAP_Win32PreviewWidget::~XAP_Win32PreviewWidget(void)
{
	// destroy the child window we created now so that we can unregister
	// the window class.  (it is ok if this fails.)
	if (m_hwndPreview)
		DestroyWindow(m_hwndPreview);
	
	m_iInstanceCount--;
	if(m_iInstanceCount == 0)
	{
		m_atomPreviewWidgetClass = NULL;
		UT_Bool bResult = UnregisterClass(m_bufClassName,m_pWin32App->getInstance());
		UT_ASSERT(bResult);
	}

	DELETEP(m_pGraphics);
}

void XAP_Win32PreviewWidget::getWindowSize(UT_uint32 * pWidth, UT_uint32 * pHeight) const
{
	RECT r;
	GetClientRect(m_hwndPreview,&r);
	if (pWidth)
		*pWidth = (r.right-r.left);
	if (pHeight)
		*pHeight = (r.bottom-r.top);
	return;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

LRESULT CALLBACK XAP_Win32PreviewWidget::_wndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// static callback
	XAP_Win32PreviewWidget * pThis = GWL(hwnd);
	if (!pThis)							// SWL() not yet called in constructor
		return DefWindowProc(hwnd,iMsg,wParam,lParam);

	// We handle all the windows messages here.  in the simplest type of
	// preview widget, all the platform & xp logic does is draw into it
	// (a view-only widget).  we send up paint events to help out with
	// this.  this is all supported in the base class.
	//
	// as we make more compilcated preview windows (with output and some
	// user input (such as clicking on the edges of a box to enable/disable
	// borders on a paragraph or cell), we may need to expand the set of
	// actions we propagate.  the intent here is that we will add virtual
	// stub functions in this base class (that do nothing) and let them
	// be overridden by the more advanced sub-classes.  having said this,
	// it sounds fairly obvious, but then most all comments just state
	// the obvious.... -- jeff
	
	switch (iMsg)
	{
	case WM_PAINT:			
		return pThis->onPaint(hwnd);
	
	case WM_LBUTTONDOWN:
		return pThis->onLeftButtonDown(LOWORD(lParam), HIWORD(lParam)); 


	default:				
		break;
	}
	
	return DefWindowProc(hwnd,iMsg,wParam,lParam);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

LRESULT XAP_Win32PreviewWidget::onPaint(HWND hwnd)
{
	UT_ASSERT(hwnd == m_hwndPreview);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	if (m_pPreview)
		m_pPreview->draw();

	EndPaint(hwnd, &ps);
	return 0;
}

LRESULT	XAP_Win32PreviewWidget::onLeftButtonDown(UT_sint32 x, UT_sint32 y)
{
	if (m_pPreview)
		m_pPreview->onLeftButtonDown(x, y);
	
	return 0;
}
