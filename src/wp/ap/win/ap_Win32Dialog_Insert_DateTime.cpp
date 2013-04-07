/* AbiWord
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
#include <time.h>
#include <locale.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_locale.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Insert_DateTime.h"
#include "ap_Win32Dialog_Insert_DateTime.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Win32Resources.rc2"

/*****************************************************************/

AP_Win32Dialog_Insert_DateTime::AP_Win32Dialog_Insert_DateTime(XAP_DialogFactory * pDlgFactory,
                         XAP_Dialog_Id id)
    : AP_Dialog_Insert_DateTime(pDlgFactory,id)
{
}

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Insert_DateTime::static_constructor(XAP_DialogFactory * pFactory,
                             XAP_Dialog_Id id)
{
    AP_Win32Dialog_Insert_DateTime * p = new AP_Win32Dialog_Insert_DateTime(pFactory,id);
    return p;
}

/*****************************************************************/

void AP_Win32Dialog_Insert_DateTime::runModal(XAP_Frame * pFrame)
{
    // raise the dialog
    
    UT_return_if_fail (m_id == AP_DIALOG_ID_INSERT_DATETIME);
    createModal(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_DATETIME));
}

void AP_Win32Dialog_Insert_DateTime::SetFormatsList(void)
{
    int i;
    wchar_t szCurrentDateTime[CURRENT_DATE_TIME_SIZE];
    time_t  tim = time(NULL);
    struct tm *pTime = localtime(&tim);

// debug code to produce widest possible string (in English, at least)
#if 0				
	struct tm wide = { 42, 3, 8, 27, 8, 100, 3 };	// Wednesday, September 27, 2000
	pTime = &wide;
#endif

	UT_LocaleInfo localeInfo;
	UT_String s;
	char buf[100];

	// 
	// MS not recognise standard ISO language and country codes. So, here
	// we retrieve the long language and country name for the users
	// default locale and pass these to setlocale.
	GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_SENGLANGUAGE, &buf[0], 100);
	s += buf;
	s += "_";
	
	GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_SENGCOUNTRY, &buf[0], 100);
	s += buf;
	
	UT_LocaleTransactor t(LC_ALL, s.c_str());
    UT_Win32LocaleString str;

    for (i = 0;InsertDateTimeFmts[i] != NULL;i++)
    {
      	str.fromASCII (InsertDateTimeFmts[i]);
        wcsftime(szCurrentDateTime, CURRENT_DATE_TIME_SIZE, str.c_str(), pTime);
        SendMessageW(m_hwndFormats, LB_ADDSTRING, 0, (LPARAM)szCurrentDateTime);
    }
}


#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Insert_DateTime::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_DateTime_DateTimeTitle));

	// localize controls
	_DSX(DATETIME_BTN_OK,			DLG_OK);
	_DSX(DATETIME_BTN_CANCEL,		DLG_Cancel);

	_DS(DATETIME_TEXT_FORMATS,		DLG_DateTime_AvailableFormats);

	// set initial state
    m_hwndFormats = GetDlgItem(hWnd, AP_RID_DIALOG_DATETIME_LIST_FORMATS);
    SetFormatsList();
    SendMessageW(m_hwndFormats,LB_SETCURSEL,(WPARAM)0,(LPARAM)0);
    
    XAP_Win32DialogHelper::s_centerDialog(hWnd);	

    return 1;             // 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Insert_DateTime::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
    WORD wId = LOWORD(wParam);

    switch (wId){
    case IDCANCEL:            // also AP_RID_DIALOG_DATETIME_BTN_CANCEL
        m_answer = a_CANCEL;
        // fall through

    case IDOK:              // also AP_RID_DIALOG_DATETIME_BTN_OK
        EndDialog(hWnd,0);
        return 1;
    
    case AP_RID_DIALOG_DATETIME_LIST_FORMATS:
        switch (HIWORD(wParam)){
            case LBN_SELCHANGE:
                _FormatListBoxChange();
                return 1;

            case LBN_DBLCLK:
                EndDialog(hWnd,0);
                return 1;

            default:
                return 0;
        }
        break;
    default:              // we did not handle this notification
        UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
        return 0;           // return zero to let windows take care of it.
    }
}


void AP_Win32Dialog_Insert_DateTime::_FormatListBoxChange(void)
{
    m_iFormatIndex = SendMessageW(m_hwndFormats, LB_GETCURSEL, 0, 0);
}

