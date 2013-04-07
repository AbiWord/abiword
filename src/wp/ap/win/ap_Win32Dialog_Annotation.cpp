/* AbiWord
 *  Copyright (C) 2007 Jordi Mas i Hernàndez
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

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Win32LocaleString.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "ap_Dialog_Id.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Win32Dialog_Annotation.h"
#include "ap_Strings.h"
#include "ap_Win32App.h"

#include "ap_Win32Resources.rc2"
#include "ap_Win32Res_DlgAnnotation.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Annotation::static_constructor (XAP_DialogFactory * pFactory,
																XAP_Dialog_Id id)
{
	AP_Win32Dialog_Annotation * p = new AP_Win32Dialog_Annotation(pFactory,id);
	return p;
}

AP_Win32Dialog_Annotation::AP_Win32Dialog_Annotation(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Annotation(pDlgFactory,id)
{
}

AP_Win32Dialog_Annotation::~AP_Win32Dialog_Annotation(void)
{
}

void AP_Win32Dialog_Annotation::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail (m_id == AP_DIALOG_ID_ANNOTATION);

	 createModal(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_ANNOTATION));
}

#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_ANNOTATION_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_ANNOTATION_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Annotation::_onInitDialog(HWND /*hWnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_Annotation_Title));
	
	// localize controls
	_DSX(BTN_OK,			DLG_OK);
	_DSX(BTN_CANCEL,		DLG_Cancel);
	_DS(TEXT_TITLE,			DLG_Annotation_Title_LBL);
	_DS(TEXT_AUTHOR,		DLG_Annotation_Author_LBL);
	_DS(TEXT_DESCRIPTION,	DLG_Annotation_Description_LBL);

	_set_text(AP_RID_DIALOG_ANNOTATION_EDIT_TITLE, getTitle ());
	_set_text(AP_RID_DIALOG_ANNOTATION_EDIT_AUTHOR, getAuthor ());
	_set_text(AP_RID_DIALOG_ANNOTATION_EDIT_DESCRIPTION, getDescription ());

	centerDialog();
	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Annotation::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
		case IDCANCEL:
			setAnswer(a_CANCEL);
			EndDialog(hWnd,0);
			return 1;

		case IDOK:
		{
			std::string text;

			_get_text(AP_RID_DIALOG_ANNOTATION_EDIT_TITLE, text);
			setTitle(text);

			_get_text(AP_RID_DIALOG_ANNOTATION_EDIT_AUTHOR, text);
			setAuthor(text);

			_get_text(AP_RID_DIALOG_ANNOTATION_EDIT_DESCRIPTION, text);
			setDescription(text);

			setAnswer(a_OK);
			EndDialog(hWnd,0);
			return 1;
		}

		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}

void AP_Win32Dialog_Annotation::_get_text(int nID, std::string &text)
{
	UT_Win32LocaleString str;
	getDlgItemText(nID, str);
	text = str.utf8_str().utf8_str();
}

void AP_Win32Dialog_Annotation::_set_text(int nID, const std::string & text)
{
	setDlgItemText (nID, text.c_str());
}
