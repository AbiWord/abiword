/* AbiWord - Win32 PageNumbers Dialog
 * Copyright (C) 2001 Mike Nordell
 * Copyright (C) 2003 Jordi Mas i Hern�ndez
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32PreviewWidget.h"
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
	m_pPreviewWidget(0),
	m_hThisDlg(0)
{
}


AP_Win32Dialog_PageNumbers::~AP_Win32Dialog_PageNumbers()
{
	delete m_pPreviewWidget;
}

void AP_Win32Dialog_PageNumbers::runModal(XAP_Frame* pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == AP_DIALOG_ID_PAGE_NUMBERS);
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32LabelledSeparator_RegisterClass(pWin32App);
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCE(AP_RID_DIALOG_PAGENUMBERS));

}



BOOL AP_Win32Dialog_PageNumbers::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hThisDlg = hWnd;

	// Update the caption
	localizeDialogTitle(AP_STRING_ID_DLG_PageNumbers_Title);

	/* Localise controls*/
	localizeControlText(AP_RID_DIALOG_PAGENUMBERS_BTN_OK,			XAP_STRING_ID_DLG_OK);
	localizeControlText(AP_RID_DIALOG_PAGENUMBERS_BTN_CANCEL,		XAP_STRING_ID_DLG_Cancel);
	localizeControlText(AP_RID_DIALOG_PAGENUMBERS_STATIC_ALIGNMENT,		AP_STRING_ID_DLG_PageNumbers_Alignment);
	localizeControlText(AP_RID_DIALOG_PAGENUMBERS_STATIC_POSITION,		AP_STRING_ID_DLG_PageNumbers_Position);
	localizeControlText(AP_RID_DIALOG_PAGENUMBERS_STATIC_PREVIEW,		AP_STRING_ID_DLG_PageNumbers_Preview);
	localizeControlText(AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONLEFT,	AP_STRING_ID_DLG_PageNumbers_Left);
	localizeControlText(AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONRIGHT,	AP_STRING_ID_DLG_PageNumbers_Right);
	localizeControlText(AP_RID_DIALOG_PAGENUMBERS_RADIO_POSITIONCENTER,	AP_STRING_ID_DLG_PageNumbers_Center);
	localizeControlText(AP_RID_DIALOG_PAGENUMBERS_RADIO_ALIGNFOOTER,	AP_STRING_ID_DLG_PageNumbers_Footer);
	localizeControlText(AP_RID_DIALOG_PAGENUMBERS_RADIO_ALIGNHEADER,	AP_STRING_ID_DLG_PageNumbers_Header);
	
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

BOOL AP_Win32Dialog_PageNumbers::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const WORD	wNotifyCode	= HIWORD(wParam);
	const WORD	wId			= LOWORD(wParam);
	HWND		hWndCtrl	= (HWND)lParam;

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
	UT_ASSERT(m_hThisDlg);

	HWND hWndChild = GetDlgItem(m_hThisDlg, AP_RID_DIALOG_PAGENUMBERS_STATIC_PREVIEW_WIDGET);

	// If we don't get the preview control things are bad!
	UT_ASSERT(hWndChild);

	if (hWndChild)	// don't let it crash in non-debug builds
	{
		m_pPreviewWidget =
			new XAP_Win32PreviewWidget(	static_cast<XAP_Win32App*>(m_pApp),
										  hWndChild,
										  0);
		UT_uint32 w,h;
		m_pPreviewWidget->getWindowSize(&w, &h);
		_createPreviewFromGC(m_pPreviewWidget->getGraphics(), w, h);
		m_pPreviewWidget->setPreview(m_preview);	// make it handle WM_PAINT!
	}
}



