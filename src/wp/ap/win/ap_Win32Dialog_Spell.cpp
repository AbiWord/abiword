/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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
#include <richedit.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Spell.h"
#include "ap_Win32Dialog_Spell.h"

#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Spell::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
   AP_Win32Dialog_Spell * p = new AP_Win32Dialog_Spell(pFactory,id);
   return p;
}

AP_Win32Dialog_Spell::AP_Win32Dialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : AP_Dialog_Spell(pDlgFactory,id)
{
	m_bChangingSelection = 0;
}

AP_Win32Dialog_Spell::~AP_Win32Dialog_Spell(void)
{
}

/*****************************************************************/

void AP_Win32Dialog_Spell::runModal(XAP_Frame * pFrame)
{
	// call the base class method to initialize some basic xp stuff
	AP_Dialog_Spell::runModal(pFrame);

	m_bCancelled = UT_FALSE;
	UT_Bool bRes = nextMisspelledWord();

	// if nothing misspelled, then nothing to do
	if (!bRes)
		return;

	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == AP_DIALOG_ID_SPELL);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_SPELL);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

BOOL CALLBACK AP_Win32Dialog_Spell::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	AP_Win32Dialog_Spell * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_Spell *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (AP_Win32Dialog_Spell *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	default:
		return 0;
	}
}

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Spell::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_Spell_SpellTitle));

	// localize controls
	_DSX(SPELL_BTN_CANCEL,		DLG_Cancel);

	_DS(SPELL_TEXT_NOT,			DLG_Spell_UnknownWord);
	_DS(SPELL_TEXT_CHANGE,		DLG_Spell_ChangeTo);
	_DS(SPELL_TEXT_SUGGEST,		DLG_Spell_Suggestions);
	_DS(SPELL_BTN_IGNORE,		DLG_Spell_Ignore);
	_DS(SPELL_BTN_IGNOREALL,	DLG_Spell_IgnoreAll);
	_DS(SPELL_BTN_ADD,			DLG_Spell_AddToDict);
	_DS(SPELL_BTN_CHANGE,		DLG_Spell_Change);
	_DS(SPELL_BTN_CHANGEALL,	DLG_Spell_ChangeAll);

	// remember the windows we're using 
	m_hwndDlg = hWnd;
	m_hwndSentence = GetDlgItem(hWnd, AP_RID_DIALOG_SPELL_RICH_SENTENCE);
	m_hwndChangeTo = GetDlgItem(hWnd, AP_RID_DIALOG_SPELL_EDIT_CHANGE);
	m_hwndSuggest  = GetDlgItem(hWnd, AP_RID_DIALOG_SPELL_LIST_SUGGEST);

	// set initial state
	makeWordVisible();
	_showMisspelledWord();

	return 1;							// 1 == we did not call SetFocus()
}

void AP_Win32Dialog_Spell::_toggleChangeButtons(UT_Bool bEnable) const
{
	EnableWindow(GetDlgItem(m_hwndDlg,AP_RID_DIALOG_SPELL_BTN_CHANGE),bEnable);
	EnableWindow(GetDlgItem(m_hwndDlg,AP_RID_DIALOG_SPELL_BTN_CHANGEALL),bEnable);
}

void AP_Win32Dialog_Spell::_showMisspelledWord(void)
{
	// clear existing values
	SendMessage(m_hwndSentence, WM_SETTEXT, 0, 0);
	SendMessage(m_hwndChangeTo, WM_SETTEXT, 0, 0);
	SendMessage(m_hwndSuggest, LB_RESETCONTENT, 0, 0);

	CHARFORMAT cf;
	UT_UCSChar *p;
	UT_uint32 len;
	UT_uint32 sum = 0;
	char * buf;

	// insert start of sentence
	SendMessage(m_hwndSentence, EM_SETSEL, (WPARAM) sum, (LPARAM) sum);

	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_PROTECTED;
	cf.dwEffects = CFE_AUTOCOLOR | CFE_PROTECTED;
	SendMessage(m_hwndSentence, EM_SETCHARFORMAT, (WPARAM) SCF_ALL, (LPARAM) &cf);

	p = _getPreWord();
	len = UT_UCS_strlen(p);
	if (len)
	{
		buf = new char [len + 1];
		UT_UCS_strcpy_to_char(buf, p);
		SendMessage(m_hwndSentence, WM_SETTEXT, 0, (LPARAM)buf);
		DELETEP(buf);
	}
	FREEP(p);
	sum += len;

	// insert misspelled word (in highlight color)
	SendMessage(m_hwndSentence, EM_SETSEL, (WPARAM) sum, (LPARAM) sum);

	cf.dwMask = CFM_COLOR | CFM_PROTECTED;
	cf.dwEffects = CFE_PROTECTED;
	cf.crTextColor = RGB(255,0,0);
	SendMessage(m_hwndSentence, EM_SETCHARFORMAT, (WPARAM) SCF_SELECTION, (LPARAM) &cf);

	p = _getCurrentWord();
	len = UT_UCS_strlen(p);
	if (len)
	{
		buf = new char [len + 1];
		UT_UCS_strcpy_to_char(buf, p);
		SendMessage(m_hwndSentence, EM_REPLACESEL, FALSE, (LPARAM)buf);
		DELETEP(buf);
	}
	FREEP(p);
	sum += len;

	// insert end of sentence
	SendMessage(m_hwndSentence, EM_SETSEL, (WPARAM) sum, (LPARAM) sum);
	cf.dwMask = CFM_COLOR | CFM_PROTECTED;
	cf.dwEffects = CFE_AUTOCOLOR | CFE_PROTECTED;
	SendMessage(m_hwndSentence, EM_SETCHARFORMAT, (WPARAM) SCF_SELECTION, (LPARAM) &cf);

	p = _getPostWord();
	len = UT_UCS_strlen(p);
	if (len)
	{
		buf = new char [len + 1];
		UT_UCS_strcpy_to_char(buf, p);
		SendMessage(m_hwndSentence, EM_REPLACESEL, FALSE, (LPARAM)buf);
		DELETEP(buf);
	}
	FREEP(p);

	// insert suggestions
	if (!m_Suggestions.count) 
	{
		const XAP_StringSet * pSS = m_pApp->getStringSet();
		SendMessage(m_hwndSuggest, LB_ADDSTRING, 0, (LPARAM) pSS->getValue(AP_STRING_ID_DLG_Spell_NoSuggestions));

		m_iSelectedRow = -1;
		_toggleChangeButtons(UT_FALSE);
	} 
	else 
	{
		for (int i = 0; i < m_Suggestions.count; i++)
		{
			p = (UT_UCSChar *) m_Suggestions.word[i];
			len = UT_UCS_strlen(p);
			if (len)
			{
				buf = new char [len + 1];
				UT_UCS_strcpy_to_char(buf, p);
				SendMessage(m_hwndSuggest, LB_ADDSTRING, 0, (LPARAM)buf);
				DELETEP(buf);
			}
		}

		m_iSelectedRow = 0;
		_toggleChangeButtons(UT_TRUE);
	}

	SendMessage(m_hwndSuggest, LB_SETCURSEL, m_iSelectedRow, 0);

	// populate the change field, if appropriate
	m_bChangingSelection = 1;
	_suggestChange();
	m_bChangingSelection = 0;
}

void AP_Win32Dialog_Spell::_suggestChange(void)
{
	// cast is to match Unix usage
	// should be safe here, because there just aren't that many suggestions 
	m_iSelectedRow = (short) SendMessage(m_hwndSuggest, LB_GETCURSEL, 0, 0);

	if (!m_Suggestions.count) 
	{
		// no change to suggest, ignore it
		if (m_iSelectedRow != -1)
			SendMessage(m_hwndSuggest, LB_SETCURSEL, -1, 0);
	}
	else
	{
		// copy suggestion to edit field
		UT_ASSERT((m_iSelectedRow > -1));

		char buf[256];
		SendMessage(m_hwndSuggest, LB_GETTEXT, m_iSelectedRow, (LPARAM)buf);
		SendMessage(m_hwndChangeTo, WM_SETTEXT, 0, (LPARAM)buf);

		// you'd think this'd be overkill...
		SendMessage(m_hwndSuggest, LB_SETCURSEL, m_iSelectedRow, 0);
	}
}

// returns a pointer which needs to be FREEP'd by the caller
static UT_UCSChar * s_getUCSText(HWND hwnd)
{
	char * pBuf = NULL;
	UT_UCSChar * pUCS = NULL;

	DWORD len = GetWindowTextLength(hwnd);
	if (!len)
		return NULL;

	pBuf = new char [len + 1];
	if (!pBuf)
		goto FreeMemory;
	GetWindowText(hwnd,pBuf,len+1);

	UT_UCS_cloneString_char(&pUCS,pBuf);
	if (!pUCS)
		goto FreeMemory;

	DELETEP(pBuf);
	return pUCS;

FreeMemory:
	DELETEP(pBuf);
	FREEP(pUCS);

	return NULL;
}

void AP_Win32Dialog_Spell::_change(void)
{
	UT_UCSChar * replace = NULL;

	if (m_iSelectedRow != -1)
	{
		replace = (UT_UCSChar*) m_Suggestions.word[m_iSelectedRow];
		changeWordWith(replace);
	}
	else
	{
		replace = s_getUCSText(m_hwndChangeTo);
		if (!UT_UCS_strlen(replace)) 
		{
			FREEP(replace);
			return;
		}
		changeWordWith(replace);
		FREEP(replace);
	}

	_tryAgain();
}

void AP_Win32Dialog_Spell::_changeAll(void)
{
	UT_UCSChar * replace = NULL;
	if (m_iSelectedRow != -1)
	{
		replace = (UT_UCSChar*) m_Suggestions.word[m_iSelectedRow];
		addChangeAll(replace);
		changeWordWith(replace);
	}
	else
	{
		replace = s_getUCSText(m_hwndChangeTo);
		if (!UT_UCS_strlen(replace)) 
		{
			FREEP(replace);
			return;
		}
		addChangeAll(replace);
		changeWordWith(replace);
		FREEP(replace);
	}

	_tryAgain();
}

void AP_Win32Dialog_Spell::_tryAgain(void)
{
	// clear prior suggestions
	_purgeSuggestions();

	// what's next
	UT_Bool bRes = nextMisspelledWord();

	if (bRes)
	{
		// show word in main window
		makeWordVisible();

		// update dialog with new misspelled word info/suggestions
		_showMisspelledWord();
	}
	else
	{
		// all done
		UT_ASSERT((m_bCancelled == UT_FALSE));
		EndDialog(m_hwndDlg,0);
	}
}

BOOL AP_Win32Dialog_Spell::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case AP_RID_DIALOG_SPELL_RICH_SENTENCE:
		switch (wNotifyCode)
		{
		case EN_SETFOCUS:
			{
				UT_sint32 iAfter = m_iWordOffset + m_iWordLength - m_iSentenceStart;

				// set scroll position so misspelled word is centered
				SendMessage(m_hwndSentence, EM_SETSEL, (WPARAM) iAfter, (LPARAM) iAfter);	
				SendMessage(m_hwndSentence, EM_SCROLLCARET, 0, 0);
			}
			return 1;

		default:
			return 0;
		}
		
	case AP_RID_DIALOG_SPELL_EDIT_CHANGE:
		switch (wNotifyCode)
		{
		case EN_CHANGE:
			if (GetWindowTextLength(hWndCtrl))
			{
				if( !m_bChangingSelection )
				{
					m_iSelectedRow = -1;
					SendMessage(m_hwndSuggest, LB_SETCURSEL, m_iSelectedRow, 0);
					_toggleChangeButtons(UT_TRUE);
				}
			}
			else
			{
				_toggleChangeButtons(UT_FALSE);
			}
			return 1;

		default:
			return 0;
		}
		
	case AP_RID_DIALOG_SPELL_LIST_SUGGEST:
		switch (HIWORD(wParam))
		{
			case LBN_SELCHANGE:
				m_bChangingSelection = 1;
				_suggestChange();
				m_bChangingSelection = 0;
				return 1;

			case LBN_DBLCLK:
				m_bChangingSelection = 1;
				UT_ASSERT((m_iSelectedRow == SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0)));
				_change();
				m_bChangingSelection = 0;
				return 1;

			default:
				return 0;
		}
		break;

	case AP_RID_DIALOG_SPELL_BTN_IGNORE:
		ignoreWord();
		_tryAgain();
		return 1;

	case AP_RID_DIALOG_SPELL_BTN_IGNOREALL:
		addIgnoreAll();
		ignoreWord();
		_tryAgain();
		return 1;

	case AP_RID_DIALOG_SPELL_BTN_ADD:
		addToDict();
		ignoreWord();
		_tryAgain();
		return 1;

	case AP_RID_DIALOG_SPELL_BTN_CHANGE:
		_change();
		return 1;

	case AP_RID_DIALOG_SPELL_BTN_CHANGEALL:
		_changeAll();
		return 1;

	case IDCANCEL:						// also AP_RID_DIALOG_SPELL_BTN_CANCEL
		m_bCancelled = UT_TRUE;
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

