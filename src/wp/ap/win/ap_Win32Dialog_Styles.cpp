/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Styles.h"
#include "ap_Win32Dialog_Styles.h"

#include "ap_Win32Resources.rc2"

#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fv_View.h"
#include "pd_Style.h"
#include "ut_string_class.h"
#include "gr_Win32Graphics.h"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Styles::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_Styles * p = new AP_Win32Dialog_Styles(pFactory,id);
	return p;
}

AP_Win32Dialog_Styles::AP_Win32Dialog_Styles(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Styles(pDlgFactory,id), _win32Dialog(this), m_whichType(AP_Win32Dialog_Styles::USED_STYLES)
{
}

AP_Win32Dialog_Styles::~AP_Win32Dialog_Styles(void)
{
}

void AP_Win32Dialog_Styles::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
//
// Get View and Document pointers. Place them in member variables
//

	setFrame(pFrame);
	setView((FV_View *) pFrame->getCurrentView());
	UT_ASSERT(getView());

	setDoc(getView()->getLayout()->getDocument());

	UT_ASSERT(getDoc());


	// raise the dialog

	_win32Dialog.runModal(pFrame, AP_DIALOG_ID_STYLES, AP_RID_DIALOG_STYLES_TOP, this);

	if(m_answer == AP_Dialog_Styles::a_OK)
	{
		getDoc()->updateDocForStyleChange(getCurrentStyle(),true);
		getView()->getCurrentBlock()->setNeedsRedraw();
		getDoc()->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
	}

}

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Styles::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);
	UT_ASSERT(app);

//	m_hThisDlg = hWnd;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_Styles_StylesTitle));

	// localize controls
	_DSX(STYLES_TOP_BUTTON_OK,					DLG_OK);
	_DSX(STYLES_TOP_BUTTON_CANCEL,				DLG_Cancel);

	// Set the list combo.

	_win32Dialog.addItemToCombo(AP_RID_DIALOG_STYLES_TOP_COMBO_LIST, 
									pSS->getValue (AP_STRING_ID_DLG_Styles_LBL_InUse));
	_win32Dialog.addItemToCombo(AP_RID_DIALOG_STYLES_TOP_COMBO_LIST, 
									pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_All));
	_win32Dialog.addItemToCombo(AP_RID_DIALOG_STYLES_TOP_COMBO_LIST,
									pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_UserDefined));
	_win32Dialog.selectComboItem(AP_RID_DIALOG_STYLES_TOP_COMBO_LIST, (int)m_whichType);
	
/*
	_DS(COLUMN_GROUP1,					DLG_Column_Number);
	_DS(COLUMN_GROUP2,					DLG_Column_Preview);
	_DS(COLUMN_TEXT_ONE,				DLG_Column_One);
	_DS(COLUMN_TEXT_TWO,				DLG_Column_Two);
	_DS(COLUMN_TEXT_THREE,				DLG_Column_Three);
	_DS(COLUMN_CHECK_LINE_BETWEEN,		DLG_Column_Line_Between);
*/

	// Create a preview windows.

	HWND hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_STYLES_TOP_TEXT_PARAGRAPH_PREVIEW);

	m_pParaPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
													  hwndChild,
													  0);
	UT_uint32 w,h;
	m_pParaPreviewWidget->getWindowSize(&w,&h);
	_createParaPreviewFromGC(m_pParaPreviewWidget->getGraphics(), w, h);
	m_pParaPreviewWidget->setPreview(m_pParaPreview);

	hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_STYLES_TOP_TEXT_CHARACTER_PREVIEW);

	m_pCharPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
													  hwndChild,
													  0);
	m_pCharPreviewWidget->getWindowSize(&w,&h);
	_createCharPreviewFromGC(m_pCharPreviewWidget->getGraphics(), w, h);
	m_pCharPreviewWidget->setPreview(m_pCharPreview);

	_populateWindowData();

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Styles::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case IDCANCEL:
		m_answer = a_CANCEL;
		// fall through

	case IDOK:
		m_answer = a_OK;
		EndDialog(hWnd,0);
		return 1;


	case AP_RID_DIALOG_STYLES_TOP_COMBO_LIST:
		if(wNotifyCode == CBN_SELCHANGE)
		{
			switch(_win32Dialog.getComboSelectedIndex(AP_RID_DIALOG_STYLES_TOP_COMBO_LIST))
			{
			case 0:
				m_whichType = USED_STYLES;
				break;
				
			case 1:
				m_whichType = ALL_STYLES;
				break;
				
			case 2:
				m_whichType = USER_STYLES;
				break;
			}

			_populateWindowData();
		}
		return 1;

	case AP_RID_DIALOG_STYLES_TOP_LIST_STYLES:
		if(wNotifyCode == LBN_SELCHANGE)
		{
			int row = _win32Dialog.getListSelectedIndex(AP_RID_DIALOG_STYLES_TOP_LIST_STYLES);

			if(row == -1)
			{
				m_selectedStyle = "";
				return 1;
			}


			char *p_buffer = new char [1024];

			_win32Dialog.getListText(AP_RID_DIALOG_STYLES_TOP_LIST_STYLES, row, p_buffer);

			m_selectedStyle = p_buffer;
			delete [] p_buffer;

			// refresh the previews
			_populatePreviews(false);
		}
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_Styles::_onDeltaPos(NM_UPDOWN * pnmud)
{
	return 0;
}

void AP_Win32Dialog_Styles::_populateWindowData(void)
{
	_populateCList();
	_populatePreviews(false);
}

void AP_Win32Dialog_Styles::_populateCList(void)
{
	const PD_Style * pStyle;
	const char * name = NULL;

	size_t nStyles = getDoc()->getStyleCount();
	xxx_UT_DEBUGMSG(("DOM: we have %d styles\n", nStyles));

	_win32Dialog.resetContent(AP_RID_DIALOG_STYLES_TOP_LIST_STYLES);
	
	for (UT_uint32 i = 0; i < nStyles; i++)
	{
	    const char * data[1];

	    getDoc()->enumStyles((UT_uint32)i, &name, &pStyle);

		// style has been deleted probably
		if (!pStyle)
			continue;

	    // all of this is safe to do... append should take a const char **
	    data[0] = name;

	    if ((m_whichType == ALL_STYLES) || 
			(m_whichType == USED_STYLES && pStyle->isUsed()) ||
			(m_whichType == USER_STYLES && pStyle->isUserDefined()))
		{
			_win32Dialog.addItemToList(AP_RID_DIALOG_STYLES_TOP_LIST_STYLES, *data);
		}
	}

}

const char * AP_Win32Dialog_Styles::getCurrentStyle (void) const
{

	if(m_selectedStyle.size() != 0)
		return m_selectedStyle.c_str();
	else
		return NULL;

}

void AP_Win32Dialog_Styles::setDescription (const char * desc) const
{
	AP_Win32Dialog_Styles *p_This = (AP_Win32Dialog_Styles *)this; // Cast away const

	p_This->_win32Dialog.setControlText(AP_RID_DIALOG_STYLES_TOP_LABEL_DESCRIPTION, desc);
}

