/* AbiSource Application Framework
 * Copyright (C) 2001 Hubert Figuiere
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


#include <Carbon/Carbon.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_MacToolbar.h"
#include "xap_MacToolbar_Control.h"
#include "xap_MacFrame.h"

int XAP_MacToolbar_Control::m_instanceCount = 0;

XAP_MacToolbar_Control::XAP_MacToolbar_Control ()
	: m_window (NULL)
{
	UT_ASSERT (m_instanceCount == 0);
	m_pMacFrame = NULL;
	
	if (m_instanceCount == 0) {
		UT_DEBUGMSG (("creating XAP_MacToolbar_Control\n"));
		
		ControlRef rootControl;
		OSStatus err;
		Rect bounds;
		SInt16 top = ::GetMBarHeight ();
		// TODO find the appropriate width and menu bar height
		::SetRect (&bounds, 0, top, 1024, top + 40);
		err = CreateNewWindow (kToolbarWindowClass, 
		                       kWindowStandardHandlerAttribute, 
							   &bounds, &m_window);
		UT_ASSERT (err == noErr);
		if (err) {
			UT_DEBUGMSG (("err = %d\n", err));
		}
		m_instanceCount++;
		err = ::CreateRootControl (m_window, &rootControl);
		UT_ASSERT (err == noErr);
		if (err) {
			UT_DEBUGMSG (("err = %d\n", err));
		}		
	}
}


XAP_MacToolbar_Control::~XAP_MacToolbar_Control ()
{
	if (m_window) {
		::DisposeWindow (m_window);
	}
}


void
XAP_MacToolbar_Control::show ()
{
	UT_ASSERT (m_window);
	UT_DEBUGMSG (("show XAP_MacToolbar_Control\n"));
	::ShowWindow (m_window);
}


/*!
	Set the current frame whose toolbar should be displayed
 */
void 
XAP_MacToolbar_Control::setToolbar (XAP_MacFrame * frame)
{
	UT_uint32 c, i;
	SInt16 height = 0;
	OSStatus err;
	ControlHandle toolbarControl;

	if (frame == m_pMacFrame) {
		UT_DEBUGMSG (("XAP_MacToolbar_Control::setToolbar: Already that frame !!\n"));
		return;
	}
	if (m_pMacFrame) {
		// remove the controls
		c = m_pMacFrame->_getToolbarCount();
		for (i = 0; i < c; i++) {
			toolbarControl = ((EV_MacToolbar *)m_pMacFrame->getToolbar(i))->getControl();
			::HideControl (toolbarControl);
		}
	}
	toolbarControl = NULL;
	
	m_pMacFrame = frame;
	c = m_pMacFrame->_getToolbarCount();
	for (i = 0; i < c; i++) {
		if (toolbarControl != NULL) {
			Rect r;
			::GetControlBounds (toolbarControl, &r);
			height += r.bottom - r.top;
			xxx_UT_DEBUGMSG (("height is %d\n", height));
		}
		else {
			height = 0;
		}
		toolbarControl = ((EV_MacToolbar *)m_pMacFrame->getToolbar(i))->getControl();
		if (height) {
			::MoveControl (toolbarControl, 0, height);
		}
		::ShowControl (toolbarControl);
	}
	if (toolbarControl) {
		Rect rWin, rCtrl;
		OSStatus err;
		::GetControlBounds (toolbarControl, &rCtrl);
		err = ::GetWindowBounds (m_window, kWindowContentRgn, &rWin);
		UT_ASSERT (err == noErr);
		rWin.bottom = rWin.top + rCtrl.bottom;
		xxx_UT_DEBUGMSG (("rCtrl.bottom is %d\n", rCtrl.bottom));
		err = ::SetWindowBounds (m_window, kWindowContentRgn, &rWin);
		UT_ASSERT (err == noErr);
	}
	
	// TODO check the position of the frame window and move it away if needed.
}


void
XAP_MacToolbar_Control::requestToolbarRect (Rect & r) const
{
	::SetRect (&r, 0, 0, 1024, 40);
}
