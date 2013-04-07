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

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Goto.h"
#include "ap_Win32Dialog_Goto.h"
#include "ap_Win32Resources.rc2"
#include "fv_View.h"
#include "ap_Win32App.h"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Goto::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	AP_Win32Dialog_Goto * p = new AP_Win32Dialog_Goto(pFactory,id);
	return p;
}

AP_Win32Dialog_Goto::AP_Win32Dialog_Goto(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id) : AP_Dialog_Goto(pDlgFactory,id)
{
}

AP_Win32Dialog_Goto::~AP_Win32Dialog_Goto(void)
{
}


void AP_Win32Dialog_Goto::activate(void)
{
	ConstructWindowName();
	setDialogTitle (m_WindowName);

	SetFocus( GetDlgItem( m_hWnd,AP_RID_DIALOG_GOTO_EDIT_NUMBER ) );

	showWindow(SW_SHOW);
	bringWindowToTop();
}


void AP_Win32Dialog_Goto::destroy(void)
{
    destroyWindow();
	modeless_cleanup();
}

void AP_Win32Dialog_Goto::notifyActiveFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLongPtrW(m_hWnd, GWLP_HWNDPARENT) != static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		// Update the caption
		ConstructWindowName();
        setDialogTitle(m_WindowName);
		SetWindowLongPtrW(m_hWnd, GWLP_HWNDPARENT, (LONG_PTR)static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow());
		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void AP_Win32Dialog_Goto::notifyCloseFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLongPtrW(m_hWnd, GWLP_HWNDPARENT) == static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		SetWindowLongPtrW(m_hWnd, GWLP_HWNDPARENT, 0);
		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void AP_Win32Dialog_Goto::runModeless(XAP_Frame * pFrame)
{
	UT_return_if_fail (pFrame && m_id == AP_DIALOG_ID_GOTO);
    createModeless (pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_GOTO));

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)	getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);
 	m_pView->focusChange(AV_FOCUS_MODELESS);
}

void AP_Win32Dialog_Goto::GoTo (const char *number)
{
	UT_UCSChar *ucsnumber = (UT_UCSChar *) g_try_malloc (sizeof (UT_UCSChar) * (strlen(number) + 1));
	UT_return_if_fail(ucsnumber);
	UT_UCS4_strcpy_utf8_char(ucsnumber, number);
	int target = this->getSelectedRow ();
	this->getView()->gotoTarget ((AP_JumpTarget) target, ucsnumber);
	g_free (ucsnumber);
}

void AP_Win32Dialog_Goto::setSelectedRow (int row)
{
	m_iRow = row;
}

int AP_Win32Dialog_Goto::getSelectedRow (void)
{
	return (m_iRow);
}

#define _DSI(c,i)	SetDlgItemInt(m_hDlg,AP_RID_DIALOG_##c,m_count.##i,FALSE)
#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Goto::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	int iTarget;
	const char **ppszTargets;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	UT_Win32LocaleString str;

	m_oldvalue.clear();

	// Update the caption
	ConstructWindowName();
	setDialogTitle(m_WindowName); 

	// Disable the Go To button until something has been entered into the Number box
	EnableWindow( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_BTN_GOTO), FALSE );

	m_iRow = 0;
	ppszTargets = getJumpTargets();
	for ( iTarget = 0; ppszTargets[ iTarget ] != NULL; iTarget++ ) {
		str.fromUTF8(ppszTargets[ iTarget ] );
		SendMessageW( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_LIST_WHAT), LB_ADDSTRING, 0, (LPARAM)str.c_str());
	}

	SendMessageW(GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_LIST_WHAT), LB_SETCURSEL, m_iRow, 0);

	// localize controls
	_DSX(GOTO_BTN_CLOSE,		DLG_Close);
	_DS(GOTO_BTN_GOTO,			DLG_Goto_Btn_Goto);
	_DS(GOTO_BTN_PREV,			DLG_Goto_Btn_Prev);
	_DS(GOTO_BTN_NEXT,			DLG_Goto_Btn_Next);
	_DS(GOTO_TEXT_WHAT, 		DLG_Goto_Label_What);
	_DS(GOTO_TEXT_NUMBER,		DLG_Goto_Label_Number);
	_DS(GOTO_TEXT_INFO, 		DLG_Goto_Label_Help);

	// Initial data

	UT_uint32 count = getExistingBookmarksCount();
	for( UT_uint32 i = 0; i < count; i++) {
		str.fromUTF8(getNthExistingBookmark(i).c_str());
		SendMessageW( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_LIST_BOOKMARKS),
					 LB_ADDSTRING,
					 0,
					(LPARAM)str.c_str());
	}

	ShowWindow(GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), FALSE);


	SetFocus( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_EDIT_NUMBER) );
	
	centerDialog();

	return 0;							// 0 == we called SetFocus()
}

BOOL AP_Win32Dialog_Goto::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	wchar_t * pBuf = NULL;
	bool bValueOK = TRUE;
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	DWORD dwTextLength, dwCounter, dwStart;

	// before doing anything else, make sure that the main window is not entirely
	// without focus
	m_pView->focusChange(AV_FOCUS_MODELESS);

	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_GOTO_BTN_CLOSE
		destroy();
		return 1;

	case AP_RID_DIALOG_GOTO_LIST_WHAT:
		switch( wNotifyCode )
		{
		case LBN_SELCHANGE:
			m_iRow = (short) SendMessageW(GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_LIST_WHAT), LB_GETCURSEL, 0, 0);
			if( m_iRow == (short) AP_JUMPTARGET_BOOKMARK )
			{
				ShowWindow(GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_TEXT_INFO),FALSE);
				ShowWindow(GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_LIST_BOOKMARKS),TRUE);
				const XAP_StringSet * pSS = m_pApp->getStringSet();
				setDlgItemText(AP_RID_DIALOG_GOTO_TEXT_NUMBER,pSS->getValue(AP_STRING_ID_DLG_Goto_Label_Name));
			}
			else
			{
				ShowWindow(GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_TEXT_INFO),TRUE);
				ShowWindow(GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_LIST_BOOKMARKS),FALSE);
				const XAP_StringSet * pSS = m_pApp->getStringSet();
				setDlgItemText(AP_RID_DIALOG_GOTO_TEXT_NUMBER,pSS->getValue(AP_STRING_ID_DLG_Goto_Label_Number));
			}
			return 1;

		default:
			return 0;
		}

	case AP_RID_DIALOG_GOTO_LIST_BOOKMARKS:
		switch( wNotifyCode )
		{
		case LBN_DBLCLK:
		case LBN_SELCHANGE:
			{
				int nIndex = SendMessageW( hWndCtrl, LB_GETCURSEL, 0, 0);
				if( nIndex != LB_ERR )
				{
					wchar_t value[1024];
					SendMessageW( hWndCtrl, LB_GETTEXT, (WPARAM) nIndex, (LPARAM) &value);
					m_oldvalue.fromLocale (value);
					SetWindowTextW (GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_EDIT_NUMBER), value);
					EnableWindow( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_BTN_GOTO), TRUE );
					if( wNotifyCode == LBN_DBLCLK )
						GoTo( m_oldvalue.utf8_str().utf8_str());
				}
				return 1;
			}

		default:
			return 0;
		}

	case AP_RID_DIALOG_GOTO_BTN_PREV:
		if( m_iRow == (short) AP_JUMPTARGET_BOOKMARK )
		{
			m_oldvalue.clear();
			wchar_t value[1024];

			int nIndex = SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_GETCURSEL, 0, 0);
			if( nIndex == LB_ERR ) nIndex = 0;
			if( nIndex == 0 )
			{
				SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_SETCURSEL, (WPARAM) nIndex, 0);
                SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_GETTEXT,	(WPARAM) nIndex, (LPARAM) value);
			}
			else
			{
				SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_SETCURSEL, (WPARAM) nIndex-1, 0);
				SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_GETTEXT,	(WPARAM) nIndex-1, (LPARAM)value);
			}
            m_oldvalue.fromLocale (value);
			SetWindowTextW( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_EDIT_NUMBER), value);
			EnableWindow( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_BTN_GOTO), TRUE );

			if (!m_oldvalue.empty ())
				GoTo(m_oldvalue.utf8_str().utf8_str());
		}
		else
		{
			GoTo("-1");
		}
		return 1;

	case AP_RID_DIALOG_GOTO_BTN_NEXT:
		if( m_iRow == (short) AP_JUMPTARGET_BOOKMARK )
		{
			m_oldvalue.clear();
			wchar_t value[1024];

			int nCount = SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_GETCOUNT, 0, 0);
			int nIndex = SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_GETCURSEL, 0, 0);
			if( nIndex == LB_ERR ) nIndex = nCount-1;
			if( nIndex == nCount-1 )
			{
				SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_SETCURSEL, (WPARAM) nIndex, 0);
				SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_GETTEXT,	(WPARAM) nIndex, (LPARAM) value);
			}
			else
			{
				SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_SETCURSEL, (WPARAM) nIndex+1, 0);
				SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_GETTEXT,	(WPARAM) nIndex+1, (LPARAM) value);
			}
			SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_GETTEXT,	(WPARAM) nIndex+1, (LPARAM) value);
			EnableWindow( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_BTN_GOTO), TRUE );

			if (!m_oldvalue.empty ())
				GoTo( m_oldvalue.utf8_str().utf8_str());	
		}
		else
		{		
			GoTo("+1");
		}
		return 1;

	case AP_RID_DIALOG_GOTO_BTN_GOTO:
		if (!m_oldvalue.empty())		
			GoTo(m_oldvalue.utf8_str().utf8_str());
		return 1;

	case AP_RID_DIALOG_GOTO_EDIT_NUMBER:
		switch (wNotifyCode)
		{
		case EN_UPDATE:
			dwTextLength = GetWindowTextLengthW( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_EDIT_NUMBER) );
			if( m_iRow != (short) AP_JUMPTARGET_BOOKMARK )
			{
				if( dwTextLength )
				{
					pBuf = new wchar_t [ dwTextLength + 1 ];
					if( !pBuf )
						return 0;
				
					GetWindowTextW( hWndCtrl, pBuf, dwTextLength + 1 );

					// If the first character is + or -, skip over it in the
					// check loop below
					if( *pBuf == '-' || *pBuf == '+' )
						dwStart = 1;
					else
						dwStart = 0;

					// Make sure everything we have is numeric
					for( dwCounter = dwStart; dwCounter < dwTextLength; dwCounter++ )
					{
						if( !UT_UCS4_isdigit( pBuf[ dwCounter ] ) )
						{
							if (!m_oldvalue.empty ())
								m_oldvalue.fromLocale(L"0");
 							
							SetWindowTextW( hWndCtrl, m_oldvalue.c_str());
							
							bValueOK = FALSE;

							break;
						}
					}
	
					if( bValueOK )
					{
						m_oldvalue.fromLocale (pBuf);
	
						// Only enable the goto button if what we have actually contains a number
						EnableWindow( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_BTN_GOTO), !(((pBuf[ 0 ] == '-') || (pBuf[ 0 ] == '+')) && (pBuf[ 1 ] == '\0')) );
					}
					else
					{
						FREEP( pBuf );
		
						EnableWindow( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_BTN_GOTO), FALSE );
					}
				}
				else
				{
					m_oldvalue.clear();
					EnableWindow( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_BTN_GOTO), FALSE );
				}
			}
			else
			{

				if( dwTextLength )
				{
					EnableWindow( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_BTN_GOTO), TRUE ); 						
					wchar_t* value = new wchar_t [dwTextLength+1];
					GetWindowTextW( hWndCtrl, value, dwTextLength + 1 );
					m_oldvalue.fromLocale (value);
					delete value;
				}
				else
				{
					EnableWindow( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_BTN_GOTO), FALSE );
				}
			}
			return 1;

		case EN_SETFOCUS:
			SendMessageW( GetDlgItem(hWnd, AP_RID_DIALOG_GOTO_LIST_BOOKMARKS), LB_SETCURSEL, (WPARAM) -1, 0);
			return 1;

		default:
			return 0;
		}

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

