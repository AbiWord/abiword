 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/


#include <windows.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ev_Mouse.h"
#include "ev_Win32Mouse.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"

ev_Win32Mouse::ev_Win32Mouse(EV_EditEventMapper * pEEM)
	: EV_Mouse(pEEM)
{
	reset();
}

void ev_Win32Mouse::reset(void)
{
	m_iCaptureCount = 0;
}

void ev_Win32Mouse::onButtonDown(FV_View * pView,
								 HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems;
	UT_uint32 iPrefix;
	EV_EditEventMapperResult result;

	m_iCaptureCount++;			// keep track of number of clicks/releases
	if (m_iCaptureCount > 1)	// ignore subsequent clicks (other mouse buttons) during drag
		return;

	SetCapture(hWnd);
	m_embCaptured = emb;		// remember button which caused capture

	ems = 0;
	if (fwKeys & MK_SHIFT)
		ems |= EV_EMS_SHIFT;
	if (fwKeys & MK_CONTROL)
		ems |= EV_EMS_CONTROL;
	if (GetKeyState(VK_MENU) & 0x8000)
		ems |= EV_EMS_ALT;

	UT_DEBUGMSG(("onButtonDown: 0x%08lx [b=%d m=%d]\n",EV_EMO_SINGLECLICK|emb|ems,emb,ems));
	
	result = m_pEEM->Mouse(EV_EMO_SINGLECLICK|emb|ems, &pEM,&iPrefix);
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,iPrefix,xPos,yPos);
		return;
	case EV_EEMR_INCOMPLETE:
		// I'm not sure this makes any sense, but we allow it.
		return;
	case EV_EEMR_BOGUS_START:
	case EV_EEMR_BOGUS_CONT:
		// TODO What to do ?? Should we beep at them or just be quiet ??
		return;
	default:
		UT_ASSERT(0);
		return;
	}
}

void ev_Win32Mouse::onButtonMove(FV_View * pView,
								 HWND hWnd, WPARAM fwKeys, WPARAM xPos, WPARAM yPos)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems;
	UT_uint32 iPrefix;
	EV_EditEventMapperResult result;

	if (!m_iCaptureCount)		// ignore free movements
		return;

	ems = 0;
	if (fwKeys & MK_SHIFT)
		ems |= EV_EMS_SHIFT;
	if (fwKeys & MK_CONTROL)
		ems |= EV_EMS_CONTROL;
	if (GetKeyState(VK_MENU) & 0x8000)
		ems |= EV_EMS_ALT;

	// report movements under the mouse button that we did the capture on

	UT_DEBUGMSG(("onButtonMove: 0x%08lx [b=%d m=%d]\n",EV_EMO_DRAG|m_embCaptured|ems, m_embCaptured, ems));
	
	result = m_pEEM->Mouse(EV_EMO_DRAG|m_embCaptured|ems, &pEM,&iPrefix);
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,iPrefix,xPos,yPos);
		return;
	case EV_EEMR_INCOMPLETE:
		// I'm not sure this makes any sense, but we allow it.
		return;
	case EV_EEMR_BOGUS_START:
	case EV_EEMR_BOGUS_CONT:
		// TODO What to do ?? Should we beep at them or just be quiet ??
		return;
	default:
		UT_ASSERT(0);
		return;
	}
}

void ev_Win32Mouse::onButtonUp(FV_View * pView,
							   HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems;
	UT_uint32 iPrefix;
	EV_EditEventMapperResult result;

	if (m_iCaptureCount > 0)
		m_iCaptureCount--;

	if (emb != m_embCaptured)	// ignore other button releases
		return;
	ReleaseCapture();
	m_iCaptureCount==0;

	ems = 0;
	if (fwKeys & MK_SHIFT)
		ems |= EV_EMS_SHIFT;
	if (fwKeys & MK_CONTROL)
		ems |= EV_EMS_CONTROL;
	if (GetKeyState(VK_MENU) & 0x8000)
		ems |= EV_EMS_ALT;

	UT_DEBUGMSG(("onButtonUp  : 0x%08lx [b=%d m=%d]\n",EV_EMO_RELEASE|m_embCaptured|ems, m_embCaptured, ems));

	result = m_pEEM->Mouse(EV_EMO_RELEASE|m_embCaptured|ems, &pEM,&iPrefix);
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,iPrefix,xPos,yPos);
		return;
	case EV_EEMR_INCOMPLETE:
		// I'm not sure this makes any sense, but we allow it.
		return;
	case EV_EEMR_BOGUS_START:
	case EV_EEMR_BOGUS_CONT:
		// TODO What to do ?? Should we beep at them or just be quiet ??
		return;
	default:
		UT_ASSERT(0);
		return;
	}
}

void ev_Win32Mouse::onDoubleClick(FV_View * pView,
								 HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems;
	UT_uint32 iPrefix;
	EV_EditEventMapperResult result;

#if 0
	m_iCaptureCount++;			// keep track of number of clicks/releases
	if (m_iCaptureCount > 1)	// ignore subsequent clicks (other mouse buttons) during drag
		return;

	SetCapture(hWnd);
	m_embCaptured = emb;		// remember button which caused capture
#endif

	ems = 0;
	if (fwKeys & MK_SHIFT)
		ems |= EV_EMS_SHIFT;
	if (fwKeys & MK_CONTROL)
		ems |= EV_EMS_CONTROL;
	if (GetKeyState(VK_MENU) & 0x8000)
		ems |= EV_EMS_ALT;

	UT_DEBUGMSG(("onDoubleClick: 0x%08lx [b=%d m=%d]\n",EV_EMO_DOUBLECLICK|emb|ems,emb,ems));
	
	result = m_pEEM->Mouse(EV_EMO_DOUBLECLICK|emb|ems, &pEM,&iPrefix);
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,iPrefix,xPos,yPos);
		return;
	case EV_EEMR_INCOMPLETE:
		// I'm not sure this makes any sense, but we allow it.
		return;
	case EV_EEMR_BOGUS_START:
	case EV_EEMR_BOGUS_CONT:
		// TODO What to do ?? Should we beep at them or just be quiet ??
		return;
	default:
		UT_ASSERT(0);
		return;
	}
}
