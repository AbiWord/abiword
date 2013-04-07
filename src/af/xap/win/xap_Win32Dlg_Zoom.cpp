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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xav_View.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Zoom.h"
#include "xap_Win32Dlg_Zoom.h"

#include "xap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_Zoom::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	XAP_Win32Dialog_Zoom * p = new XAP_Win32Dialog_Zoom(pFactory,id);
	return p;
}

XAP_Win32Dialog_Zoom::XAP_Win32Dialog_Zoom(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: XAP_Dialog_Zoom(pDlgFactory,id)
{
	m_bEditPctEnabled = false;
}

XAP_Win32Dialog_Zoom::~XAP_Win32Dialog_Zoom(void)
{

}

/*****************************************************************/

void XAP_Win32Dialog_Zoom::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;

	/*
	  This dialog is non-persistent.
	  
	  This dialog should do the following:

	  - Construct itself to represent the base-class zoomTypes
	    z_200, z_100, z_75, z_PageWidth, z_WholePage, and z_Percent.
		The Unix one looks just like Microsoft Word 97, with the preview
		and all (even though it's not hooked up yet).

	  - Set zoom type to match "m_zoomType" and value of radio button
	    to match "m_zoomPercent".

	  On "OK" (or during user-interaction) the dialog should:

	  - Save the zoom type to "m_zoomType".
	  
	  - Save the value in the Percent spin button box to "m_zoomPercent".

	  - Just quit, the data items will be ignored by the caller.

	*/

	// raise the dialog

	UT_ASSERT(m_id == XAP_DIALOG_ID_ZOOM);
	
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCEW(XAP_RID_DIALOG_ZOOM));

}

BOOL XAP_Win32Dialog_Zoom::_onDlgMessage(HWND /*hWnd*/,UINT msg,WPARAM wParam,LPARAM /*lParam*/)
{
	switch (msg)
	{
		
	case WM_VSCROLL:
		_updatePreviewZoomPercent((UT_uint32)HIWORD(wParam));
		return 1;

	default:
		return 0;
	}
}

#define _DS(c,s)	setDlgItemText(XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_Zoom::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{		
	const XAP_StringSet* pSS = m_pApp->getStringSet();

	// Update the caption
	setDialogTitle(pSS->getValue(XAP_STRING_ID_DLG_Zoom_ZoomTitle));

	// localize controls
	_DSX(ZOOM_BTN_CLOSE,		DLG_Close);
	_DS(ZOOM_TEXT_ZOOMTO,		DLG_Zoom_RadioFrameCaption);
	_DS(ZOOM_RADIO_200,		DLG_Zoom_200);
	_DS(ZOOM_RADIO_100,		DLG_Zoom_100);
	_DS(ZOOM_RADIO_75,		DLG_Zoom_75);
	_DS(ZOOM_RADIO_WIDTH,		DLG_Zoom_PageWidth);
	_DS(ZOOM_RADIO_WHOLE,		DLG_Zoom_WholePage);
	_DS(ZOOM_RADIO_PCT,		DLG_Zoom_Percent);

	// set initial state
	checkButton(XAP_RID_DIALOG_ZOOM_RADIO_200 + m_zoomType);
	setControlInt(XAP_RID_DIALOG_ZOOM_EDIT_PCT, (UINT) m_zoomPercent);
	m_bEditPctEnabled = ((XAP_RID_DIALOG_ZOOM_RADIO_200 + m_zoomType) == XAP_RID_DIALOG_ZOOM_RADIO_PCT);
	enableControl(XAP_RID_DIALOG_ZOOM_EDIT_PCT ,m_bEditPctEnabled);
	
	SendMessageW(GetDlgItem(hWnd,XAP_RID_DIALOG_ZOOM_SPIN_PCT),UDM_SETRANGE,
				(WPARAM)0,(LPARAM)MAKELONG(XAP_DLG_ZOOM_MAXIMUM_ZOOM,XAP_DLG_ZOOM_MINIMUM_ZOOM));
		
	_updatePreviewZoomPercent(getZoomPercent());
	
	centerDialog();	
	
	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_Zoom::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);
	UT_sint32 newValue;

	m_bEditPctEnabled = (isChecked(XAP_RID_DIALOG_ZOOM_RADIO_PCT)==BST_CHECKED);
	enableControl(XAP_RID_DIALOG_ZOOM_EDIT_PCT,m_bEditPctEnabled);
	
	switch (wId)
	{
	case XAP_RID_DIALOG_ZOOM_RADIO_200:
		m_zoomType = XAP_Frame::z_200;
		m_pFrame->setZoomType(m_zoomType);
		_updatePreviewZoomPercent(200);
		return 1;

	case XAP_RID_DIALOG_ZOOM_RADIO_100:
		m_zoomType = XAP_Frame::z_100;
		m_pFrame->setZoomType(m_zoomType);
		_updatePreviewZoomPercent(100);
		return 1;

	case XAP_RID_DIALOG_ZOOM_RADIO_75:
		m_zoomType = XAP_Frame::z_75;
		m_pFrame->setZoomType(m_zoomType);
		_updatePreviewZoomPercent(75);
		return 1;

	case XAP_RID_DIALOG_ZOOM_RADIO_WIDTH:
		newValue = m_pFrame->getCurrentView()->calculateZoomPercentForPageWidth();
		m_zoomType = XAP_Frame::z_PAGEWIDTH;
		m_pFrame->setZoomType(m_zoomType);
		_updatePreviewZoomPercent(newValue);
		return 1;

	case XAP_RID_DIALOG_ZOOM_RADIO_WHOLE:
		newValue = m_pFrame->getCurrentView()->calculateZoomPercentForWholePage();
		m_zoomType = XAP_Frame::z_WHOLEPAGE;
		m_pFrame->setZoomType(m_zoomType);
		_updatePreviewZoomPercent(newValue);
		return 1;

	case XAP_RID_DIALOG_ZOOM_RADIO_PCT:
		if (m_bEditPctEnabled)
		{
			if (_getValueFromEditPct(&newValue))
			{
				m_zoomType = XAP_Frame::z_PERCENT;
				m_pFrame->setZoomType(m_zoomType);
				m_zoomPercent = newValue;
				_updatePreviewZoomPercent(newValue);
			}
		}
		return 1;

	case XAP_RID_DIALOG_ZOOM_EDIT_PCT:
		if (_getValueFromEditPct(&newValue))
		{
			m_zoomPercent = newValue;
			_updatePreviewZoomPercent(newValue);
		}
		return 1;
			
	case IDCANCEL:						// Zoom is instant-apply, so treat cancel as OK
	case IDOK:							// also XAP_RID_DIALOG_ZOOM_BTN_CLOSE
		m_answer = a_OK; 
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL XAP_Win32Dialog_Zoom::_onDeltaPos(NM_UPDOWN * pnmud)
{
	// respond to WM_NOTIFY/UDN_DELTAPOS message
	// return TRUE to prevent the change from happening
	// return FALSE to allow it to occur
	// we may alter the change by changing the fields in pnmud.

	UT_ASSERT(pnmud->hdr.idFrom == XAP_RID_DIALOG_ZOOM_SPIN_PCT);
	UT_DEBUGMSG(("onDeltaPos: [idFrom %d][iPos %d][iDelta %d]\n",
				 pnmud->hdr.idFrom,pnmud->iPos,pnmud->iDelta));
	int iNew = pnmud->iPos + pnmud->iDelta;
	
	if ((pnmud->iPos < XAP_DLG_ZOOM_MINIMUM_ZOOM) || (iNew < XAP_DLG_ZOOM_MINIMUM_ZOOM))
	{
		// bogus current position or bogus delta, force it back to minimum.
		pnmud->iDelta = XAP_DLG_ZOOM_MINIMUM_ZOOM - pnmud->iPos;
		return FALSE;
	}
	
	if ((pnmud->iPos > XAP_DLG_ZOOM_MAXIMUM_ZOOM) || (iNew > XAP_DLG_ZOOM_MAXIMUM_ZOOM))
	{
		// arbitrary upper limit (XAP_DLG_ZOOM_MAXIMUM_ZOOM)
		// i did this because massive scaling of fonts
		// causes problems for virtual memory (when they
		// get rendered at huge sizes).
		// MSWORD97 also applies this limit.

		pnmud->iDelta = XAP_DLG_ZOOM_MAXIMUM_ZOOM - pnmud->iPos;
		return FALSE;
	}

	return FALSE;
}

BOOL XAP_Win32Dialog_Zoom::_getValueFromEditPct(int * pNewValue)
{
	int newValue = getControlInt(XAP_RID_DIALOG_ZOOM_EDIT_PCT);

	// valid number, but out of range clip to MIN or MAX range;
	if (newValue < XAP_DLG_ZOOM_MINIMUM_ZOOM) newValue = XAP_DLG_ZOOM_MINIMUM_ZOOM;
	if (newValue > XAP_DLG_ZOOM_MAXIMUM_ZOOM) newValue = XAP_DLG_ZOOM_MAXIMUM_ZOOM;

	*pNewValue = newValue;
	return TRUE;
}

