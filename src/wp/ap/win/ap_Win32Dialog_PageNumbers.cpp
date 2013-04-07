/* AbiWord - Win32 PageNumbers Dialog
 * Copyright (C) 2001 Mike Nordell
 * Copyright (C) 2003 Jordi Mas i Hernàndez
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
#include "xap_Win32PreviewWidget.h"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32LabelledSeparator.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_PageNumbers.h"
#include "ap_Win32Dialog_PageNumbers.h"

#include "gr_Win32Graphics.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Win32Resources.rc2"

#include "fv_View.h"

#ifdef _MSC_VER
// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif


/*****************************************************************/

XAP_Dialog* AP_Win32Dialog_PageNumbers::static_constructor(XAP_DialogFactory* pFactory, XAP_Dialog_Id id)
{
	AP_Win32Dialog_PageNumbers* p = new AP_Win32Dialog_PageNumbers(pFactory,id);
	return p;
}

AP_Win32Dialog_PageNumbers::AP_Win32Dialog_PageNumbers(XAP_DialogFactory* pDlgFactory, XAP_Dialog_Id id)
:	AP_Dialog_PageNumbers(pDlgFactory,id),
	m_hThisDlg(0),
	m_helper(this),
	m_pPreviewWidget(0)
{
}


AP_Win32Dialog_PageNumbers::~AP_Win32Dialog_PageNumbers()
{
	delete m_pPreviewWidget;
}

void AP_Win32Dialog_PageNumbers::runModal(XAP_Frame* pFrame)
{
	UT_return_if_fail (pFrame);
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
 
 	XAP_Win32LabelledSeparator_RegisterClass(pWin32App);
	createModal(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_PAGENUMBERS));
}


#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))


BOOL AP_Win32Dialog_PageNumbers::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_hThisDlg = hWnd;

	const XAP_StringSet* pSS = m_pApp->getStringSet();

	// Update the caption
	setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Title));

	/* Localise controls*/
	_DSX(PAGENUMBERS_BTN_OK,				DLG_OK);
	_DSX(PAGENUMBERS_BTN_CANCEL,			DLG_Cancel);
	_DS(PAGENUMBERS_STATIC_ALIGNMENT,		DLG_PageNumbers_Alignment);
	_DS(PAGENUMBERS_STATIC_POSITION,		DLG_PageNumbers_Position);
	_DS(PAGENUMBERS_STATIC_PREVIEW,			DLG_PageNumbers_Preview);
	
	_DS(PAGENUMBERS_RADIO_POSITIONLEFT,		DLG_PageNumbers_Left);
	_DS(PAGENUMBERS_RADIO_POSITIONRIGHT,	DLG_PageNumbers_Right);
	_DS(PAGENUMBERS_RADIO_POSITIONCENTER,	DLG_PageNumbers_Center);
	_DS(PAGENUMBERS_RADIO_ALIGNFOOTER,		DLG_PageNumbers_Footer);
	_DS(PAGENUMBERS_RADIO_ALIGNHEADER,		DLG_PageNumbers_Header);
	
	/*Set Default Radio buttons */	
	CheckRadioButton(hWnd, AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONLEFT, AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONCENTER,
		AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONCENTER);
		
	CheckRadioButton(hWnd, AP_RID_DIALOG_PAGENUMBERS_RADIO_ALIGNFOOTER, AP_RID_DIALOG_PAGENUMBERS_RADIO_ALIGNHEADER,
		AP_RID_DIALOG_PAGENUMBERS_RADIO_ALIGNFOOTER);

	_createPreviewWidget();
	m_pPreviewWidget->getGraphics()->init3dColors();
	_updatePreview(m_align, m_control);
	
	centerDialog();

	return 1;	// 0 == we called SetFocus()
}

BOOL AP_Win32Dialog_PageNumbers::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	const WORD	wId			= LOWORD(wParam);

	switch (wId)
	{
		case IDCANCEL:
			m_answer = a_CANCEL;
			EndDialog(hWnd,0);
			return 1;

		case IDOK:
			m_answer = a_OK;
			EndDialog(hWnd,0);
			return 1;
			
		/* Position */			
		
		case AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONLEFT:
		{
			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONLEFT))
				m_align = id_LALIGN;
				
			_updatePreview(m_align, m_control);
			break;
		}
		
		case AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONRIGHT:
		{
			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONRIGHT))
				m_align = id_RALIGN;
			
			_updatePreview(m_align, m_control);
			break;
		}
		
		case AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONCENTER:
		{
			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONCENTER))
				m_align = id_CALIGN;			
				
			_updatePreview(m_align, m_control);
			break;
		}
		
		/* Align */			
		
		case AP_RID_DIALOG_PAGENUMBERS_RADIO_ALIGNFOOTER:
		{
			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_PAGENUMBERS_RADIO_ALIGNFOOTER))
				m_control = id_FTR;
				
			_updatePreview(m_align, m_control);
			break;
		}
		
		case AP_RID_DIALOG_PAGENUMBERS_RADIO_ALIGNHEADER:
		{
			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_PAGENUMBERS_RADIO_ALIGNHEADER))
				m_control = id_HDR;
				
			_updatePreview(m_align, m_control);
			break;
		}

		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
	}
	return 0;						// return zero to let windows take care of it.
}

void AP_Win32Dialog_PageNumbers::_createPreviewWidget()
{
	UT_return_if_fail (m_hThisDlg);

	HWND hWndChild = GetDlgItem(m_hThisDlg, AP_RID_DIALOG_PAGENUMBERS_STATIC_PREVIEW_WIDGET);

	// If we don't get the preview control things are bad!
	UT_return_if_fail (hWndChild);

	m_pPreviewWidget =
		new XAP_Win32PreviewWidget(	static_cast<XAP_Win32App*>(m_pApp),
									hWndChild,
									0);
	UT_uint32 w,h;
	m_pPreviewWidget->getWindowSize(&w, &h);
	_createPreviewFromGC(m_pPreviewWidget->getGraphics(), w, h);
	m_pPreviewWidget->setPreview(m_preview);	// make it handle WM_PAINT!
}



