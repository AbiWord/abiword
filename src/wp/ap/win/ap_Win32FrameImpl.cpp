/* AbiWord
 * Copyright (C) 1998-2002 AbiSource, Inc.
 * Copyright (C) 2002
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

#include "ap_Win32FrameImpl.h"

AP_Win32FrameImpl::AP_Win32FrameImpl(AP_Frame *pFrame) :
	XAP_Win32FrameImpl(static_cast<XAP_Frame *>(pFrame)),
	m_hwndDocument(NULL)
{
}

XAP_FrameImpl * AP_Win32FrameImpl::createInstance(XAP_Frame *pFrame, XAP_App *pApp)
{
	XAP_FrameImpl *pFrameImpl = new AP_Win32FrameImpl(static_cast<AP_Frame *>(pFrame));

	UT_ASSERT(pFrameImpl);

	return pFrameImpl;
}

void AP_Win32FrameImpl::_createToolbars() 
{
	// TODO: currently does nothing
}

void AP_Win32FrameImpl::_refillToolbarsInFrameData() 
{
	// TODO: currently does nothing
}

void AP_Win32FrameImpl::_rebuildToolbar(UT_uint32 ibar)
{
	// TODO: currently does nothing
}

void AP_Win32FrameImpl::_translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{
	UT_return_if_fail(m_hwndDocument);

	// translate the given document mouse coordinates into absolute screen coordinates.

	POINT pt = { x, y };
	ClientToScreen(m_hwndDocument,&pt);
	x = pt.x;
	y = pt.y;
}

