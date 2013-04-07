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

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_Win32Dlg_Insert_Symbol.h"
#include "xap_Win32PreviewWidget.h"

#include "gr_Win32Graphics.h"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32Resources.rc2"
#include "ap_Win32App.h"

/*****************************************************************/

char Symbol_font_selected[32] = "Symbol";

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
	m_DrawSymbolSample     = NULL;
	
}

XAP_Win32Dialog_Insert_Symbol::~XAP_Win32Dialog_Insert_Symbol(void)
{
	DELETEP(m_pSymbolPreviewWidget);
	DELETEP(m_pSamplePreviewWidget);
	DELETEP(m_DrawSymbolSample);
}


void XAP_Win32Dialog_Insert_Symbol::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == XAP_DIALOG_ID_INSERT_SYMBOL);
	
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCEW(XAP_RID_DIALOG_INSERT_SYMBOL));
}

void XAP_Win32Dialog_Insert_Symbol::runModeless(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == XAP_DIALOG_ID_INSERT_SYMBOL);

	setDialog(this);
	HWND hWndDialog = createModeless( pFrame, MAKEINTRESOURCEW(XAP_RID_DIALOG_INSERT_SYMBOL) );

	UT_ASSERT((hWndDialog != NULL));
	ShowWindow(hWndDialog, SW_SHOW);

	m_pApp->rememberModelessId(m_id, this);
}

void XAP_Win32Dialog_Insert_Symbol::destroy(void)
{
	modeless_cleanup();
	DestroyWindow(m_hDlg);
}

void XAP_Win32Dialog_Insert_Symbol::activate(void)
{
	UT_DebugOnly<int> iResult;

	// Update the caption
	ConstructWindowName();
    setDialogTitle(m_WindowName);

	iResult = ShowWindow( m_hDlg, SW_SHOW );

	iResult = BringWindowToTop( m_hDlg );

	UT_ASSERT((iResult != 0));
}


void XAP_Win32Dialog_Insert_Symbol::notifyActiveFrame(XAP_Frame *pFrame)
{
	UT_return_if_fail(pFrame);

	HWND frameHWND = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();
	if((HWND)GetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT) != frameHWND)
	{
		// Update the caption
		ConstructWindowName();
        setDialogTitle(m_WindowName);

		SetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT, (LONG_PTR)frameHWND);
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void XAP_Win32Dialog_Insert_Symbol::notifyCloseFrame(XAP_Frame *pFrame)
{
	UT_return_if_fail(pFrame);
	if((HWND)GetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT) == static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		SetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT, 0);
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

BOOL XAP_Win32Dialog_Insert_Symbol::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_hDlg = hWnd;

	// localize controls
	localizeControlText(XAP_RID_DIALOG_INSERTSYMBOL_INSERT_BUTTON,XAP_STRING_ID_DLG_Insert);
	localizeControlText(XAP_RID_DIALOG_INSERTSYMBOL_CLOSE_BUTTON,XAP_STRING_ID_DLG_Close);


	// *** this is how we add the gc for symbol table ***
	// attach a new graphics context to the drawing area
	UT_DebugOnly<XAP_Win32App *> app = static_cast<XAP_Win32App *> (m_pApp);
	UT_ASSERT(app);

	HWND hwndChild = GetDlgItem(hWnd, XAP_RID_DIALOG_INSERTSYMBOL_SYMBOLS);

	m_pSymbolPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
													  hwndChild,
													  0);
	UT_uint32 w,h;
	m_pSymbolPreviewWidget->getWindowSize(&w,&h);
	_createSymbolFromGC(m_pSymbolPreviewWidget->getGraphics(), w, h);
	m_pSymbolPreviewWidget->setPreview(m_DrawSymbol);
	m_pSymbolPreviewWidget->setInsertSymbolParent(this);

	hwndChild = GetDlgItem(hWnd, XAP_RID_DIALOG_INSERTSYMBOL_SYMBOL_PREVIEW);

	m_pSamplePreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
													  hwndChild,
													  0);

	m_pSamplePreviewWidget->getWindowSize(&w,&h);
	_createSymbolareaFromGC(m_pSamplePreviewWidget->getGraphics(), w, h);

	m_DrawSymbolSample = new XAP_Draw_Symbol_sample(m_DrawSymbol, m_pSamplePreviewWidget->getGraphics()); 
		
	// TODO: Colour
	GR_Win32Graphics* gr = (GR_Win32Graphics*) m_DrawSymbolSample->m_pSymbolDraw->getGraphics();
	
	
	gr->setBrush((HBRUSH)GetSysColorBrush(COLOR_3DFACE));
	
	m_pSamplePreviewWidget->setPreview(m_DrawSymbolSample);

	UT_DebugOnly<XAP_Draw_Symbol *> iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);

	// Fill the list box with symbol fonts.

	HDC hDCScreen = CreateDCW(L"DISPLAY", NULL, NULL, NULL);

#if 1
	EnumFontFamiliesW(hDCScreen, (const wchar_t *)NULL, (FONTENUMPROCW)fontEnumProcedure, (LPARAM)this);
#else
	LOGFONTW LogFont;
//	LogFont.lfCharSet = SYMBOL_CHARSET; - all fonts enum is more inline with XP nature
	LogFont.lfCharSet = DEFAULT_CHARSET;
	LogFont.lfFaceName[0] = '\0';
	EnumFontFamiliesExW(hDCScreen, &LogFont, (FONTENUMPROCW)fontEnumProcedure, (LPARAM)this, 0);
#endif	
	
	DeleteDC(hDCScreen);

	// Select the current font.

	UT_sint32 Index = SendDlgItemMessageW(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_FINDSTRING, -1, (LPARAM)Symbol_font_selected);

	if(Index != -1)
	{
		_setFontFromCombo(Index);
	}
	else
	{
		_setFontFromCombo(0);
	}

	// Update the caption
	ConstructWindowName();
    setDialogTitle(m_WindowName);	
    centerDialog();	

	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_Insert_Symbol::_onCommand(HWND /*hWnd*/, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
	case XAP_RID_DIALOG_INSERTSYMBOL_CLOSE_BUTTON:
		m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;
		destroy();
		return 1;

	case XAP_RID_DIALOG_INSERTSYMBOL_INSERT_BUTTON:
		doInsertSymbol();
		return 1;

	case XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST:
		switch(wNotifyCode)
		{
		case CBN_SELCHANGE:
			_setFontFromCombo(SendDlgItemMessageW(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_GETCURSEL, 0, 0));
			return 1;
		}
		return 0;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL XAP_Win32Dialog_Insert_Symbol::_onDeltaPos(NM_UPDOWN * /*pnmud*/)
{
	return FALSE;
}

int CALLBACK XAP_Win32Dialog_Insert_Symbol::fontEnumProcedure(const LOGFONTW *pLogFont, const TEXTMETRICW *pTextMetric, DWORD Font_type, LPARAM lParam)
{

	XAP_Win32Dialog_Insert_Symbol *pThis = (XAP_Win32Dialog_Insert_Symbol *)lParam;

	return pThis->_enumFont(pLogFont, pTextMetric, Font_type);
}

int XAP_Win32Dialog_Insert_Symbol::_enumFont(const LOGFONTW *pLogFont, const TEXTMETRICW * /*pTextMetric*/, DWORD Font_type)
{
	if( ((int)Font_type) & TRUETYPE_FONTTYPE ) // Only except true type fonts.
	{
		SendDlgItemMessageW(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_ADDSTRING, 0, (LPARAM)pLogFont->lfFaceName);
	}
//	if(Font_type & TRUETYPE_FONTTYPE) // Only accept true type fonts.
//	{
//		addItemToList(XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, pLogFont->lfFaceName);
//	}
	return TRUE;
}

void XAP_Win32Dialog_Insert_Symbol::_setFontFromCombo(UT_sint32 Index)
{
	if(Index >= 0)
	{
		SendDlgItemMessageW(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_SETCURSEL, Index, 0);

		int Length = SendDlgItemMessageW(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_GETLBTEXTLEN, Index, 0);

		if(Length != CB_ERR)
		{
			char *p_buffer = new char[Length + 1];

			SendDlgItemMessageW(m_hDlg, XAP_RID_DIALOG_INSERTSYMBOL_FONT_LIST, CB_GETLBTEXT, Index, (LPARAM)p_buffer);

			strcpy(Symbol_font_selected, p_buffer);

			UT_UCSChar *p_UC_buffer = new UT_UCSChar[Length + 1];

			UT_UCS4_strcpy_char(p_UC_buffer, p_buffer);
			
			m_DrawSymbol->setSelectedFont(p_buffer);
			m_DrawSymbol->draw();
			m_DrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

			delete [] p_UC_buffer;
			delete [] p_buffer;

		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
	}
}

void XAP_Win32Dialog_Insert_Symbol::doInsertSymbol( void )
{
	m_Inserted_Symbol = m_DrawSymbol->getCurrent();
	_onInsertButton();
}
