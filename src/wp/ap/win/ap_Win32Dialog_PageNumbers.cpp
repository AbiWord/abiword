/* AbiWord - Win32 PageNumbers Dialog
 * Copyright (C) 2001 Mike Nordell
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
#include "xap_Win32Frame.h"
#include "xap_Win32PreviewWidget.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_PageNumbers.h"
#include "ap_Win32Dialog_PageNumbers.h"

#include "ap_Win32Resources.rc2"

#include "fv_View.h"

#ifdef _MSC_VER
#define for if (0) {} else for
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
	m_helper(this),
	m_pPreviewWidget(0),
	m_hThisDlg(0)
{
	// Bloody baseclasses not intitializing their own bloody data!!! Hate!
	m_align		= id_CALIGN;
	m_control	= id_FTR;
}


AP_Win32Dialog_PageNumbers::~AP_Win32Dialog_PageNumbers()
{
	delete m_pPreviewWidget;
}

void AP_Win32Dialog_PageNumbers::runModal(XAP_Frame* pFrame)
{
	UT_ASSERT(pFrame);

	// raise the dialog
	m_helper.runModal(pFrame, AP_DIALOG_ID_PAGE_NUMBERS, AP_RID_DIALOG_PAGENUMBERS, this);
}


BOOL AP_Win32Dialog_PageNumbers::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hThisDlg = hWnd;

	const XAP_StringSet* pSS = m_pApp->getStringSet();

	// Update the caption
	m_helper.setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Title));

#define LOC_CTRL_STR_PAIR(x,y)	\
	AP_RID_DIALOG_PAGENUMBERS_ ## x, AP_STRING_ID_DLG_PageNumbers_ ## y

	struct {
		UT_sint32		m_idControl;
		XAP_String_Id	m_idString;
	} static const mappings[] =
	{
		LOC_CTRL_STR_PAIR(COMBO_POSITION, Header),
		LOC_CTRL_STR_PAIR(COMBO_POSITION, Footer),
		LOC_CTRL_STR_PAIR(COMBO_ALIGN, Left),
		LOC_CTRL_STR_PAIR(COMBO_ALIGN, Right),
		LOC_CTRL_STR_PAIR(COMBO_ALIGN, Center)
	};
#undef LOC_CTRL_STR_PAIR

	for (int i=0; i < NrElements(mappings); ++i)
	{
		m_helper.addItemToCombo(mappings[i].m_idControl,
								pSS->getValue(mappings[i].m_idString));
	}

	// default to footer, center
	m_helper.selectComboItem(AP_RID_DIALOG_PAGENUMBERS_COMBO_POSITION, 1);
	m_helper.selectComboItem(AP_RID_DIALOG_PAGENUMBERS_COMBO_ALIGN, 2);

	// localize controls
#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
	_DS(PAGENUMBERS_STATIC_ALIGNMENT,		DLG_PageNumbers_Alignment);
	_DS(PAGENUMBERS_STATIC_POSITION,		DLG_PageNumbers_Position);
	_DS(PAGENUMBERS_STATIC_PREVIEW,			DLG_PageNumbers_Preview);
#undef _DS

	_createPreviewWidget();
	_updatePreview(m_align, m_control);

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

		case AP_RID_DIALOG_PAGENUMBERS_COMBO_POSITION:
			if (wNotifyCode == CBN_SELCHANGE)
			{
				switch (m_helper.getComboSelectedIndex(AP_RID_DIALOG_PAGENUMBERS_COMBO_POSITION))
				{
					case 0:
						m_control = id_HDR;
						break;
					case 1:
						m_control = id_FTR;
						break;
					default:
						UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				}
				_updatePreview(m_align, m_control);
			}
			break;

		case AP_RID_DIALOG_PAGENUMBERS_COMBO_ALIGN:
			if (wNotifyCode == CBN_SELCHANGE)
			{
				switch (m_helper.getComboSelectedIndex(AP_RID_DIALOG_PAGENUMBERS_COMBO_ALIGN))
				{
					case 0:
						m_align = id_LALIGN;
						break;
					case 1:
						m_align = id_RALIGN;
						break;
					case 2:
						m_align = id_CALIGN;
						break;
					default:
						UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				}
				_updatePreview(m_align, m_control);
			}
			break;

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

