/* AbiWord
 * Copyright (C) 1998-2002 AbiSource, Inc.
 * Copyright (C) 1998-2002 
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

#include "ap_Win32Frame.h"
#include "xap_Win32FrameImpl.h"
#include "ut_debugmsg.h"

AP_Win32Frame::AP_Win32Frame(XAP_Win32App * app) :
	AP_Frame(new AP_Win32FrameImpl(this), app)
{
}

AP_Win32Frame::AP_Win32Frame(AP_Win32Frame * f) :
	AP_Frame(static_cast<AP_Frame *>(f))
{
}

AP_Win32Frame::~AP_Win32Frame(void)
{
}

XAP_Frame* AP_Win32Frame::cloneFrame(void)
{
	AP_Win32Frame* pClone = new AP_Win32Frame(this);

	UT_ASSERT(pClone);
	return pClone;
}

bool AP_Win32Frame::initialize(XAP_FrameMode frameMode)
{
#if 0
	if (!initFrameData())
		return false;

	if (!XAP_Win32Frame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
									AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
									AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet,
									AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
									AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet))
		return false;

	_createTopLevelWindow();
	_showOrHideToolbars();
	_showOrHideStatusbar();
#endif
	return true;
}

void AP_Win32Frame::toggleTopRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData*>(getFrameData());
	UT_ASSERT(pFrameData);

#if 0
	if (bRulerOn)
	{
		UT_ASSERT(!pFrameData->m_pTopRuler);

		_createTopRuler();

		static_cast<AP_FrameData*>(m_pData)->m_pTopRuler->setView(m_pView, getZoomPercentage());
	}
	else
	{
		// delete the actual widgets
		if (m_hwndTopRuler)
			DestroyWindow(m_hwndTopRuler);

		DELETEP(static_cast<AP_FrameData*>(m_pData)->m_pTopRuler);

		m_hwndTopRuler = NULL;
	}

	// repack the child windows
	RECT r;
	GetClientRect(m_hwndContainer, &r);
	_onSize(r.right - r.left, r.bottom - r.top);
#endif
}

void AP_Win32Frame::toggleLeftRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData*>(getFrameData());
	UT_ASSERT(pFrameData);

#if 0
	if (bRulerOn)
	{
		//
		// If There is an old ruler just return
		//
		if(m_hwndLeftRuler)
		{
			return;
		}
		UT_ASSERT(!pFrameData->m_pLeftRuler);

		_createLeftRuler();

		static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler->setView(m_pView, getZoomPercentage());
	}
	else
	{
		// delete the actual widgets
		if (m_hwndLeftRuler)
			DestroyWindow(m_hwndLeftRuler);

		DELETEP(static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler);

		m_hwndLeftRuler = NULL;
	}

	// repack the child windows
	RECT r;
	GetClientRect(m_hwndContainer, &r);
	_onSize(r.right - r.left, r.bottom - r.top);
#endif
}

/************** helper methods for _showDocument ************************/
bool AP_Win32Frame::_createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom)
{
	return false;
}

bool AP_Win32Frame::_createScrollBarListeners(AV_View * pView, AV_ScrollObj *& pScrollObj, 
				       ap_ViewListener *& pViewListener, 
				       ap_Scrollbar_ViewListener *& pScrollbarViewListener,
				       AV_ListenerId &lid, 
				       AV_ListenerId &lidScrollbarViewListener)
{
	return false;
}

void AP_Win32Frame::_bindToolbars(AV_View *pView)
{
}

void AP_Win32Frame::_setViewFocus(AV_View *pView)
{
}

/*** helper methods for helper methods for _showDocument (meta-helper-methods?) :-) ***/
UT_sint32 AP_Win32Frame::_getDocumentAreaWidth(void)
{
	return 0;
}

UT_sint32 AP_Win32Frame::_getDocumentAreaHeight(void)
{
	return 0;
}

