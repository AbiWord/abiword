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

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_Win32Dlg_Insert_Symbol.h"
#include "xap_Win32PreviewWidget.h"

#include "xap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_Insert_Symbol::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_Insert_Symbol * p = new XAP_Win32Dialog_Insert_Symbol(pFactory,id);
	return p;
}

XAP_Win32Dialog_Insert_Symbol::XAP_Win32Dialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Insert_Symbol(pDlgFactory,id)
{
	m_pSymbolPreviewWidget = NULL;
	m_pSamplePreviewWidget = NULL;
}

XAP_Win32Dialog_Insert_Symbol::~XAP_Win32Dialog_Insert_Symbol(void)
{
	DELETEP(m_pSymbolPreviewWidget);
	DELETEP(m_pSamplePreviewWidget);
	DELETEP(m_DrawSymbolSample);
}


void XAP_Win32Dialog_Insert_Symbol::runModal(XAP_Frame * pFrame)
{
}

void XAP_Win32Dialog_Insert_Symbol::destroy(void)
{
DestroyWindow(m_hDlg);
}

void XAP_Win32Dialog_Insert_Symbol::runModeless(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == XAP_DIALOG_ID_INSERT_SYMBOL);

	lpTemplate = MAKEINTRESOURCE(XAP_RID_DIALOG_INSERT_SYMBOL);

	HWND hWndDialog = CreateDialogParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	ShowWindow(hWndDialog, SW_SHOW);
	UT_ASSERT((hWndDialog != NULL));

	m_pApp->rememberModelessId(m_id, hWndDialog, this);


}

void XAP_Win32Dialog_Insert_Symbol::notifyActiveFrame(XAP_Frame *pFrame)
{
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);
	if((HWND)GetWindowLong(m_hDlg, GWL_HWNDPARENT) != pWin32Frame->getTopLevelWindow())
	{
		SetWindowLong(m_hDlg, GWL_HWNDPARENT, (long)pWin32Frame->getTopLevelWindow());
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
}

void XAP_Win32Dialog_Insert_Symbol::notifyCloseFrame(XAP_Frame *pFrame)
{
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);
	if((HWND)GetWindowLong(m_hDlg, GWL_HWNDPARENT) == pWin32Frame->getTopLevelWindow())
	{
		SetWindowLong(m_hDlg, GWL_HWNDPARENT, NULL);
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
}

BOOL CALLBACK XAP_Win32Dialog_Insert_Symbol::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	XAP_Win32Dialog_Insert_Symbol * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (XAP_Win32Dialog_Insert_Symbol *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (XAP_Win32Dialog_Insert_Symbol *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	default:
		return 0;
	}
}

#define _DSI(c,i)	SetDlgItemInt(hWnd,XAP_RID_DIALOG_##c,m_count.##i,FALSE)
#define _DS(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_Insert_Symbol::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// localize controls
	SetWindowText(hWnd, pSS->getValue(XAP_STRING_ID_DLG_Insert_SymbolTitle));
	_DSX(INSERTSYMBOL_INSERT_BUTTON,DLG_Insert);
	_DSX(INSERTSYMBOL_CLOSE_BUTTON,DLG_Close);

	m_hDlg = hWnd;

	// *** this is how we add the gc for symbol table ***
	// attach a new graphics context to the drawing area
	XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);
	UT_ASSERT(app);

	HWND hwndChild = GetDlgItem(hWnd, XAP_RID_DIALOG_INSERTSYMBOL_SYMBOLS);

	m_pSymbolPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
													  hwndChild,
													  0);
	UT_uint32 w,h;
	m_pSymbolPreviewWidget->getWindowSize(&w,&h);
	_createSymbolFromGC(m_pSymbolPreviewWidget->getGraphics(), w, h);
	m_pSymbolPreviewWidget->setPreview(m_DrawSymbol);


	hwndChild = GetDlgItem(hWnd, XAP_RID_DIALOG_INSERTSYMBOL_SYMBOL_PREVIEW);

	m_pSamplePreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
													  hwndChild,
													  0);

	m_pSamplePreviewWidget->getWindowSize(&w,&h);
	_createSymbolareaFromGC(m_pSamplePreviewWidget->getGraphics(), w, h);

	m_DrawSymbolSample = new XAP_Draw_Symbol_sample(m_DrawSymbol, m_pSamplePreviewWidget->getGraphics()); 
	m_pSamplePreviewWidget->setPreview(m_DrawSymbolSample);


	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);

	// Fill the list box with symbol fonts.

	HDC hDCScreen = CreateDC("DISPLAY", NULL, NULL, NULL);

	LOGFONT LogFont;
	LogFont.lfCharSet = SYMBOL_CHARSET;
	LogFont.lfFaceName[0] = '\0';
	EnumFontFamiliesEx(hDCScreen, &LogFont, (FONTENUMPROC)fontEnumProcedure, (LPARAM)this, 0);
	DeleteDC(hDCScreen);

	// Select the current font.

	UT_sint32 Index = SendDlgItemMessage(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_FINDSTRING, -1, (LPARAM)Symbol_font_selected);

	if(Index != -1)
	{
		_setFontFromCombo(Index);
	}
	else
	{
		_setFontFromCombo(0);
	}

	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_Insert_Symbol::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case XAP_RID_DIALOG_INSERTSYMBOL_CLOSE_BUTTON:
		m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;
		ShowWindow(hWnd, SW_HIDE);
		return 1;

	case XAP_RID_DIALOG_INSERTSYMBOL_INSERT_BUTTON:
		m_Inserted_Symbol = m_DrawSymbol->getCurrent();
		_onInsertButton();
		return 1;



	case XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST:
		switch(wNotifyCode)
		{
		case CBN_SELCHANGE:
			_setFontFromCombo(SendDlgItemMessage(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_GETCURSEL, 0, 0));
			return 1;
		}
		return 0;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

int XAP_Win32Dialog_Insert_Symbol::fontEnumProcedure(const LOGFONT *pLogFont, const TEXTMETRIC *pTextMetric, DWORD Font_type, LPARAM lParam)
{

	XAP_Win32Dialog_Insert_Symbol *pThis = (XAP_Win32Dialog_Insert_Symbol *)lParam;

	return pThis->_enumFont(pLogFont, pTextMetric, Font_type);
}

int XAP_Win32Dialog_Insert_Symbol::_enumFont(const LOGFONT *pLogFont, const TEXTMETRIC *pTextMetric, DWORD Font_type)
{
	if(Font_type & TRUETYPE_FONTTYPE) // Only except true type fonts.
	{
		SendDlgItemMessage(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_ADDSTRING, 0, (LPARAM)pLogFont->lfFaceName);
	}

	return TRUE;
}

void XAP_Win32Dialog_Insert_Symbol::_setFontFromCombo(UT_sint32 Index)
{
	if(Index >= 0)
	{
		SendDlgItemMessage(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_SETCURSEL, Index, 0);

		int Length = SendDlgItemMessage(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_GETLBTEXTLEN, Index, 0);

		if(Length != CB_ERR)
		{
			char *p_buffer = new char[Length + 1];

			SendDlgItemMessage(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_GETLBTEXT, Index, (LPARAM)p_buffer);

			strcpy(Symbol_font_selected, p_buffer);

			UT_UCSChar *p_UC_buffer = new UT_UCSChar[Length + 1];

			UT_UCS_strcpy_char(p_UC_buffer, p_buffer);
			
			m_DrawSymbol->setSelectedFont(p_buffer);
			m_DrawSymbol->draw();
			m_DrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

			delete [] p_UC_buffer;
			delete [] p_buffer;

		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}
}

