/* AbiSource Program Utilities
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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ev_Mouse.h"
#include "ev_Win32Mouse.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "xav_View.h"

EV_Win32Mouse::EV_Win32Mouse(EV_EditEventMapper * pEEM)
	: EV_Mouse(pEEM)
{
	m_clickState = 0;					// no click
	m_contextState = 0;

	reset();
}

void EV_Win32Mouse::reset(void)
{
	m_iCaptureCount = 0;
}

void EV_Win32Mouse::onButtonDown(AV_View * pView,
								 HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems;
	EV_EditEventMapperResult result;
	EV_EditMouseContext emc = 0;

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

	short x = (unsigned short) xPos;
	short y = (unsigned short) yPos;

	emc = pView->getMouseContext(x,y);

	m_clickState = EV_EMO_SINGLECLICK;	// remember which type of click
	m_contextState = emc;				// remember contect of click
	
	result = m_pEEM->Mouse(emc|EV_EMO_SINGLECLICK|emb|ems, &pEM);
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,x,y);
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

void EV_Win32Mouse::onButtonMove(AV_View * pView,
								 HWND hWnd, WPARAM fwKeys, WPARAM xPos, WPARAM yPos)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems;
	EV_EditEventMapperResult result;
	EV_EditMouseOp mop;
	EV_EditMouseButton emb = 0;
	EV_EditMouseContext emc = 0;

	ems = 0;
	if (fwKeys & MK_SHIFT)
		ems |= EV_EMS_SHIFT;
	if (fwKeys & MK_CONTROL)
		ems |= EV_EMS_CONTROL;
	if (GetKeyState(VK_MENU) & 0x8000)
		ems |= EV_EMS_ALT;

	if (m_iCaptureCount)
		emb = m_embCaptured;
	else
		emb = EV_EMB_BUTTON0;

	short x = (unsigned short) xPos;
	short y = (unsigned short) yPos;

	if (m_clickState == 0)
	{
		mop = EV_EMO_DRAG;
		emc = pView->getMouseContext(x,y);
	}
	else if (m_clickState == EV_EMO_SINGLECLICK)
	{
		mop = EV_EMO_DRAG;
		emc = m_contextState;
	}
	else if (m_clickState == EV_EMO_DOUBLECLICK)
	{
		mop = EV_EMO_DOUBLEDRAG;
		emc = m_contextState;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	result = m_pEEM->Mouse(emc|mop|emb|ems, &pEM);
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,x,y);
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

void EV_Win32Mouse::onButtonUp(AV_View * pView,
							   HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems;
	EV_EditEventMapperResult result;
	EV_EditMouseOp mop;
	EV_EditMouseContext emc = 0;

	if (m_iCaptureCount > 0)
		m_iCaptureCount--;

	if (emb != m_embCaptured)	// ignore other button releases
		return;
	ReleaseCapture();
	m_iCaptureCount=0;

	ems = 0;
	if (fwKeys & MK_SHIFT)
		ems |= EV_EMS_SHIFT;
	if (fwKeys & MK_CONTROL)
		ems |= EV_EMS_CONTROL;
	if (GetKeyState(VK_MENU) & 0x8000)
		ems |= EV_EMS_ALT;

	short x = (unsigned short) xPos;
	short y = (unsigned short) yPos;

	mop = EV_EMO_RELEASE;
	if (m_clickState == EV_EMO_DOUBLECLICK)
		mop = EV_EMO_DOUBLERELEASE;
	m_clickState = 0;

	emc = m_contextState;
	result = m_pEEM->Mouse(emc|mop|m_embCaptured|ems, &pEM);
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,x,y);
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

void EV_Win32Mouse::onDoubleClick(AV_View * pView,
								 HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems;
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

	short x = (unsigned short) xPos;
	short y = (unsigned short) yPos;

	EV_EditMouseContext emc = m_contextState;
	m_clickState = EV_EMO_DOUBLECLICK;

	result = m_pEEM->Mouse(emc|EV_EMO_DOUBLECLICK|emb|ems, &pEM);
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,x,y);
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
